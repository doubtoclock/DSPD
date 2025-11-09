#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MAX_SKU 3000
#define MAX_ORD 5000
#define MAX_ITEMS 20
#define NAME_LEN 50

//struct definitions
typedef struct {
    int id;
    char name[NAME_LEN];
    float price;
    int stock;
    int soldCount;
} SKU;

typedef struct {
    int skuId;
    int quantity;
} OrderItem;

typedef struct {
    int id;
    OrderItem items[MAX_ITEMS];
    int itemCount;
    char status[15]; //placed / delivered / cancelled
    time_t timestamp;
} Order;

//global arrays
SKU skus[MAX_SKU];
Order orders[MAX_ORD];
int skuCount = 0, orderCount = 0;

//helper functions
int findSKUIndexById(int id) {
    for (int i = 0; i < skuCount; i++)
        if (skus[i].id == id)
            return i;
    return -1;
}

int findOrderIndexById(int id) {
    for (int i = 0; i < orderCount; i++)
        if (orders[i].id == id)
            return i;
    return -1;
}

void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
}

//file handling
void saveSKUsToFile() {
    FILE *f = fopen("skus.csv", "w");
    if (!f) {
        printf("Error saving SKUs!\n");
        return;
    }
    fprintf(f, "id,name,price,stock,soldCount\n");
    for (int i = 0; i < skuCount; i++)
        fprintf(f, "%d,%s,%.2f,%d,%d\n", skus[i].id, skus[i].name, skus[i].price, skus[i].stock, skus[i].soldCount);
    fclose(f);
}

void loadSKUsFromFile() {
    FILE *f = fopen("skus.csv", "r");
    if (!f) return;
    char line[200];
    fgets(line, sizeof(line), f); // skip header
    while (fgets(line, sizeof(line), f)) {
        SKU s;
        if (sscanf(line, "%d,%49[^,],%f,%d,%d", &s.id, s.name, &s.price, &s.stock, &s.soldCount) == 5)
            skus[skuCount++] = s;
    }
    fclose(f);
}

