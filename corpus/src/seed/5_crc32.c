#include <stdint.h>
#include <stddef.h>

#define CRC_POLYNOMIAL 0xEDB88320u
#define CRC_TABLE_SIZE 256

typedef struct CrcContext {
    uint32_t table[CRC_TABLE_SIZE];
    uint32_t running;
    int initialized;
} CrcContext;

void crc_build_table(CrcContext *ctx) {
    uint32_t index, crc;
    int bit;
    for (index = 0; index < CRC_TABLE_SIZE; index++) {
        crc = index;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ CRC_POLYNOMIAL;
            } else {
                crc = crc >> 1;
            }
        }
        ctx->table[index] = crc;
    }
    ctx->running = 0xFFFFFFFFu;
    ctx->initialized = 1;
}

void crc_reset(CrcContext *ctx) {
    ctx->running = 0xFFFFFFFFu;
}

void crc_update(CrcContext *ctx, const uint8_t *data, size_t length) {
    size_t i;
    uint8_t lookup_index;
    uint32_t crc;
    crc = ctx->running;
    for (i = 0; i < length; i++) {
        lookup_index = (uint8_t)(crc ^ data[i]) & 0xFFu;
        crc = (crc >> 8) ^ ctx->table[lookup_index];
    }
    ctx->running = crc;
}

uint32_t crc_finalize(CrcContext *ctx) {
    return ctx->running ^ 0xFFFFFFFFu;
}

uint32_t crc_compute(CrcContext *ctx, const uint8_t *data, size_t length) {
    crc_reset(ctx);
    crc_update(ctx, data, length);
    return crc_finalize(ctx);
}

int crc_verify(CrcContext *ctx, const uint8_t *data, size_t length, uint32_t expected) {
    uint32_t actual;
    actual = crc_compute(ctx, data, length);
    if (actual == expected) {
        return 1;
    }
    return 0;
}

uint16_t checksum_ones_complement(const uint8_t *data, size_t length) {
    uint32_t sum;
    size_t i;
    sum = 0;
    for (i = 0; i + 1 < length; i += 2) {
        sum += ((uint32_t)data[i] << 8) | data[i + 1];
    }
    if (i < length) {
        sum += (uint32_t)data[i] << 8;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
    return (uint16_t)(~sum & 0xFFFFu);
}
