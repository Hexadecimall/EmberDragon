/*
 * shopping_cart.cpp — An e-commerce cart that aggregates line items, applies a
 * percentage discount, and computes tax-inclusive totals in integer cents.
 *
 * The cart deliberately merges duplicate product IDs into a single line item
 * (adding quantities) rather than appending duplicates, so the line count
 * reflects distinct products. All money is whole cents; the discount and tax
 * rates are expressed in basis points (1% = 100 bp) to keep arithmetic integral.
 */

#include <cstdint>

namespace store {

constexpr int MAX_LINES = 32;   /* maximum number of distinct products */

/* A single cart line: one product with an aggregated quantity. */
struct LineItem {
    int32_t product_id;          /* unique product key                 */
    int32_t quantity;            /* units of this product, > 0         */
    int64_t unit_price_cents;    /* price per unit in cents            */
};

/*
 * The cart. Discount and tax are stored as basis points so that, for example,
 * an 8.25% sales tax is the integer 825. Both default to zero.
 */
class ShoppingCart {
public:
    ShoppingCart() : line_count_(0), discount_bp_(0), tax_bp_(0) {}

    /*
     * Add `qty` units of a product at `price`. If the product is already in the
     * cart its quantity is increased in place; otherwise a new line is created.
     * Returns true on success, false if qty <= 0 or the cart is at capacity for
     * a genuinely new product. O(line_count).
     */
    bool add(int32_t product_id, int32_t qty, int64_t price) {
        if (qty <= 0)
            return false;
        for (int i = 0; i < line_count_; i++) {
            if (lines_[i].product_id == product_id) {
                /* Merge into the existing line; refresh the unit price. */
                lines_[i].quantity        += qty;
                lines_[i].unit_price_cents = price;
                return true;
            }
        }
        if (line_count_ >= MAX_LINES)
            return false;            /* no room for a new distinct product */
        LineItem &l = lines_[line_count_++];
        l.product_id       = product_id;
        l.quantity         = qty;
        l.unit_price_cents = price;
        return true;
    }

    /*
     * Remove a product entirely from the cart. Returns true if it was present.
     * The hole is filled by swapping in the last line (order is not preserved).
     */
    bool remove(int32_t product_id) {
        for (int i = 0; i < line_count_; i++) {
            if (lines_[i].product_id == product_id) {
                lines_[i] = lines_[--line_count_];
                return true;
            }
        }
        return false;
    }

    /*
     * Set the cart-wide discount, expressed in basis points (0..10000). Values
     * outside that range are clamped so the discount can never be negative or
     * exceed 100%.
     */
    void set_discount_bp(int32_t bp) {
        if (bp < 0)        bp = 0;
        if (bp > 10000)    bp = 10000;
        discount_bp_ = bp;
    }

    /* Set the sales-tax rate in basis points; negative values clamp to zero. */
    void set_tax_bp(int32_t bp) {
        tax_bp_ = (bp < 0) ? 0 : bp;
    }

    /*
     * Sum of quantity * unit_price across all lines, before discount or tax.
     * Returns cents. O(line_count).
     */
    int64_t subtotal() const {
        int64_t sum = 0;
        for (int i = 0; i < line_count_; i++)
            sum += (int64_t)lines_[i].quantity * lines_[i].unit_price_cents;
        return sum;
    }

    /*
     * Final amount due in cents: subtotal reduced by the discount, then tax
     * applied to the discounted base. Basis-point math is done as (x * bp)/10000
     * with truncation toward zero, matching how most billing systems round.
     */
    int64_t total() const {
        int64_t base      = subtotal();
        int64_t discount  = base * discount_bp_ / 10000;
        int64_t taxable   = base - discount;
        int64_t tax       = taxable * tax_bp_ / 10000;
        return taxable + tax;
    }

    /* Total number of physical units across every line. O(line_count). */
    int32_t item_count() const {
        int32_t n = 0;
        for (int i = 0; i < line_count_; i++)
            n += lines_[i].quantity;
        return n;
    }

    /* Number of distinct product lines currently in the cart. */
    int line_count() const { return line_count_; }

private:
    LineItem lines_[MAX_LINES];
    int      line_count_;
    int32_t  discount_bp_;   /* discount rate in basis points */
    int32_t  tax_bp_;        /* tax rate in basis points      */
};

} // namespace store
