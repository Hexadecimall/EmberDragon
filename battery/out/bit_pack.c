#include <cstdio>

int pack(int arr, char a);
int unpack(int a, char* obj2);


int main(int argc, char** argv) {
    int result;
    int checked;
    int i;
    int v44;
    char v45;
    char v46;
    char v47;
    char v43;
    long v24;
    int v36;
    int v19;
    char v23;
    char v20;
    char v21;
    char v22;
    result = 0;
    checked = 0;
    i = 0;
    while (i < 64) {
        v44 = i << 2;
        v45 = 255 - i;
        v46 = i ^ 42;
        v47 = i & 31;
        v43 = i & 7;
        v24 = v44;
        v36 = pack(v24, v43);
        v19 = unpack(v36, &v23);
        checked++;
        if (!(v19 == v44 && v20 == v45 && v21 == v46 && v22 == v47 && v23 == v43)) {
            result++;
        }
        i++;
    }
    printf("checked=%u mismatches=%u\n", checked, result);
    return result;
}

int pack(int arr, char a) {
    char v13;
    char v14;
    char v15;
    return (v13 << 16 | arr << 24 | v14 << 8 | (v15 & 31) << 3 | a & 7);
}

int unpack(int a, char* obj2) {
    int result;
    result = (unsigned)(a) >> 24 & 255;
    (unsigned)(a) >> 16 & 255;
    (unsigned)(a) >> 8 & 255;
    (unsigned)(a) >> 3 & 31;
    *obj2 = a & 7;
    return result;
}

