/*
 * inventory.c — Fixed-capacity warehouse inventory with SKU lookup and
 * low-stock alerting.
 *
 * Items are stored in a flat array keyed by an integer SKU. The module supports
 * adding new line items, restocking and selling existing ones (never letting
 * on-hand quantity go negative), and scanning for items that have fallen at or
 * below their reorder threshold. All quantities and prices are integers
 * (price is whole cents).
 */

#include <stdint.h>
#include <string.h>

#define MAX_ITEMS    64   /* hard cap on distinct SKUs in this warehouse */
#define NAME_LEN     32   /* including the NUL terminator                */

/* One stock-keeping unit and its current state on the shelf. */
typedef struct {
    int32_t sku;                 /* unique positive identifier            */
    char    name[NAME_LEN];      /* human-readable label, NUL-terminated  */
    int32_t quantity;            /* units currently on hand, >= 0         */
    int32_t reorder_level;       /* restock when quantity <= this value   */
    int64_t unit_price_cents;    /* sale price per unit in cents          */
} Item;

/* The whole catalog: a packed array plus a live element count. */
typedef struct {
    Item items[MAX_ITEMS];
    int  count;
} Inventory;

/* Reset the inventory to empty. Existing slots are logically discarded. */
void inventory_init(Inventory *inv) {
    inv->count = 0;
}

/*
 * Locate an item by SKU. Returns the array index of the match, or -1 if no
 * item with that SKU exists. Linear scan, O(count).
 */
int inventory_find(const Inventory *inv, int32_t sku) {
    for (int i = 0; i < inv->count; i++) {
        if (inv->items[i].sku == sku)
            return i;
    }
    return -1;
}

/*
 * Register a brand-new SKU. Returns 1 on success; 0 if the catalog is full or
 * the SKU already exists (duplicates are rejected to keep `sku` a unique key).
 * The name is copied and always NUL-terminated even if truncated.
 */
int inventory_add(Inventory *inv, int32_t sku, const char *name,
                  int32_t quantity, int32_t reorder_level,
                  int64_t unit_price_cents) {
    if (inv->count >= MAX_ITEMS)
        return 0;
    if (inventory_find(inv, sku) != -1)
        return 0;

    Item *it = &inv->items[inv->count++];
    it->sku             = sku;
    it->quantity        = quantity;
    it->reorder_level   = reorder_level;
    it->unit_price_cents = unit_price_cents;

    /* Copy at most NAME_LEN-1 bytes and force a terminator. */
    strncpy(it->name, name, NAME_LEN - 1);
    it->name[NAME_LEN - 1] = '\0';
    return 1;
}

/*
 * Add `amount` units to an existing SKU's on-hand count. Returns the new
 * quantity, or -1 if the SKU is unknown or `amount` is negative.
 */
int32_t inventory_restock(Inventory *inv, int32_t sku, int32_t amount) {
    if (amount < 0)
        return -1;
    int idx = inventory_find(inv, sku);
    if (idx < 0)
        return -1;
    inv->items[idx].quantity += amount;
    return inv->items[idx].quantity;
}

/*
 * Fulfill a sale of `amount` units. Returns the remaining quantity on success,
 * or -1 if the SKU is unknown, the amount is non-positive, or there is
 * insufficient stock (the sale is then rejected atomically).
 */
int32_t inventory_sell(Inventory *inv, int32_t sku, int32_t amount) {
    if (amount <= 0)
        return -1;
    int idx = inventory_find(inv, sku);
    if (idx < 0)
        return -1;
    if (inv->items[idx].quantity < amount)
        return -1;                 /* not enough on hand to satisfy order */
    inv->items[idx].quantity -= amount;
    return inv->items[idx].quantity;
}

/*
 * Compute the total retail value of all stock: sum over items of
 * quantity * unit_price_cents, in cents. O(count). Uses 64-bit accumulation to
 * avoid overflow on large warehouses.
 */
int64_t inventory_total_value(const Inventory *inv) {
    int64_t total = 0;
    for (int i = 0; i < inv->count; i++) {
        total += (int64_t)inv->items[i].quantity *
                 inv->items[i].unit_price_cents;
    }
    return total;
}

/*
 * Collect the SKUs of every item at or below its reorder level into the
 * caller-provided `out` buffer (capacity `cap`). Returns the number of SKUs
 * written, which may be less than the true count if `cap` is exceeded.
 */
int inventory_low_stock(const Inventory *inv, int32_t *out, int cap) {
    int n = 0;
    for (int i = 0; i < inv->count && n < cap; i++) {
        if (inv->items[i].quantity <= inv->items[i].reorder_level)
            out[n++] = inv->items[i].sku;
    }
    return n;
}
