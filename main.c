#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_SKU 3000
#define MAX_ORD 5000
#define MAX_ITEMS 20
#define NAME_LEN 50

// ---------- STRUCT DEFINITIONS ----------
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
    char status[15];   // PLACED / DELIVERED / CANCELLED
    time_t timestamp;
} Order;

// ---------- GLOBAL ARRAYS ----------
SKU skus[MAX_SKU];
Order orders[MAX_ORD];
int skuCount = 0, orderCount = 0;

// ---------- HELPER FUNCTIONS ----------
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

// ---------- FEATURE IMPLEMENTATIONS ----------

// 1. Add SKU
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

// 2. Update SKU
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

// 3. Delete SKU
void deleteSKU() {
    int id;
    printf("Enter SKU ID to delete: ");
    scanf("%d", &id);
    int idx = findSKUIndexById(id);
    if (idx == -1) {
        printf("SKU not found.\n");
        return;
    }

    // Check if SKU appears in any delivered order
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

// 4. Place Order
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

// 5. Deliver Order
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

// 6. Cancel Order
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

// 7. Search SKU by name substring
void searchSKU() {
    char query[NAME_LEN], tempName[NAME_LEN];
    printf("Enter substring to search: ");
    scanf(" %[^\n]", query);
    toLowerCase(query);

    printf("Results:\n");
    for (int i = 0; i < skuCount; i++) {
        strcpy(tempName, skus[i].name);
        toLowerCase(tempName);
        if (strstr(tempName, query))
            printf("ID: %d | Name: %s | Price: %.2f | Stock: %d\n", skus[i].id, skus[i].name, skus[i].price, skus[i].stock);
    }
}

// 8. Sort Orders by time
void sortOrdersByTime() {
    for (int i = 0; i < orderCount - 1; i++) {
        for (int j = i + 1; j < orderCount; j++) {
            if (orders[i].timestamp > orders[j].timestamp ||
                (orders[i].timestamp == orders[j].timestamp && orders[i].id > orders[j].id)) {
                Order temp = orders[i];
                orders[i] = orders[j];
                orders[j] = temp;
            }
        }
    }
    printf("Orders sorted by time.\n");
}

// 9. Top-K Bestsellers
void topKBestsellers() {
    int k;
    printf("Enter K: ");
    scanf("%d", &k);
    if (k > skuCount) k = skuCount;

    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (skus[i].soldCount < skus[j].soldCount ||
               (skus[i].soldCount == skus[j].soldCount && strcmp(skus[i].name, skus[j].name) > 0)) {
                SKU temp = skus[i];
                skus[i] = skus[j];
                skus[j] = temp;
            }

    printf("Top-%d Bestsellers:\n", k);
    for (int i = 0; i < k; i++)
        printf("%d. %s (Sold: %d)\n", i + 1, skus[i].name, skus[i].soldCount);
}

// 10. ABC Analysis
void abcAnalysis() {
    int totalSold = 0;
    for (int i = 0; i < skuCount; i++)
        totalSold += skus[i].soldCount;
    if (totalSold == 0) {
        printf("No sales data yet.\n");
        return;
    }

    // Sort by soldCount descending
    for (int i = 0; i < skuCount - 1; i++)
        for (int j = i + 1; j < skuCount; j++)
            if (skus[i].soldCount < skus[j].soldCount) {
                SKU temp = skus[i];
                skus[i] = skus[j];
                skus[j] = temp;
            }

    float cumulative = 0;
    printf("ABC Analysis:\n");
    for (int i = 0; i < skuCount; i++) {
        cumulative += (float)skus[i].soldCount / totalSold * 100;
        char cat = (cumulative <= 80) ? 'A' : (cumulative <= 95) ? 'B' : 'C';
        printf("%-20s | Sold: %4d | Category: %c\n", skus[i].name, skus[i].soldCount, cat);
    }
}

// ---------- MAIN MENU ----------
int main() {
    int choice;
    do {
        printf("\n--- Blinkit-Lite Menu ---\n");
        printf("1. Add SKU\n2. Update SKU\n3. Delete SKU\n4. Place Order\n5. Deliver Order\n6. Cancel Order\n");
        printf("7. Search SKU\n8. Sort Orders by Time\n9. Top-K Bestsellers\n10. ABC Analysis\n0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addSKU(); break;
            case 2: updateSKU(); break;
            case 3: deleteSKU(); break;
            case 4: placeOrder(); break;
            case 5: deliverOrder(); break;
            case 6: cancelOrder(); break;
            case 7: searchSKU(); break;
            case 8: sortOrdersByTime(); break;
            case 9: topKBestsellers(); break;
            case 10: abcAnalysis(); break;
            case 0: printf("Exiting...\n"); break;
            default: {printf("Invalid choice!\n");}
        }
    } while (choice != 0);

    return 0;
}
