#include <cstring>
#include <iostream>
#include <string>

using namespace std;

int edit_distance(long a, long b);


static const char str[] = "\n";

int main(int argc, char** argv) {
char buf[64];
int n;
int i;
long v48;
long v40;
long v32;
long v24;
int distance;
long v8;
    memcpy(buf, "@*", 64);
    n = 4;
    i = 0;
    while (i < n) {
        std::string v96 = buf[(long)(i) << 4];
        std::string v72 = (buf + (i << 4))[8];
        v48 = cout << v96;
        v40 = v48 << " -> ";
        v32 = v40 << v72;
        v24 = v32 << " : ";
        distance = edit_distance(&v96, &v72);
        v8 = v24 << distance;
        i++;
    }
    return 0;
}

int edit_distance(std::string const& a, std::string const& b) {
    int v156;
    int v152;
    char buf2[64];
    char buf[64];
    int k;
    int v64;
    int j;
    int v60;
    int i;
    int v56;
    int v52;
    int v76;
    int v72;
    int v68;
    long v40;
    int* obj;
    int v12;
    v156 = a.size();
    v152 = b.size();
    vector(buf2, v152 + 1);
    vector(buf, v152 + 1);
    k = 0;
    while (k <= v152) {
        v64 = k;
        *operator_index(buf2, k) = v64;
        k++;
    }
    j = 1;
    while (j <= v156) {
        v60 = j;
        *operator_index(buf, 0) = v60;
        i = 1;
        while (i <= v152) {
            v56 = *a[j - 1];
            if (v56 == *b[i - 1]) {
                v52 = *operator_index(buf2, i - 1);
                *operator_index(buf, i) = v52;
            } else {
                v76 = *operator_index(buf2, i) + 1;
                v72 = *operator_index(buf, i - 1) + 1;
                v68 = *operator_index(buf2, i - 1) + 1;
                v40 = min(&v72, &v68);
                obj = min(&v76, v40);
                v12 = *obj;
                *operator_index(buf, i) = v12;
            }
            i++;
        }
        swap(buf2, buf);
        j++;
    }
    return (*operator_index(buf2, v152));
}

