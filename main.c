// blinkit-lite.c
// simplified: no readInt/readFloat helpers; inline scanf checks

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_SKU 3000
#define MAX_ORD 5000
#define MAX_ITEMS 20
#define NAME_LEN 64
#define SKU_FILE "skus.csv"
#define ORDER_FILE "orders.csv"

// structs (only these two)
typedef struct {
    int skuId;
    char name[NAME_LEN];
    char category[NAME_LEN];
    float price;
    int stock;
    int soldCount;
} SKU;

typedef struct {
    int orderId;
    int orderTime;           // YYYYMMDDHHMM
    int status;              // 0=Placed,1=Delivered,2=Cancelled
    int deliveryTime;        // 0 if NA
    int itemIds[MAX_ITEMS];
    int itemQty[MAX_ITEMS];
    int itemCount;
    float subtotal;
} Order;

SKU SKUList[MAX_SKU];
Order OrderList[MAX_ORD];
int skuCount = 0;
int orderCount = 0;

// prototypes
int findSKUIndexById(int id);
int findOrderIndexById(int id);
void toLower(const char *src, char *dst);
int validYYYYMMDDHHMM(int t);
void loadSKUs();
void saveSKUs();
void loadOrders();
void saveOrders();

void Add_SKU();
void Update_or_Delete_SKU();
void Place_Order();
void Deliver_Order();
void Cancel_Order();
void Search_SKU_ByName_Substring();
void Sort_Orders_ByTime();
void TopK_Bestsellers();
void ABC_Analysis();

void shiftLeftSKUs(int idx);
void shiftLeftOrders(int idx);

// implementations

int findSKUIndexById(int id) {
    for (int i = 0; i < skuCount; i++)
        if (SKUList[i].skuId == id) return i;
    return -1;
}

int findOrderIndexById(int id) {
    for (int i = 0; i < orderCount; i++)
        if (OrderList[i].orderId == id) return i;
    return -1;
}

void toLower(const char *src, char *dst) {
    int i;
    for (i = 0; src[i] && i < NAME_LEN-1; i++) dst[i] = tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

int validYYYYMMDDHHMM(int t) {
    if (t <= 0) return 0;
    int yyyy = t / 100000000;
    int mon  = (t / 1000000) % 100;
    int dd   = (t / 10000) % 100;
    int hh   = (t / 100) % 100;
    int mm   = t % 100;

    if (yyyy < 1900 || mon < 1 || mon > 12 || dd < 1 || dd > 31 || hh < 0 || hh > 23 || mm < 0 || mm > 59)
        return 0;

    int daysInMonth;
    switch (mon) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: daysInMonth = 31; break;
        case 4: case 6: case 9: case 11: daysInMonth = 30; break;
        case 2:
            if ((yyyy % 400 == 0) || (yyyy % 4 == 0 && yyyy % 100 != 0)) daysInMonth = 29;
            else daysInMonth = 28;
            break;
        default: return 0;
    }
    if (dd > daysInMonth) return 0;
    return 1;
}

// file persistence

void loadSKUs() {
    FILE *f = fopen(SKU_FILE, "r");
    if (!f) return;
    char line[512];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    if (strstr(line, "skuId") == NULL) fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        SKU s;
        if (sscanf(line, "%d,%63[^,],%63[^,],%f,%d,%d", &s.skuId, s.name, s.category, &s.price, &s.stock, &s.soldCount) == 6) {
            if (skuCount < MAX_SKU) SKUList[skuCount++] = s;
        }
    }
    fclose(f);
}

void saveSKUs() {
    FILE *f = fopen(SKU_FILE, "w");
    if (!f) { printf("error: cannot save skus\n"); return; }
    fprintf(f, "skuId,name,category,price,stock,soldCount\n");
    for (int i = 0; i < skuCount; i++)
        fprintf(f, "%d,%s,%s,%.2f,%d,%d\n",
                SKUList[i].skuId, SKUList[i].name, SKUList[i].category, SKUList[i].price, SKUList[i].stock, SKUList[i].soldCount);
    fclose(f);
}

