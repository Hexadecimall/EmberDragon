#include <stdint.h>

/* Run-length encoder and decoder operating on byte buffers. */

class RunLengthCodec {
public:
    struct Run {
        unsigned char value;
        int count;
    };

    static int encode(const unsigned char *input, int input_len,
                      unsigned char *output, int output_capacity) {
        int written = 0;
        int index = 0;
        while (index < input_len) {
            unsigned char current = input[index];
            int run = 1;
            while (index + run < input_len && input[index + run] == current && run < 255) {
                run++;
            }
            if (written + 2 > output_capacity) {
                return -1;
            }
            output[written] = (unsigned char)run;
            output[written + 1] = current;
            written += 2;
            index += run;
        }
        return written;
    }

    static int decode(const unsigned char *input, int input_len,
                      unsigned char *output, int output_capacity) {
        int written = 0;
        int index = 0;
        while (index + 1 < input_len) {
            int run = input[index];
            unsigned char value = input[index + 1];
            if (written + run > output_capacity) {
                return -1;
            }
            for (int repeat = 0; repeat < run; repeat++) {
                output[written] = value;
                written++;
            }
            index += 2;
        }
        return written;
    }

    static int decoded_length(const unsigned char *input, int input_len) {
        int total = 0;
        int index = 0;
        while (index + 1 < input_len) {
            total += input[index];
            index += 2;
        }
        return total;
    }

    static int count_runs(const unsigned char *input, int input_len) {
        int runs = 0;
        int index = 0;
        while (index < input_len) {
            unsigned char current = input[index];
            int run = 1;
            while (index + run < input_len && input[index + run] == current && run < 255) {
                run++;
            }
            runs++;
            index += run;
        }
        return runs;
    }
};

int roundtrip_check(const unsigned char *data, int data_len, unsigned char *scratch) {
    unsigned char encoded[512];
    int encoded_len = RunLengthCodec::encode(data, data_len, encoded, 512);
    if (encoded_len < 0) {
        return -1;
    }
    int decoded_len = RunLengthCodec::decode(encoded, encoded_len, scratch, data_len);
    return decoded_len;
}
