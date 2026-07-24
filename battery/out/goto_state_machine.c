#include <cstdio>

const char* s_input_s = "input: \"%s\"\n";
const char* s_idents_d_numbers_d_o = "idents=%d numbers=%d ops=%d\n";

int main(int argc, char** argv) {
    char* obj;
    int v36;
    int i;
    int v28;
    char v27;
    long t5;
    long t6;
    obj = "ab12 + 34cd";
    v36 = 0;
    i = 0;
    v28 = 0;
    v27 = *obj;
    while (v27 == 0) {
        t5 = printf(&s_input_s, "ab12 + 34cd");
        t6 = printf(&s_idents_d_numbers_d_o, v36, i, v28);
        return 0;
        i++;
        while (isdigit(*obj) != 0) {
            obj++;
        }
    }
    t5 = printf(&s_input_s, "ab12 + 34cd");
    t6 = printf(&s_idents_d_numbers_d_o, v36, i, v28);
    return 0;
}

