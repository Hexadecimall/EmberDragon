/*
 * inventory.c
 *
 * A fixed-capacity warehouse inventory table keyed by SKU. It supports
 * adding items, receiving and shipping stock with low-stock protection,
 * and computing aggregate metrics such as total on-hand units and the
 * count of items below their reorder threshold.
 */

#include <stdint.h>
#include <string.h>

#define MAX_ITEMS   128
#define SKU_LEN     16

/* One stock-keeping unit. quantity is the units physically on hand;
 * reorder_level is the threshold at or below which the item is "low". */
typedef struct {
    char sku[SKU_LEN];      /* null-terminated unique identifier         */
    int32_t quantity;       /* units currently on hand (>= 0)            */
    int32_t reorder_level;  /* low-stock threshold (>= 0)                */
    int32_t unit_cost;      /* cost per unit in cents, for valuation     */
} Item;

/* The whole inventory: a flat array searched linearly by SKU. */
typedef struct {
    Item items[MAX_ITEMS];
    int count;              /* number of populated slots (0..MAX_ITEMS)  */
} Inventory;

/*
 * Reset an inventory to empty.
 * Parameters: inv - inventory to clear (non-NULL).
 * Returns nothing.
 */
void inventory_init(Inventory *inv) {
    inv->count = 0;
}

/*
 * Locate an item by SKU.
 * Parameters: inv - inventory to search; sku - null-terminated key.
 * Returns a pointer to the matching Item, or NULL if no item matches.
 * The returned pointer aliases storage inside inv and must not be freed.
 * Complexity: O(count) linear scan.
 */
Item *inventory_find(Inventory *inv, const char *sku) {
    for (int i = 0; i < inv->count; i++) {
        if (strncmp(inv->items[i].sku, sku, SKU_LEN) == 0)
            return &inv->items[i];
    }
    return NULL;
}

/*
 * Register a new SKU with an initial quantity and reorder threshold.
 * Parameters: inv, sku (key), qty (initial units), reorder (threshold),
 *             cost (cents per unit).
 * Returns 1 on success, 0 if the table is full or the SKU already exists.
 * The SKU is copied and always null-terminated.
 */
int inventory_add(Inventory *inv, const char *sku, int32_t qty,
                  int32_t reorder, int32_t cost) {
    if (inv->count >= MAX_ITEMS)
        return 0;                       /* no room left */
    if (inventory_find(inv, sku) != NULL)
        return 0;                       /* SKUs are unique */
    Item *slot = &inv->items[inv->count];
    strncpy(slot->sku, sku, SKU_LEN - 1);
    slot->sku[SKU_LEN - 1] = '\0';      /* guarantee termination */
    slot->quantity = qty < 0 ? 0 : qty; /* never start negative */
    slot->reorder_level = reorder;
    slot->unit_cost = cost;
    inv->count++;
    return 1;
}

/*
 * Add received stock to an existing SKU.
 * Parameters: inv, sku, qty (units received, must be > 0).
 * Returns the new on-hand quantity, or -1 if the SKU is unknown or qty <= 0.
 */
int32_t inventory_receive(Inventory *inv, const char *sku, int32_t qty) {
    if (qty <= 0)
        return -1;
    Item *it = inventory_find(inv, sku);
    if (it == NULL)
        return -1;
    it->quantity += qty;
    return it->quantity;
}

/*
 * Ship stock out, refusing to oversell.
 * Parameters: inv, sku, qty (units to ship, must be > 0).
 * Returns the remaining quantity on success, or -1 if the SKU is unknown,
 * qty is non-positive, or there is insufficient stock (no change made).
 */
int32_t inventory_ship(Inventory *inv, const char *sku, int32_t qty) {
    if (qty <= 0)
        return -1;
    Item *it = inventory_find(inv, sku);
    if (it == NULL)
        return -1;
    if (it->quantity < qty)
        return -1;                      /* cannot ship what we don't have */
    it->quantity -= qty;
    return it->quantity;
}

/*
 * Count items whose on-hand quantity is at or below their reorder level.
 * Parameters: inv - inventory to scan.
 * Returns the number of items needing replenishment. O(count).
 */
int inventory_count_low_stock(const Inventory *inv) {
    int low = 0;
    for (int i = 0; i < inv->count; i++) {
        if (inv->items[i].quantity <= inv->items[i].reorder_level)
            low++;
    }
    return low;
}

/*
 * Compute the total monetary value of all stock on hand.
 * Parameters: inv - inventory to value.
 * Returns sum over all items of quantity * unit_cost, in cents. O(count).
 * A 64-bit accumulator is used to avoid overflow on large inventories.
 */
int64_t inventory_total_value(const Inventory *inv) {
    int64_t total = 0;
    for (int i = 0; i < inv->count; i++) {
        total += (int64_t)inv->items[i].quantity * inv->items[i].unit_cost;
    }
    return total;
}
