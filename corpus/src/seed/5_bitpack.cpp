#include <stdint.h>
#include <stddef.h>

struct BitWriter {
    uint8_t *buffer;
    size_t capacity;
    size_t byte_index;
    int bit_index;
};

struct BitReader {
    const uint8_t *buffer;
    size_t capacity;
    size_t byte_index;
    int bit_index;
};

void writer_init(BitWriter *writer, uint8_t *buffer, size_t capacity) {
    writer->buffer = buffer;
    writer->capacity = capacity;
    writer->byte_index = 0;
    writer->bit_index = 0;
    for (size_t i = 0; i < capacity; i++) {
        buffer[i] = 0;
    }
}

int writer_put_bit(BitWriter *writer, int bit) {
    if (writer->byte_index >= writer->capacity) {
        return -1;
    }
    if (bit) {
        writer->buffer[writer->byte_index] |= (uint8_t)(1 << (7 - writer->bit_index));
    }
    writer->bit_index++;
    if (writer->bit_index == 8) {
        writer->bit_index = 0;
        writer->byte_index++;
    }
    return 0;
}

int writer_put_field(BitWriter *writer, uint32_t value, int width) {
    for (int i = width - 1; i >= 0; i--) {
        int bit = (value >> i) & 1;
        if (writer_put_bit(writer, bit) != 0) {
            return -1;
        }
    }
    return width;
}

size_t writer_total_bits(const BitWriter *writer) {
    return writer->byte_index * 8 + (size_t)writer->bit_index;
}

void reader_init(BitReader *reader, const uint8_t *buffer, size_t capacity) {
    reader->buffer = buffer;
    reader->capacity = capacity;
    reader->byte_index = 0;
    reader->bit_index = 0;
}

int reader_get_bit(BitReader *reader) {
    int bit;
    if (reader->byte_index >= reader->capacity) {
        return -1;
    }
    bit = (reader->buffer[reader->byte_index] >> (7 - reader->bit_index)) & 1;
    reader->bit_index++;
    if (reader->bit_index == 8) {
        reader->bit_index = 0;
        reader->byte_index++;
    }
    return bit;
}

uint32_t reader_get_field(BitReader *reader, int width, int *error) {
    uint32_t value = 0;
    *error = 0;
    for (int i = 0; i < width; i++) {
        int bit = reader_get_bit(reader);
        if (bit < 0) {
            *error = 1;
            return value;
        }
        value = (value << 1) | (uint32_t)bit;
    }
    return value;
}

int rle_pack_bits(const uint8_t *bits, size_t count, BitWriter *writer) {
    size_t i = 0;
    int runs = 0;
    while (i < count) {
        uint8_t current = bits[i];
        size_t run_length = 0;
        while (i < count && bits[i] == current && run_length < 255) {
            run_length++;
            i++;
        }
        if (writer_put_bit(writer, current) != 0) {
            return -1;
        }
        if (writer_put_field(writer, (uint32_t)run_length, 8) < 0) {
            return -1;
        }
        runs++;
    }
    return runs;
}
