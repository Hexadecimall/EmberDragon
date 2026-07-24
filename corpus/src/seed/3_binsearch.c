#include <stdint.h>

/* Binary search variants over a sorted table of user account IDs. */

typedef struct AccountTable {
    const int64_t *ids;
    int length;
} AccountTable;

int findAccountIndex(const AccountTable *table, int64_t targetId) {
    int low = 0;
    int high = table->length - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int64_t current = table->ids[mid];
        if (current == targetId) {
            return mid;
        } else if (current < targetId) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

int lowerBoundIndex(const AccountTable *table, int64_t targetId) {
    int low = 0;
    int high = table->length;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (table->ids[mid] < targetId) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

int upperBoundIndex(const AccountTable *table, int64_t targetId) {
    int low = 0;
    int high = table->length;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (table->ids[mid] <= targetId) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

int countOccurrences(const AccountTable *table, int64_t targetId) {
    int first = lowerBoundIndex(table, targetId);
    int last = upperBoundIndex(table, targetId);
    return last - first;
}

int containsAccount(const AccountTable *table, int64_t targetId) {
    return findAccountIndex(table, targetId) >= 0 ? 1 : 0;
}
