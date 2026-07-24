#include <cstring>
#include <iostream>

using namespace std;

static const long data_init[8] = {
    25, 25, 4294967296, 25, 4294967296, 50, 50, 4294967296 
};
static const char str[] = "\n";

struct Pair {
    int value;
    char flag;
};
int main(int argc, char** argv) {
    int ret;
    int v236;
    int sum;
    int i;
    long data[8];
    long v184;
    char buf[64];
    long v168;
    long v160;
    struct Pair* obj;
    long v120;
    long value2;
    long v104;
    long v96;
    int v136;
    long v88;
    long v80;
    long v72;
    long v64;
    long v56;
    long v48;
    long v40;
    long v32;
    long v24;
    long v16;
    ret = 0;
    v236 = 0;
    sum = 0;
    i = 0;
    memcpy(data, data_init, 64);
    v184 = data;
    vector(buf, v184, 8);
    v168 = begin(buf);
    v160 = end(buf);
    while (v168 != v160) {
        obj = operator_mul&v168;
        if (obj->value > 0) {
            sum += obj->value;
            v236 = 1;
            v120 = cout << "inserted ";
            value2 = v120 << obj->value;
            v104 = value2 << "c, credit=";
            v96 = v104 << sum;
            v96 << str;
        }
        if (obj->flag) {
            if (v236 == 1) {
                if (sum >= 75) {
                    v236 = 2;
                    v136 = sum - 75;
                    i++;
                    v88 = cout << "DISPENSE #";
                    v80 = v88 << i;
                    v72 = v80 << " change=";
                    v64 = v72 << v136;
                    v64 << "c\n";
                    sum = 0;
                    v236 = 0;
                    goto L1;
                }
            } else {
                v56 = cout << "rejected: need ";
                v48 = v56 << 75 - sum;
                v48 << "c more\n";
            }
            L1:
        }
        operator_inc&v168;
    }
    v40 = cout << "total dispensed=";
    v32 = v40 << i;
    v24 = v32 << " final credit=";
    v16 = v24 << sum;
    v16 << str;
    ret = 0;
    return ret;
}

