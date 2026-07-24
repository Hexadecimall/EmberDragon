/*
 * rational.cpp — Exact rational-number arithmetic over 64-bit integers.
 *
 * Models a fraction numerator/denominator that is always kept in lowest
 * terms with a positive denominator. Supporting exact addition,
 * subtraction, multiplication, division, and comparison lets callers avoid
 * the rounding error of floating point for problems that need precise
 * fractions (probabilities, continued fractions, exact linear algebra).
 */

#include <cstdint>

/*
 * Compute the greatest common divisor of two non-negative magnitudes.
 * Parameters: a, b — values (assumed already non-negative).
 * Returns gcd(a, b); gcd(0, 0) == 0. Iterative Euclid, O(log min(a,b)).
 */
static int64_t gcd_u(int64_t a, int64_t b) {
    while (b != 0) {
        int64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

/*
 * An exact rational number stored as num/den, always normalized: den > 0 and
 * gcd(|num|, den) == 1. The sign lives entirely on the numerator.
 */
class Rational {
public:
    /*
     * Construct a rational n/d.
     * Parameters: n — numerator (default 0); d — denominator (default 1).
     * Notable: a zero denominator is coerced to 1 to keep the object valid
     * instead of trapping. The value is immediately reduced to lowest terms.
     */
    Rational(int64_t n = 0, int64_t d = 1) : num_(n), den_(d) {
        if (den_ == 0)
            den_ = 1;             /* fail safe: treat n/0 as n/1 */
        normalize();
    }

    /* Accessors for the reduced numerator and denominator. */
    int64_t numerator() const { return num_; }
    int64_t denominator() const { return den_; }

    /*
     * Add two rationals exactly.
     * Parameters: other — right-hand operand.
     * Returns the reduced sum. Combines over the common denominator
     * den_*other.den_; result is normalized by the constructor.
     */
    Rational add(const Rational &other) const {
        return Rational(num_ * other.den_ + other.num_ * den_,
                        den_ * other.den_);
    }

    /*
     * Subtract another rational from this one.
     * Parameters: other — value to subtract.
     * Returns the reduced difference.
     */
    Rational sub(const Rational &other) const {
        return Rational(num_ * other.den_ - other.num_ * den_,
                        den_ * other.den_);
    }

    /*
     * Multiply two rationals.
     * Parameters: other — right-hand operand.
     * Returns the reduced product (num1*num2)/(den1*den2).
     */
    Rational mul(const Rational &other) const {
        return Rational(num_ * other.num_, den_ * other.den_);
    }

    /*
     * Divide this rational by another.
     * Parameters: other — divisor.
     * Returns the reduced quotient, computed by multiplying by the
     * reciprocal. Dividing by zero yields this value unchanged as a safe
     * fallback rather than producing an invalid object.
     */
    Rational div(const Rational &other) const {
        if (other.num_ == 0)
            return *this;         /* undefined: leave the operand untouched */
        return Rational(num_ * other.den_, den_ * other.num_);
    }

    /*
     * Order two rationals.
     * Parameters: other — value to compare against.
     * Returns -1, 0, or +1 for less/equal/greater. Cross-multiplies; this is
     * exact because both denominators are positive after normalization.
     */
    int compare(const Rational &other) const {
        int64_t lhs = num_ * other.den_;
        int64_t rhs = other.num_ * den_;
        if (lhs < rhs) return -1;
        if (lhs > rhs) return 1;
        return 0;
    }

private:
    /*
     * Reduce the fraction to lowest terms and force a positive denominator.
     * Returns nothing. Handles the zero value by canonicalizing it to 0/1.
     */
    void normalize() {
        if (num_ == 0) {
            den_ = 1;             /* canonical zero */
            return;
        }
        /* Move any sign from the denominator onto the numerator so den_ > 0. */
        if (den_ < 0) {
            num_ = -num_;
            den_ = -den_;
        }
        /* Divide out the common factor using magnitudes of both parts. */
        int64_t a = num_ < 0 ? -num_ : num_;
        int64_t g = gcd_u(a, den_);
        if (g > 1) {
            num_ /= g;
            den_ /= g;
        }
    }

    int64_t num_;   /* signed numerator */
    int64_t den_;   /* strictly positive denominator */
};
