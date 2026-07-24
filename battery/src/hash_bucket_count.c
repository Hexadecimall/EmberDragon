/* Open-addressing-free bucket histogram: hash keys into fixed buckets, count loads. */
#include <stdio.h>
#include <stdint.h>

#define NBUCKETS 7

static unsigned hash_key(uint32_t key) {
    /* simple multiplicative mix, then modulo bucket count */
    uint32_t h = key * 2654435761u;
    h ^= h >> 15;
    return (unsigned)(h % NBUCKETS);
}

int main(void) {
    unsigned buckets[NBUCKETS] = {0};
    uint32_t keys[] = {3, 14, 15, 92, 65, 35, 89, 79, 32, 38, 46, 26};
    int nkeys = (int)(sizeof(keys) / sizeof(keys[0]));

    for (int i = 0; i < nkeys; i++) {
        unsigned b = hash_key(keys[i]);
        buckets[b]++;
    }

    unsigned max = 0, total = 0;
    int max_idx = 0;
    for (int b = 0; b < NBUCKETS; b++) {
        total += buckets[b];
        if (buckets[b] > max) { max = buckets[b]; max_idx = b; }
        printf("bucket[%d]=%u\n", b, buckets[b]);
    }
    printf("total=%u busiest=bucket[%d](%u)\n", total, max_idx, max);
    return 0;
}
