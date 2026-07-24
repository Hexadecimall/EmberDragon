/*
 * robinhood.cpp — an open-addressing hash set using Robin Hood hashing.
 *
 * Robin Hood hashing is a linear-probing variant that equalizes probe lengths:
 * when inserting, an element that has travelled far from its ideal slot can
 * "steal" the position of a richer element that is closer to home, swapping
 * places and continuing. This bounds the variance of probe distances, keeping
 * worst-case lookups short even at high load factors. This set stores 32-bit
 * integer keys.
 */

#include <cstdint>
#include <cstddef>
#include <cstdlib>

/* One bucket. An empty bucket is signalled by probe_distance == EMPTY; an
 * occupied bucket records how far it sits from its ideal slot so insertions can
 * compare "richness". */
struct Bucket {
    uint32_t key;
    uint32_t probe_distance; /* distance from the ideal slot, or EMPTY */
};

/* Sentinel probe distance marking a free bucket. Real distances are small. */
static const uint32_t EMPTY = 0xFFFFFFFFu;

/* A Robin Hood hash set over a power-of-two bucket array. */
class RobinHoodSet {
public:
    /*
     * Construct a set with the given initial capacity (rounded up to a power of
     * two, minimum 8). Allocates the bucket array; aborts via the empty-marker
     * if allocation fails (callers may also check capacity() afterward).
     */
    explicit RobinHoodSet(size_t initial_capacity = 8)
        : buckets_(nullptr), capacity_(8), size_(0) {
        while (capacity_ < initial_capacity) capacity_ <<= 1;
        buckets_ = static_cast<Bucket *>(malloc(capacity_ * sizeof(Bucket)));
        if (buckets_) clear_buckets();
    }

    /* Destructor: release the bucket array. */
    ~RobinHoodSet() { free(buckets_); }

    /*
     * Insert a key into the set.
     *
     * Returns true if the key was newly inserted, false if it was already
     * present. Triggers a doubling resize when the load factor passes ~90%.
     * Amortized O(1); the Robin Hood swap bounds individual probe lengths.
     */
    bool insert(uint32_t key) {
        if ((size_ + 1) * 10 >= capacity_ * 9) grow();
        return insert_into(buckets_, capacity_, key, /*count_size=*/true);
    }

    /*
     * Test membership.
     *
     * Returns true if key is in the set. The scan can stop early: because Robin
     * Hood keeps every element within its probe distance of home, once we reach
     * a bucket whose probe_distance is smaller than how far we have travelled,
     * the key cannot be present. Average O(1).
     */
    bool contains(uint32_t key) const {
        size_t mask = capacity_ - 1;
        size_t idx = hash(key) & mask;
        uint32_t dist = 0;
        for (;;) {
            const Bucket &b = buckets_[idx];
            if (b.probe_distance == EMPTY) return false; /* hit a hole */
            if (b.probe_distance < dist) return false;   /* we are too poor */
            if (b.key == key) return true;
            idx = (idx + 1) & mask;
            dist++;
        }
    }

    /* Number of keys currently stored. */
    size_t size() const { return size_; }

    /* Number of buckets allocated (a power of two). */
    size_t capacity() const { return capacity_; }

private:
    Bucket *buckets_;
    size_t  capacity_;
    size_t  size_;

    /*
     * Avalanche a 32-bit key so sequential or aligned keys disperse well.
     * Returns the mixed hash. O(1).
     */
    static uint32_t hash(uint32_t key) {
        key ^= key >> 16;
        key *= 0x85EBCA6Bu;
        key ^= key >> 13;
        key *= 0xC2B2AE35u;
        key ^= key >> 16;
        return key;
    }

    /* Mark every bucket in the current array empty. O(capacity). */
    void clear_buckets() {
        for (size_t i = 0; i < capacity_; i++) buckets_[i].probe_distance = EMPTY;
    }

    /*
     * Core Robin Hood insertion into a specific bucket array.
     *
     * Carries a "displaced" element forward: whenever the element being placed
     * is richer (smaller probe distance) than the occupant, they swap and the
     * occupant continues the search. count_size is false during a resize, where
     * size_ is set explicitly by the caller, and true for normal inserts.
     * Returns true if a distinct key was placed, false on a duplicate.
     */
    static bool insert_into(Bucket *buckets, size_t capacity, uint32_t key,
                            bool count_size_unused) {
        (void)count_size_unused;
        size_t mask = capacity - 1;
        size_t idx = hash(key) & mask;
        uint32_t dist = 0;
        uint32_t cur_key = key;

        for (;;) {
            Bucket &b = buckets[idx];
            if (b.probe_distance == EMPTY) {
                b.key = cur_key;
                b.probe_distance = dist;
                return true;
            }
            if (b.probe_distance == dist && b.key == cur_key) {
                return false; /* duplicate of the original key */
            }
            if (b.probe_distance < dist) {
                /* The occupant is closer to home (richer); take its slot and
                 * carry the evicted element onward — the Robin Hood swap. */
                uint32_t tmp_key = b.key;
                uint32_t tmp_dist = b.probe_distance;
                b.key = cur_key;
                b.probe_distance = dist;
                cur_key = tmp_key;
                dist = tmp_dist;
            }
            idx = (idx + 1) & mask;
            dist++;
        }
    }

    /*
     * Double the capacity and re-insert every live key.
     *
     * A fresh array resets all probe distances, so chains are rebuilt cleanly.
     * If allocation fails the old array is kept and the set is left unchanged.
     * O(new capacity).
     */
    void grow() {
        size_t new_cap = capacity_ << 1;
        Bucket *fresh = static_cast<Bucket *>(malloc(new_cap * sizeof(Bucket)));
        if (!fresh) return;
        for (size_t i = 0; i < new_cap; i++) fresh[i].probe_distance = EMPTY;

        for (size_t i = 0; i < capacity_; i++) {
            if (buckets_[i].probe_distance != EMPTY) {
                insert_into(fresh, new_cap, buckets_[i].key, false);
            }
        }
        free(buckets_);
        buckets_ = fresh;
        capacity_ = new_cap;
        /* size_ is unchanged: we re-inserted exactly the same keys. */
    }
};
