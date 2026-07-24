#include <stdlib.h>

/* Top-down mergesort of timestamped log records by timestamp. */

typedef struct LogRecord {
    long timestamp;
    int sourceId;
} LogRecord;

static void mergeHalves(LogRecord *records, LogRecord *scratch,
                        int begin, int middle, int end) {
    int leftCursor = begin;
    int rightCursor = middle;
    int outCursor = begin;

    while (leftCursor < middle && rightCursor < end) {
        if (records[leftCursor].timestamp <= records[rightCursor].timestamp) {
            scratch[outCursor] = records[leftCursor];
            leftCursor = leftCursor + 1;
        } else {
            scratch[outCursor] = records[rightCursor];
            rightCursor = rightCursor + 1;
        }
        outCursor = outCursor + 1;
    }
    while (leftCursor < middle) {
        scratch[outCursor] = records[leftCursor];
        leftCursor = leftCursor + 1;
        outCursor = outCursor + 1;
    }
    while (rightCursor < end) {
        scratch[outCursor] = records[rightCursor];
        rightCursor = rightCursor + 1;
        outCursor = outCursor + 1;
    }
    for (int i = begin; i < end; i++) {
        records[i] = scratch[i];
    }
}

static void mergeSortSpan(LogRecord *records, LogRecord *scratch,
                          int begin, int end) {
    if (end - begin <= 1) {
        return;
    }
    int middle = begin + (end - begin) / 2;
    mergeSortSpan(records, scratch, begin, middle);
    mergeSortSpan(records, scratch, middle, end);
    mergeHalves(records, scratch, begin, middle, end);
}

int sortLogRecords(LogRecord *records, int count) {
    if (count <= 1) {
        return 0;
    }
    LogRecord *scratch = (LogRecord *)malloc((size_t)count * sizeof(LogRecord));
    if (scratch == NULL) {
        return -1;
    }
    mergeSortSpan(records, scratch, 0, count);
    free(scratch);
    return 0;
}

long earliestTimestamp(const LogRecord *records, int count) {
    if (count == 0) {
        return -1;
    }
    long best = records[0].timestamp;
    for (int i = 1; i < count; i++) {
        if (records[i].timestamp < best) {
            best = records[i].timestamp;
        }
    }
    return best;
}
