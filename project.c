#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SKU 3000
#define MAX_ORD 5000
#define MAX_ITEMS 20

//SKU structure 
typedef struct {
    int skuId;
    char name[64];
    char category[32];
    float price;
    int stock;
    int soldCount;
} SKU;

//Order structure
typedef struct {
    int orderId;
    long long orderTime;
    int status; //0=Placed,1=Delivered,2=Cancelled
    long long deliveryTime;
    int itemIds[MAX_ITEMS];
    int itemQty[MAX_ITEMS];
    int itemCount;
    float subtotal;
} Order;

//Global arrays
SKU skus[MAX_SKU];
int skuCount = 0;
Order orders[MAX_ORD];
int orderCount = 0;

//Filenames
const char *SKU_FILE = "skus.csv";
const char *ORDER_FILE = "orders.csv";

//Utility functions
void strtolowercpy(char *dest, const char *src) {
    for (; *src; src++, dest++) 
        *dest = (char)tolower((char)*src);
    *dest = '\0';
}

int findSKUIndexById(int id) {//// cant use binary search as SKUs may or may not be sorted by id depending on the inputs and time of calling of fn
    for (int i = 0; i < skuCount; i++)
        if (skus[i].skuId == id) return i;
    return -1;
}

int findOrderIndexById(int id) {// cant use binary search as orders may be sorted by time or id depending on the inputs and time of calling of fn
    for (int i = 0; i < orderCount; ++i)
        if (orders[i].orderId == id) 
            return i;
    return -1;
}

