#include <stdint.h>
#include <stddef.h>

class VarintBuffer {
public:
    static const int CAPACITY = 256;

    VarintBuffer() : position(0), readCursor(0) {
        for (int i = 0; i < CAPACITY; i++) {
            storage[i] = 0;
        }
    }

    int encodeUnsigned(uint64_t value) {
        int written = 0;
        while (value >= 0x80) {
            if (position >= CAPACITY) {
                return -1;
            }
            storage[position++] = (uint8_t)((value & 0x7F) | 0x80);
            value >>= 7;
            written++;
        }
        if (position >= CAPACITY) {
            return -1;
        }
        storage[position++] = (uint8_t)(value & 0x7F);
        written++;
        return written;
    }

    int encodeSigned(int64_t value) {
        uint64_t zigzag = ((uint64_t)(value << 1)) ^ (uint64_t)(value >> 63);
        return encodeUnsigned(zigzag);
    }

    uint64_t decodeUnsigned(int *consumed) {
        uint64_t result = 0;
        int shift = 0;
        int count = 0;
        while (readCursor < position) {
            uint8_t byte = storage[readCursor++];
            result |= ((uint64_t)(byte & 0x7F)) << shift;
            count++;
            if ((byte & 0x80) == 0) {
                break;
            }
            shift += 7;
        }
        if (consumed) {
            *consumed = count;
        }
        return result;
    }

    int64_t decodeSigned(int *consumed) {
        uint64_t raw = decodeUnsigned(consumed);
        return (int64_t)(raw >> 1) ^ -(int64_t)(raw & 1);
    }

    int sizeOf(uint64_t value) {
        int bytes = 1;
        while (value >= 0x80) {
            value >>= 7;
            bytes++;
        }
        return bytes;
    }

    void rewind() {
        readCursor = 0;
    }

    int bytesUsed() const {
        return position;
    }

private:
    uint8_t storage[CAPACITY];
    int position;
    int readCursor;
};