void loadOrders() {
    FILE *f = fopen(ORDER_FILE, "r");
    if (!f) return;
    char line[1024];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    if (strstr(line, "orderId") == NULL) fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        Order o;
        char itemsStr[512] = {0};
        int scanned = sscanf(line, "%d,%d,%d,%d,%d,%f,%511[^\n]",
                             &o.orderId, &o.orderTime, &o.status, &o.deliveryTime, &o.itemCount, &o.subtotal, itemsStr);
        if (scanned >= 6) {
            o.itemCount = 0;
            if (scanned == 7) {
                char *tok = strtok(itemsStr, ";");
                while (tok && o.itemCount < MAX_ITEMS) {
                    int sid = 0, q = 0;
                    if (sscanf(tok, "%d:%d", &sid, &q) == 2) {
                        o.itemIds[o.itemCount] = sid;
                        o.itemQty[o.itemCount] = q;
                        o.itemCount++;
                    }
                    tok = strtok(NULL, ";");
                }
            }
            if (orderCount < MAX_ORD) OrderList[orderCount++] = o;
        }
    }
    fclose(f);
}

void saveOrders() {
    FILE *f = fopen(ORDER_FILE, "w");
    if (!f) { printf("error: cannot save orders\n"); return; }
    fprintf(f, "orderId,orderTime,status,deliveryTime,itemCount,subtotal,items\n");
    for (int i = 0; i < orderCount; i++) {
        Order *o = &OrderList[i];
        fprintf(f, "%d,%d,%d,%d,%d,%.2f,", o->orderId, o->orderTime, o->status, o->deliveryTime, o->itemCount, o->subtotal);
        for (int j = 0; j < o->itemCount; j++) fprintf(f, "%d:%d;", o->itemIds[j], o->itemQty[j]);
        fprintf(f, "\n");
    }
    fclose(f);
}

// features

void shiftLeftSKUs(int idx) {
    for (int i = idx; i < skuCount - 1; i++) SKUList[i] = SKUList[i+1];
    skuCount--;
}

void shiftLeftOrders(int idx) {
    for (int i = idx; i < orderCount - 1; i++) OrderList[i] = OrderList[i+1];
    orderCount--;
}

void Add_SKU() {
    if (skuCount >= MAX_SKU) { printf("sku list full\n"); return; }
    SKU s;
    printf("enter sku id (int, unique): ");
    if (scanf("%d", &s.skuId) != 1) { while (getchar()!='\n'); printf("invalid id\n"); return; }
    if (s.skuId <= 0) { printf("invalid id\n"); return; }
    if (findSKUIndexById(s.skuId) != -1) { printf("sku id exists\n"); return; }
    printf("enter name (no comma): ");
    scanf(" %[^\n]", s.name);
    printf("enter category (no comma): ");
    scanf(" %[^\n]", s.category);
    printf("enter price (float, >=0): ");
    if (scanf("%f", &s.price) != 1) { while (getchar()!='\n'); printf("invalid price\n"); return; }
    if (s.price < 0) { printf("negative price not allowed\n"); return; }
    printf("enter stock (int, >=0): ");
    if (scanf("%d", &s.stock) != 1) { while (getchar()!='\n'); printf("invalid stock\n"); return; }
    if (s.stock < 0) { printf("negative stock not allowed\n"); return; }
    s.soldCount = 0;
    SKUList[skuCount++] = s;
    printf("sku added\n");
}

void Update_or_Delete_SKU() {
    printf("enter sku id to update or delete: ");
    int id;
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("invalid id\n"); return; }
    int idx = findSKUIndexById(id);
    if (idx == -1) { printf("sku not found\n"); return; }
    printf("enter 1 to update, 2 to delete: ");
    int op; if (scanf("%d", &op) != 1) { while (getchar()!='\n'); printf("invalid\n"); return; }
    if (op == 1) {
        printf("enter new name (no comma): ");
        scanf(" %[^\n]", SKUList[idx].name);
        printf("enter new category (no comma): ");
        scanf(" %[^\n]", SKUList[idx].category);
        printf("enter new price (>=0): ");
        if (scanf("%f", &SKUList[idx].price) != 1) { while (getchar()!='\n'); printf("invalid price\n"); return; }
        if (SKUList[idx].price < 0) { printf("invalid price\n"); return; }
        printf("enter new stock (>=0): ");
        if (scanf("%d", &SKUList[idx].stock) != 1) { while (getchar()!='\n'); printf("invalid stock\n"); return; }
        if (SKUList[idx].stock < 0) { printf("invalid stock\n"); return; }
        printf("sku updated\n");
    } else if (op == 2) {
        for (int i = 0; i < orderCount; i++) {
            if (OrderList[i].status == 1) {
                for (int j = 0; j < OrderList[i].itemCount; j++)
                    if (OrderList[i].itemIds[j] == id) {
                        printf("cannot delete: referenced by delivered order %d\n", OrderList[i].orderId);
                        return;
                    }
            }
        }
        shiftLeftSKUs(idx);
        printf("sku deleted\n");
    } else printf("invalid op\n");
}