int isValidDateTime(long long t) {
    long long year = t / 100000000LL;
    long long month = (t / 1000000LL) % 100;
    long long day = (t / 10000LL) % 100;
    long long hour = (t / 100LL) % 100;
    long long minute = (t)% 100;

    if (year < 2000 || year > 9999 || month < 1 || month > 12 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return 0;

    int maxDay;
    if (month == 2)
        maxDay = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28;
    else if (month == 4 || month == 6 || month == 9 || month == 11)
        maxDay = 30;
    else
        maxDay = 31;

    return (day >= 1 && day <= maxDay);
}

// CSV Save / Load 
void saveDataToFiles() {
    FILE *f = fopen(SKU_FILE, "w");
    if (!f) { 
        printf("Error saving %s\n", SKU_FILE);  
    }
    else{
        fprintf(f, "skuId,name,category,price,stock,soldCount\n");
        for (int i = 0; i < skuCount; i++) 
        {
            SKU *s = &skus[i];
            fprintf(f, "%d,%s,%s,%.2f,%d,%d\n", s->skuId, s->name, s->category, s->price, s->stock, s->soldCount);
        }
        fclose(f);
    }

    f = fopen(ORDER_FILE, "w");
    if (!f) { 
        printf("Error saving %s\n", ORDER_FILE); 
    }
    else{
        fprintf(f, "orderId,orderTime,status,deliveryTime,itemCount,subtotal,itemIds,itemQtys\n");
        for (int i = 0; i < orderCount; i++) {
            Order *o = &orders[i];
            fprintf(f, "%d,%lld,%d,%lld,%d,%.2f,\"", o->orderId, o->orderTime, o->status, o->deliveryTime, o->itemCount, o->subtotal);
            for (int j = 0; j < o->itemCount; j++) {
                fprintf(f, "%d", o->itemIds[j]);
                if (j < o->itemCount - 1) fprintf(f, "|");
            }
            fprintf(f, "\",\"");
            for (int j = 0; j < o->itemCount; j++) {
                fprintf(f, "%d", o->itemQty[j]);
                if (j < o->itemCount - 1) fprintf(f, "|");
            }
            fprintf(f, "\"\n");
        }
        fclose(f);
        printf("Data saved to CSV files successfully.\n");
    }   
}

void loadDataFromFiles() {
    FILE *f = fopen(SKU_FILE, "r");
    if (f) {
        char line[256];
        fgets(line, sizeof(line), f); //skip header
        skuCount = 0;
        while (fgets(line, sizeof(line), f)) {
            SKU s;
            if (sscanf(line, "%d,%63[^,],%31[^,],%f,%d,%d",
                       &s.skuId, s.name, s.category, &s.price, &s.stock, &s.soldCount) == 6) {
                skus[skuCount++] = s;
            }
        }
        fclose(f);
    }

    f = fopen(ORDER_FILE, "r");
    if (f) {
        char line[512];
        fgets(line, sizeof(line), f); //skip header
        orderCount = 0;
        while (fgets(line, sizeof(line), f)) {
            Order o;
            char ids[256], qtys[256];
            if (sscanf(line, "%d,%lld,%d,%lld,%d,%f,\"%255[^\"]\",\"%255[^\"]\"",
                       &o.orderId, &o.orderTime, &o.status, &o.deliveryTime, &o.itemCount, &o.subtotal, ids, qtys) == 8) {
                o.itemCount = (o.itemCount > MAX_ITEMS) ? MAX_ITEMS : o.itemCount;
                int k = 0;
                char *tok = strtok(ids, "|");
                while (tok && k < o.itemCount) {
                    o.itemIds[k++] = atoi(tok);
                    tok = strtok(NULL, "|");
                }
                k = 0;
                tok = strtok(qtys, "|");
                while (tok && k < o.itemCount) {
                    o.itemQty[k++] = atoi(tok);
                    tok = strtok(NULL, "|");
                }
                orders[orderCount++] = o;
            }
        }
        fclose(f);
    }
}

// Print Functions 
void printSKU(const SKU *s) {
    printf("ID:%d | Name:%s | Cat:%s | Price:%.2f | Stock:%d | Sold:%d\n",
           s->skuId, s->name, s->category, s->price, s->stock, s->soldCount);
}
void printOrder(const Order *o) {
    printf("OrderID:%d | Time:%lld | Status:%s | DeliveryTime:%lld | Items:%d | Subtotal:%.2f\n",
           o->orderId, o->orderTime,
           (o->status == 0 ? "Placed" : (o->status == 1 ? "Delivered" : "Cancelled")),
           o->deliveryTime, o->itemCount, o->subtotal);
    for (int i = 0; i < o->itemCount; ++i)
        printf("   Item %d -> SKU:%d x %d\n", i + 1, o->itemIds[i], o->itemQty[i]);
}

// Functional Requirements 

void addSKU() {

    if (skuCount >= MAX_SKU) { 
        printf("Max SKUs reached.\n"); return; 
    }
    SKU s;
    printf("Enter SKU ID: "); scanf("%d", &s.skuId);
    if (findSKUIndexById(s.skuId) != -1) { 
        printf("SKU ID exists.\n"); return; 
    }
    getchar();
    printf("Enter Name: "); 
    fgets(s.name, 64, stdin); 
    s.name[strcspn(s.name, "\n")] = 0;
    printf("Enter Category: "); 
    fgets(s.category, 32, stdin); 
    s.category[strcspn(s.category, "\n")] = 0;
    printf("Enter Price: "); 
    scanf("%f", &s.price);
    printf("Enter Stock: "); 
    scanf("%d", &s.stock);
    s.soldCount = 0;
    skus[skuCount++] = s;
    printf("SKU Added Successfully.\n");
}

void updateOrDeleteSKU() {
    int id;
    printf("Enter SKU ID: "); 
    scanf("%d", &id);
    int idx = findSKUIndexById(id);
    if (idx == -1) { printf("Not found.\n"); return; }
    printSKU(&skus[idx]);
    printf("1) Update  2) Delete  0) Cancel: ");
    int ch; scanf("%d", &ch);
    if (ch == 1) {
        getchar();
        char buf[64];
        printf("Enter new name (blank keep): "); fgets(buf, 64, stdin); buf[strcspn(buf, "\n")] = 0;
        if (strlen(buf)) strcpy(skus[idx].name, buf);
        printf("Enter new category (blank keep): "); fgets(buf, 64, stdin); buf[strcspn(buf, "\n")] = 0;
        if (strlen(buf)) strcpy(skus[idx].category, buf);
        printf("Enter new price (-1 keep): "); float p; scanf("%f", &p); if (p >= 0) skus[idx].price = p;
        printf("Enter new stock (-1 keep): "); int st; scanf("%d", &st); if (st >= 0) skus[idx].stock = st;
        printf("Updated.\n");
    } else if (ch == 2) {
        int referenced = 0;
        for (int i = 0; i < orderCount; i++)
            if (orders[i].status == 1)
                for (int j = 0; j < orders[i].itemCount; j++)
                    if (orders[i].itemIds[j] == id) referenced = 1;
        if (referenced) { printf("Cannot delete; referenced by delivered orders.\n"); return; }
        for (int i = idx; i < skuCount - 1; i++) skus[i] = skus[i + 1];
        skuCount--;
        printf("Deleted.\n");
    }
}

void placeOrder() {
    if (orderCount >= MAX_ORD) return;
    Order o; o.status = 0; o.deliveryTime = 0; o.subtotal = 0;
    printf("Enter Order ID: "); scanf("%d", &o.orderId);
    if (findOrderIndexById(o.orderId) != -1) { 
        printf("Exists.\n"); 
        return; 
    }
    printf("Enter orderTime (YYYYMMDDHHMM): "); 
    scanf("%lld", &o.orderTime);
    if(!isValidDateTime(o.orderTime))
    {
        printf("Invalid datetime\n");
        return;
    }

    printf("Number of items (1-%d): ", MAX_ITEMS);
    scanf("%d", &o.itemCount);
    for (int i = 0; i < o.itemCount; i++) {
        printf("Item %d SKU ID: ", i + 1); scanf("%d", &o.itemIds[i]);
        int idx = findSKUIndexById(o.itemIds[i]);
        if (idx == -1) 
        { 
            printf("Invalid SKU.\n"); \
            return; 
        }
        printf("Quantity: "); scanf("%d", &o.itemQty[i]);
        SKU *s = &skus[idx];
        if (s->stock < o.itemQty[i])
        {
            printf("Not enough stock");
            return;
        }
        o.subtotal += skus[idx].price * o.itemQty[i];
    }
    orders[orderCount++] = o;
    printf("Order placed.\n");
}

void deliverOrder() {
    int id; printf("Order ID: "); scanf("%d", &id);
    int idx = findOrderIndexById(id);
    if (idx == -1) { 
        printf("Not found.\n"); 
        return; 
    }
    Order *o = &orders[idx];
    if (o->status != 0) { 
        printf("Not in placed state.\n"); 
        return; 
    }

    printf("Delivery time (YYYYMMDDHHMM): "); 
    scanf("%lld", &o->deliveryTime);
    if(!isValidDateTime(o->deliveryTime))
    {
        printf("Invalid datetime");
        return;
    }

    for (int i = 0; i < o->itemCount; i++) {
        int sidx = findSKUIndexById(o->itemIds[i]);
        if (sidx == -1 || skus[sidx].stock < o->itemQty[i]) {
            printf("Insufficient stock for SKU %d.\n", o->itemIds[i]);
            return;
        }
    }
    for (int i = 0; i < o->itemCount; i++) {
        int sidx = findSKUIndexById(o->itemIds[i]);
        skus[sidx].stock -= o->itemQty[i];
        skus[sidx].soldCount += o->itemQty[i];
    }
    o->status = 1;
    printf("Delivered successfully.\n");
}

void cancelOrder() {
    int id; printf("Order ID: "); scanf("%d", &id);
    int idx = findOrderIndexById(id);

    if (idx == -1) { 
        printf("Not found.\n"); 
        return; 
    }

    if (orders[idx].status != 0) { 
        printf("Cannot cancel now.\n"); 
        return; 
    }
    orders[idx].status = 2;
    printf("Cancelled.\n");
}

void searchSKUByName() {
    getchar();
    char q[64]; printf("Substring: "); 
    fgets(q, 64, stdin); 
    q[strcspn(q, "\n")] = 0;
    char ql[64]; strtolowercpy(ql, q);
    int found = 0;
    for (int i = 0; i < skuCount; i++) {
        char name[64]; strtolowercpy(name, skus[i].name);
        if (strstr(name, ql)) { printSKU(&skus[i]); found = 1; }
    }
    if (!found) printf("No matches.\n");
}

void sortOrdersByTime() {
    for (int i = 0; i < orderCount - 1; i++)
        for (int j = i + 1; j < orderCount; j++)
            if (orders[j].orderTime < orders[i].orderTime) {
                Order tmp = orders[i]; orders[i] = orders[j]; orders[j] = tmp;
            }
    printf("Sorted by time.\n");
}

void topBestsellers() {
    int K; printf("Enter K: "); scanf("%d", &K);
    if (K > skuCount) K = skuCount;
    int idxs[MAX_SKU]; for (int i = 0; i < skuCount; i++) idxs[i] = i;
    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (skus[idxs[j]].soldCount > skus[idxs[i]].soldCount) {
                int t = idxs[i]; idxs[i] = idxs[j]; idxs[j] = t;
            }
    for (int i = 0; i < K; i++)
        printf("%d) %s Sold:%d\n", i + 1, skus[idxs[i]].name, skus[idxs[i]].soldCount);
}

void abcAnalysis() {
    long total = 0;
    for (int i = 0; i < skuCount; i++) total += skus[i].soldCount;
    if (total == 0) { printf("No sales.\n"); return; }
    int idxs[MAX_SKU]; for (int i = 0; i < skuCount; i++) idxs[i] = i;
    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (skus[idxs[j]].soldCount > skus[idxs[i]].soldCount) {
                int t = idxs[i]; idxs[i] = idxs[j]; idxs[j] = t;
            }
    double cum = 0;
    for (int i = 0; i < skuCount; i++) {
        SKU *s = &skus[idxs[i]];
        cum += (100.0 * s->soldCount) / total;
        char cat = (cum <= 80) ? 'A' : (cum <= 95 ? 'B' : 'C');
        printf("%d) %s Sold:%d Cumulative:%.2f%% Category:%c\n", i + 1, s->name, s->soldCount, cum, cat);
    }
}

// Listing/Menu 
void listAllSKUs() { 
    for (int i = 0; i < skuCount; i++) 
        printSKU(&skus[i]); 
}

void listAllOrders() { 
    for (int i = 0; i < orderCount; i++) 
        printOrder(&orders[i]); 
}

void main() {
    int ch;
    loadDataFromFiles();
    do {
        printf("\n--- BLINKIT-LITE MENU ---\n");
        printf("1) Add SKU\n");
        printf("2) Update/Delete SKU\n");
        printf("3) Place Order\n");
        printf("4) Deliver Order\n");
        printf("5) Cancel Order\n");
        printf("6) Search SKU\n");
        printf("7) Sort Orders\n");
        printf("8) Top K Bestsellers\n");
        printf("9) ABC Analysis\n");
        printf("10) List SKUs\n");
        printf("11) List Orders\n");
        printf("12) Save Now\n");
        printf("0) Save & Exit\n");
        printf("Choice: ");

        if (scanf("%d", &ch) != 1) {
            //handle non-integer input 
            while (getchar() != '\n'); //flush 
            printf("Invalid input. Please enter a number corresponding to a menu choice.\n");
            ch = -1; //set to invalid so switch->default runs on next loop 
            continue;
        }

        switch (ch) {
            case 1: addSKU(); break;
            case 2: updateOrDeleteSKU(); break;
            case 3: placeOrder(); break;
            case 4: deliverOrder(); break;
            case 5: cancelOrder(); break;
            case 6: searchSKUByName(); break;
            case 7: sortOrdersByTime(); break;
            case 8: topBestsellers(); break;
            case 9: abcAnalysis(); break;
            case 10: listAllSKUs(); break;
            case 11: listAllOrders(); break;
            case 12: saveDataToFiles(); break;
            case 0:
                saveDataToFiles();
                printf("Data saved. Exiting program.\n");
                break;
            default:
                saveDataToFiles();
                ch=0;
                printf("Choice entered is not valid. Please try again.\n");
                break;
        }

    } while (ch != 0);
}
