#include <cstdint>
#include <cstring>

class Inventory {
public:
    struct Item {
        int32_t sku;
        char name[24];
        int32_t quantity;
        int32_t reorder_level;
        int64_t unit_price_cents;
    };

    static const int CAPACITY = 128;

    Inventory() : item_count(0) {}

    int add_item(int32_t sku, const char *name, int32_t qty, int32_t reorder, int64_t price) {
        if (item_count >= CAPACITY) {
            return -1;
        }
        if (lookup(sku) != nullptr) {
            return -2;
        }
        Item *it = &items[item_count];
        it->sku = sku;
        it->quantity = qty;
        it->reorder_level = reorder;
        it->unit_price_cents = price;
        int i = 0;
        while (name[i] != '\0' && i < 23) {
            it->name[i] = name[i];
            i++;
        }
        it->name[i] = '\0';
        item_count++;
        return 0;
    }

    Item *lookup(int32_t sku) {
        for (int i = 0; i < item_count; i++) {
            if (items[i].sku == sku) {
                return &items[i];
            }
        }
        return nullptr;
    }

    int receive_stock(int32_t sku, int32_t amount) {
        Item *it = lookup(sku);
        if (it == nullptr || amount <= 0) {
            return -1;
        }
        it->quantity += amount;
        return 0;
    }

    int fulfill_order(int32_t sku, int32_t amount) {
        Item *it = lookup(sku);
        if (it == nullptr || amount <= 0) {
            return -1;
        }
        if (it->quantity < amount) {
            return -2;
        }
        it->quantity -= amount;
        return 0;
    }

    int count_below_reorder() {
        int low = 0;
        for (int i = 0; i < item_count; i++) {
            if (items[i].quantity <= items[i].reorder_level) {
                low++;
            }
        }
        return low;
    }

    int64_t inventory_value() {
        int64_t total = 0;
        for (int i = 0; i < item_count; i++) {
            total += (int64_t)items[i].quantity * items[i].unit_price_cents;
        }
        return total;
    }

private:
    Item items[CAPACITY];
    int item_count;
};