void Place_Order() {
    if (orderCount >= MAX_ORD) { printf("order list full\n"); return; }
    Order o;
    printf("enter order id (int, unique): ");
    if (scanf("%d", &o.orderId) != 1) { while (getchar()!='\n'); printf("invalid id\n"); return; }
    if (o.orderId <= 0) { printf("invalid id\n"); return; }
    if (findOrderIndexById(o.orderId) != -1) { printf("order id exists\n"); return; }
    printf("enter orderTime (YYYYMMDDHHMM): ");
    if (scanf("%d", &o.orderTime) != 1) { while (getchar()!='\n'); printf("invalid\n"); return; }
    if (!validYYYYMMDDHHMM(o.orderTime)) { printf("invalid orderTime format\n"); return; }
    o.status = 0; o.deliveryTime = 0; o.itemCount = 0; o.subtotal = 0.0f;
    printf("enter number of distinct items (max %d): ", MAX_ITEMS);
    int n; if (scanf("%d", &n) != 1) { while (getchar()!='\n'); printf("invalid\n"); return; }
    if (n <= 0 || n > MAX_ITEMS) { printf("invalid item count\n"); return; }
    for (int i = 0; i < n; i++) {
        printf("enter skuId and qty for item %d: ", i+1);
        int sid, q;
        if (scanf("%d %d", &sid, &q) != 2) { while (getchar()!='\n'); printf("invalid input, skipping\n"); continue; }
        if (q <= 0) { printf("qty must be >0, skipping item\n"); continue; }
        int sidx = findSKUIndexById(sid);
        if (sidx == -1) { printf("invalid sku id %d, skipping\n", sid); continue; }
        o.itemIds[o.itemCount] = sid;
        o.itemQty[o.itemCount] = q;
        o.itemCount++;
        o.subtotal += SKUList[sidx].price * (float)q;
    }
    if (o.itemCount == 0) { printf("no valid items, order not placed\n"); return; }
    OrderList[orderCount++] = o;
    printf("order placed with id %d, subtotal %.2f\n", o.orderId, o.subtotal);
}

void Deliver_Order() {
    printf("enter order id to deliver: ");
    int id; if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("invalid id\n"); return; }
    int idx = findOrderIndexById(id);
    if (idx == -1) { printf("order not found\n"); return; }
    Order *o = &OrderList[idx];
    if (o->status != 0) { printf("order not in placed state\n"); return; }
    printf("enter deliveryTime (YYYYMMDDHHMM): ");
    int dtime; if (scanf("%d", &dtime) != 1) { while (getchar()!='\n'); printf("invalid\n"); return; }
    if (!validYYYYMMDDHHMM(dtime)) { printf("invalid delivery time\n"); return; }
    for (int i = 0; i < o->itemCount; i++) {
        int sid = o->itemIds[i], q = o->itemQty[i];
        int sidx = findSKUIndexById(sid);
        if (sidx == -1) { printf("delivery failed: sku %d missing\n", sid); return; }
        if (SKUList[sidx].stock < q) { printf("delivery failed: insufficient stock for sku %d\n", sid); return; }
    }
    for (int i = 0; i < o->itemCount; i++) {
        int sid = o->itemIds[i], q = o->itemQty[i];
        int sidx = findSKUIndexById(sid);
        SKUList[sidx].stock -= q;
        SKUList[sidx].soldCount += q;
    }
    o->status = 1; o->deliveryTime = dtime;
    printf("order delivered successfully\n");
}

void Cancel_Order() {
    printf("enter order id to cancel: ");
    int id; if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("invalid id\n"); return; }
    int idx = findOrderIndexById(id);
    if (idx == -1) { printf("order not found\n"); return; }
    Order *o = &OrderList[idx];
    if (o->status != 0) { printf("only placed orders can be cancelled (current status %d)\n", o->status); return; }
    o->status = 2;
    printf("order cancelled\n");
}

void Search_SKU_ByName_Substring() {
    char query[NAME_LEN];
    printf("enter substring to search (case-insensitive): ");
    scanf(" %[^\n]", query);
    char qlow[NAME_LEN]; toLower(query, qlow);
    int found = 0;
    for (int i = 0; i < skuCount; i++) {
        char nameLow[NAME_LEN]; toLower(SKUList[i].name, nameLow);
        if (strstr(nameLow, qlow) != NULL) {
            printf("skuId:%d | name:%s | category:%s | price:%.2f | stock:%d | sold:%d\n",
                   SKUList[i].skuId, SKUList[i].name, SKUList[i].category, SKUList[i].price, SKUList[i].stock, SKUList[i].soldCount);
            found++;
        }
    }
    if (!found) printf("no matches\n");
}