void saveOrdersToFile() {
    FILE *f = fopen("orders.csv", "w");
    if (!f) {
        printf("Error saving Orders!\n");
        return;
    }
    fprintf(f, "id,status,timestamp,itemCount,items\n");
    for (int i = 0; i < orderCount; i++) {
        fprintf(f, "%d,%s,%ld,%d,", orders[i].id, orders[i].status, orders[i].timestamp, orders[i].itemCount);
        for (int j = 0; j < orders[i].itemCount; j++) {
            fprintf(f, "%d:%d;", orders[i].items[j].skuId, orders[i].items[j].quantity);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void loadOrdersFromFile() {
    FILE *f = fopen("orders.csv", "r");
    if (!f) return;
    char line[400];
    fgets(line, sizeof(line), f); // skip header
    while (fgets(line, sizeof(line), f)) {
        Order o;
        char itemsStr[300];
        if (sscanf(line, "%d,%14[^,],%ld,%d,%299[^\n]", &o.id, o.status, &o.timestamp, &o.itemCount, itemsStr) == 5) {
            o.itemCount = 0;
            char *token = strtok(itemsStr, ";");
            while (token && o.itemCount < MAX_ITEMS) {
                sscanf(token, "%d:%d", &o.items[o.itemCount].skuId, &o.items[o.itemCount].quantity);
                o.itemCount++;
                token = strtok(NULL, ";");
            }
            orders[orderCount++] = o;
        }
    }
    fclose(f);
}

//feature implementations
void addSKU() {
    SKU s;
    printf("Enter SKU ID: ");
    scanf("%d", &s.id);

    if (findSKUIndexById(s.id) != -1) {
        printf("SKU with this ID already exists!\n");
        return;
    }

    printf("Enter SKU Name: ");
    scanf(" %[^\n]", s.name);
    printf("Enter Price: ");
    scanf("%f", &s.price);
    printf("Enter Stock: ");
    scanf("%d", &s.stock);

    if (s.price < 0 || s.stock < 0) {
        printf("Invalid price or stock.\n");
        return;
    }

    s.soldCount = 0;
    skus[skuCount++] = s;
    printf("SKU added successfully!\n");
}

void updateSKU() {
    int id;
    printf("Enter SKU ID to update: ");
    scanf("%d", &id);
    int idx = findSKUIndexById(id);
    if (idx == -1) {
        printf("SKU not found.\n");
        return;
    }

    printf("Enter new price: ");
    scanf("%f", &skus[idx].price);
    printf("Enter new stock: ");
    scanf("%d", &skus[idx].stock);
    printf("Updated successfully!\n");
}

void deleteSKU() {
    int id;
    printf("Enter SKU ID to delete: ");
    scanf("%d", &id);
    int idx = findSKUIndexById(id);
    if (idx == -1) {
        printf("SKU not found.\n");
        return;
    }

    for (int i = 0; i < orderCount; i++) {
        if (strcmp(orders[i].status, "DELIVERED") == 0) {
            for (int j = 0; j < orders[i].itemCount; j++) {
                if (orders[i].items[j].skuId == id) {
                    printf("Cannot delete SKU referenced by delivered order %d.\n", orders[i].id);
                    return;
                }
            }
        }
    }

    for (int i = idx; i < skuCount - 1; i++)
        skus[i] = skus[i + 1];
    skuCount--;
    printf("SKU deleted successfully.\n");
}

void placeOrder() {
    if (skuCount == 0) {
        printf("No SKUs available.\n");
        return;
    }

    Order o;
    o.id = orderCount + 1;
    o.itemCount = 0;
    strcpy(o.status, "PLACED");
    o.timestamp = time(NULL);

    int itemCount;
    printf("Enter number of items: ");
    scanf("%d", &itemCount);

    float total = 0;
    for (int i = 0; i < itemCount; i++) {
        int skuId, qty;
        printf("Enter SKU ID and Quantity: ");
        scanf("%d %d", &skuId, &qty);
        int idx = findSKUIndexById(skuId);

        if (idx == -1) {
            printf("Invalid SKU ID %d\n", skuId);
            continue;
        }
        if (qty > skus[idx].stock) {
            printf("Insufficient stock for %s.\n", skus[idx].name);
            continue;
        }

        o.items[o.itemCount].skuId = skuId;
        o.items[o.itemCount].quantity = qty;
        o.itemCount++;
        total += skus[idx].price * qty;
    }

    orders[orderCount++] = o;
    printf("Order placed successfully! Order ID: %d | Subtotal: %.2f\n", o.id, total);
}

void deliverOrder() {
    int id;
    printf("Enter Order ID to deliver: ");
    scanf("%d", &id);
    int idx = findOrderIndexById(id);
    if (idx == -1) {
        printf("Order not found.\n");
        return;
    }
    if (strcmp(orders[idx].status, "PLACED") != 0) {
        printf("Order cannot be delivered (not in PLACED state).\n");
        return;
    }

    for (int i = 0; i < orders[idx].itemCount; i++) {
        int skuId = orders[idx].items[i].skuId;
        int qty = orders[idx].items[i].quantity;
        int sidx = findSKUIndexById(skuId);

        if (sidx == -1 || skus[sidx].stock < qty) {
            printf("Delivery failed: stock issue for SKU %d.\n", skuId);
            return;
        }
    }

    for (int i = 0; i < orders[idx].itemCount; i++) {
        int skuId = orders[idx].items[i].skuId;
        int qty = orders[idx].items[i].quantity;
        int sidx = findSKUIndexById(skuId);

        skus[sidx].stock -= qty;
        skus[sidx].soldCount += qty;
    }

    strcpy(orders[idx].status, "DELIVERED");
    printf("Order delivered successfully.\n");
}

void cancelOrder() {
    int id;
    printf("Enter Order ID to cancel: ");
    scanf("%d", &id);
    int idx = findOrderIndexById(id);
    if (idx == -1) {
        printf("Order not found.\n");
        return;
    }
    if (strcmp(orders[idx].status, "PLACED") != 0) {
        printf("Order cannot be cancelled (already %s).\n", orders[idx].status);
        return;
    }

    strcpy(orders[idx].status, "CANCELLED");
    printf("Order cancelled successfully.\n");
}

void topKBestsellers() {
    int k;
    printf("Enter K: ");
    scanf("%d", &k);
    if (k > skuCount) k = skuCount;

    SKU sorted[MAX_SKU];
    memcpy(sorted, skus, sizeof(SKU) * skuCount);

    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (sorted[i].soldCount < sorted[j].soldCount)
                { SKU temp = sorted[i]; sorted[i] = sorted[j]; sorted[j] = temp; }

    printf("Top-%d Bestsellers:\n", k);
    for (int i = 0; i < k; i++)
        printf("%d. %s (Sold: %d)\n", i + 1, sorted[i].name, sorted[i].soldCount);
}

//main menu
int main() {
    loadSKUsFromFile();
    loadOrdersFromFile();

    int choice;
    do {
        printf("\nBlinkit-Lite Menu\n");
        printf("1. Add SKU\n2. Update SKU\n3. Delete SKU\n4. Place Order\n5. Deliver Order\n6. Cancel Order\n");
        printf("7. Top-K Bestsellers\n0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addSKU(); break;
            case 2: updateSKU(); break;
            case 3: deleteSKU(); break;
            case 4: placeOrder(); break;
            case 5: deliverOrder(); break;
            case 6: cancelOrder(); break;
            case 7: topKBestsellers(); break;
            case 0:
                printf("Saving data...\n");
                saveSKUsToFile();
                saveOrdersToFile();
                printf("Exiting...\n");
                break;
            default:
                {printf("Invalid choice!\n"); choice = 0;}
        }
    } while (choice != 0);

    return 0;
}