/*
 * sequence_generator.cpp — A configurable polymorphic sequence generator.
 *
 * Defines an abstract Sequence interface plus three concrete generators
 * (arithmetic, geometric, and an in-place Fibonacci-like recurrence). Each
 * advances lazily on demand, so consumers can pull arbitrarily many terms
 * through a uniform virtual interface. Strictly integer arithmetic.
 */

#include <cstdint>
#include <cstdlib>

/*
 * Abstract base for any integer sequence that can be advanced term by term.
 *
 * Subclasses implement next() to return the current term and move forward,
 * and reset() to return to the initial state.
 */
class Sequence {
public:
    virtual ~Sequence() {}

    /*
     * Return the current term and advance to the next.
     *
     * return: the term that was current before advancing.
     */
    virtual int64_t next() = 0;

    /*
     * Restore the sequence to its starting state so it can replay.
     */
    virtual void reset() = 0;
};

/*
 * Arithmetic progression: start, start+delta, start+2*delta, ...
 */
class ArithmeticSequence : public Sequence {
public:
    /*
     * start:  the first term.
     * delta:  the constant difference added each step (may be negative).
     */
    ArithmeticSequence(int64_t start, int64_t delta)
        : start_(start), delta_(delta), value_(start) {}

    /*
     * return: the current term, then steps forward by delta. O(1).
     */
    int64_t next() override {
        int64_t out = value_;
        value_ += delta_;
        return out;
    }

    void reset() override {
        value_ = start_;
    }

private:
    int64_t start_; /* original first term, kept for reset */
    int64_t delta_; /* constant step */
    int64_t value_; /* current term */
};

/*
 * Geometric progression: start, start*ratio, start*ratio^2, ...
 */
class GeometricSequence : public Sequence {
public:
    /*
     * start:  the first term.
     * ratio:  the constant multiplier applied each step.
     */
    GeometricSequence(int64_t start, int64_t ratio)
        : start_(start), ratio_(ratio), value_(start) {}

    /*
     * return: the current term, then multiplies by ratio. O(1). Overflows
     *         once a term exceeds the 64-bit range — the caller bounds use.
     */
    int64_t next() override {
        int64_t out = value_;
        value_ *= ratio_;
        return out;
    }

    void reset() override {
        value_ = start_;
    }

private:
    int64_t start_; /* original first term */
    int64_t ratio_; /* constant multiplier */
    int64_t value_; /* current term */
};

/*
 * General two-term linear recurrence: t = a*prev + b*prev2.
 *
 * With a = b = 1 and the right seeds this is the Fibonacci sequence; other
 * coefficients yield Lucas numbers, Pell numbers, and similar families.
 */
class RecurrenceSequence : public Sequence {
public:
    /*
     * first:  the 0th term.
     * second: the 1st term.
     * a:      coefficient on the immediately previous term.
     * b:      coefficient on the term before that.
     */
    RecurrenceSequence(int64_t first, int64_t second, int64_t a, int64_t b)
        : first_(first), second_(second), a_(a), b_(b),
          prev2_(first), prev_(second) {}

    /*
     * Emit the next term of the recurrence.
     *
     * Returns the older of the two stored terms, then folds the pair forward
     * one position so the next call continues the sequence.
     *
     * return: the next term. O(1) time, O(1) state.
     */
    int64_t next() override {
        int64_t out = prev2_;
        /* Combine the two retained terms into the new leading term. */
        int64_t combined = a_ * prev_ + b_ * prev2_;
        prev2_ = prev_;
        prev_ = combined;
        return out;
    }

    void reset() override {
        prev2_ = first_;
        prev_ = second_;
    }

private:
    int64_t first_, second_; /* seed terms, retained for reset */
    int64_t a_, b_;          /* recurrence coefficients */
    int64_t prev2_, prev_;   /* the two most recent terms (older, newer) */
};

/*
 * Pull `count` terms from any Sequence into a buffer.
 *
 * Works through the abstract interface, so it handles every generator above
 * identically — the point of the virtual design.
 *
 * seq:    the sequence to drive (advanced by `count` terms).
 * out:    destination array, at least `count` elements.
 * count:  how many terms to take.
 * return: the number of terms written (equal to count).
 */
uint32_t take(Sequence *seq, int64_t *out, uint32_t count) {
    uint32_t written = 0;
    while (written < count) {
        out[written++] = seq->next();
    }
    return written;
}

/*
 * Sum the next `count` terms of a sequence.
 *
 * Consumes terms via take()'s same protocol but accumulates instead of
 * storing, requiring no buffer.
 *
 * seq:    the sequence to drive.
 * count:  number of terms to add.
 * return: the integer sum of those terms.
 */
int64_t sum_terms(Sequence *seq, uint32_t count) {
    int64_t total = 0;
    for (uint32_t i = 0; i < count; i++) {
        total += seq->next();
    }
    return total;
}