void Sort_Orders_ByTime() {
    if (orderCount < 2) { printf("not enough orders to sort\n"); return; }
    for (int i = 0; i < orderCount - 1; i++)
        for (int j = i + 1; j < orderCount; j++)
            if (OrderList[i].orderTime > OrderList[j].orderTime ||
                (OrderList[i].orderTime == OrderList[j].orderTime && OrderList[i].orderId > OrderList[j].orderId)) {
                Order tmp = OrderList[i];
                OrderList[i] = OrderList[j];
                OrderList[j] = tmp;
            }
    printf("orders sorted by time\n");
}

void TopK_Bestsellers() {
    printf("enter K: ");
    int K; if (scanf("%d", &K) != 1) { while (getchar()!='\n'); printf("invalid\n"); return; }
    if (K <= 0) { printf("invalid k\n"); return; }
    if (skuCount == 0) { printf("no skus\n"); return; }
    if (K > skuCount) K = skuCount;
    int idxs[MAX_SKU];
    for (int i = 0; i < skuCount; i++) idxs[i] = i;
    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++) {
            SKU *si = &SKUList[idxs[i]];
            SKU *sj = &SKUList[idxs[j]];
            if (si->soldCount < sj->soldCount || (si->soldCount == sj->soldCount && strcmp(si->name, sj->name) > 0)) {
                int t = idxs[i]; idxs[i] = idxs[j]; idxs[j] = t;
            }
        }
    printf("top-%d bestsellers:\n", K);
    for (int i = 0; i < K; i++) {
        SKU *s = &SKUList[idxs[i]];
        printf("%d. skuId:%d name:%s sold:%d\n", i+1, s->skuId, s->name, s->soldCount);
    }
}

void ABC_Analysis() {
    if (skuCount == 0) { printf("no skus\n"); return; }
    long totalSold = 0;
    for (int i = 0; i < skuCount; i++) totalSold += SKUList[i].soldCount;
    if (totalSold == 0) { printf("no sales data yet\n"); return; }
    int idxs[MAX_SKU];
    for (int i = 0; i < skuCount; i++) idxs[i] = i;
    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (SKUList[idxs[i]].soldCount < SKUList[idxs[j]].soldCount) {
                int t = idxs[i]; idxs[i] = idxs[j]; idxs[j] = t;
            }
    double cumulativePercent = 0.0;
    int aCount = 0, bCount = 0, cCount = 0;
    printf("abc analysis:\n");
    for (int i = 0; i < skuCount; i++) {
        SKU *s = &SKUList[idxs[i]];
        cumulativePercent += ((double)s->soldCount / (double)totalSold) * 100.0;
        char cat = (cumulativePercent <= 80.0) ? 'A' : (cumulativePercent <= 95.0) ? 'B' : 'C';
        if (cat == 'A') aCount++; else if (cat == 'B') bCount++; else cCount++;
        printf("%-30s | sold:%5d | cat:%c | cum%%:%.2f\n", s->name, s->soldCount, cat, cumulativePercent);
    }
    printf("category totals: A:%d B:%d C:%d\n", aCount, bCount, cCount);
}

// main
int main() {
    loadSKUs();
    loadOrders();
    int ch;
    do {
        printf("\n--- blinkit-lite menu ---\n");
        printf("1. Add_SKU\n2. Update_or_Delete_SKU\n3. Place_Order\n4. Deliver_Order\n5. Cancel_Order\n6. Search_SKU_ByName_Substring\n7. Sort_Orders_ByTime\n8. TopK_Bestsellers\n9. ABC_Analysis\n0. Exit (auto-save)\n");
        printf("enter choice: ");
        if (scanf("%d", &ch) != 1) { while (getchar()!='\n'); printf("invalid choice\n"); ch = -1; }
        switch (ch) {
            case 1: Add_SKU(); break;
            case 2: Update_or_Delete_SKU(); break;
            case 3: Place_Order(); break;
            case 4: Deliver_Order(); break;
            case 5: Cancel_Order(); break;
            case 6: Search_SKU_ByName_Substring(); break;
            case 7: Sort_Orders_ByTime(); break;
            case 8: TopK_Bestsellers(); break;
            case 9: ABC_Analysis(); break;
            case 0:
                printf("saving data...\n");
                saveSKUs();
                saveOrders();
                printf("exiting\n");
                break;
            default: if (ch != 0) printf("invalid choice\n");
        }
    } while (ch != 0);
    return 0;
}