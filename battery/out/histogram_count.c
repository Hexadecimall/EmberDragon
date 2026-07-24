#include <cstring>
#include <iostream>
#include <string>

using namespace std;

static const char str[] = "\n";

int main(int argc, char** argv) {
    int ret;
    int buf[26];
    long v128;
    long v120;
    char v119;
    int sum;
    int v108;
    int j;
    int i;
    long v72;
    long v64;
    long v56;
    int k;
    long v48;
    long v40;
    long v32;
    long v24;
    ret = 0;
    std::string v144 = "the quick brown fox jumps over the lazy dog the end";
    memset(buf, 0, 104);
    v128 = v144.begin();
    v120 = v144.end();
    while (v128 != v120) {
        v119 = *(operator_mul&v128);
        if (v119 >= 97) {
            if (v119 <= 122) {
                buf[v119 - 97] = buf[v119 - 97] + 1;
            }
        }
        operator_inc&v128;
    }
    sum = 0;
    v108 = 0;
    j = 0;
    while (j < 26) {
        sum += buf[j];
        if (!(buf[j] <= v108)) {
            v108 = buf[j];
        }
        j++;
    }
    i = 0;
    while (i < 26) {
        if (buf[i] != 0) {
            v72 = cout << i + 97;
            v64 = v72 << " (";
            v56 = v64 << buf[i];
            v56 << ") ";
            k = 0;
            while (k < buf[i]) {
                cout << 42;
                k++;
            }
            cout << str;
        }
        i++;
    }
    v48 = cout << "total letters: ";
    v40 = v48 << sum;
    v32 = v40 << ", peak: ";
    v24 = v32 << v108;
    v24 << str;
    ret = 0;
    return ret;
}

