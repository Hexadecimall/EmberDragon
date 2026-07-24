#include <cstdio>

int fmt(long data, long a, long arr, int n2);
long itoa10(int a, long data);


static const int k = 7;
static const int k2 = 1000;

int main(int argc, char** argv) {
    long ch;
    int ch2;
    int len;
    char buf[64];
    ch = k;
    ch2 = k2;
    len = fmt(buf, "x=%d y=%d z=%d (100%%)", &ch, 3);
    printf("[%s] len=%d\n", buf, len);
    return 0;
}

int fmt(long data, long a, long arr, int n2) {
int n;
int j;
int i;
    n = n2;
    j = 0;
    i = 0;
    while (*a != 0) {
        if (*a != 37) {
            j++;
            data[j] = *a;
        } else {
            a++;
            if (*a == 37) {
                j++;
                data[j] = 37;
            } else {
                if (*a == 100) {
                    if (i < n) {
                        i++;
                        j += itoa10(arr[i], data + j);
                        goto L1;
                    }
                } else {
                    j++;
                    data[j] = 63;
                }
                L1:
            }
        }
        a++;
    }
    data[j] = 0;
    return j;
}

long itoa10(int a, long data) {
    int i;
    int v24;
    int v12;
    int v20;
    char buf[64];
    int j;
    i = 0;
    v24 = (a >= 0 ? 0 : 1);
    if (v24 != 0) {
        v12 = 0 - a;
    } else {
        v12 = a;
    }
    v20 = v12;
    do {
        i++;
        buf[i] = v20 - v20 / 10 * 10 + 48;
        v20 /= 10;
    } while (v20 != 0);
    j = 0;
    if (v24 != 0) {
        j++;
        data[j] = 45;
    }
    while (i > 0) {
        i--;
        j++;
        data[j] = buf[i - 1];
    }
    return j;
}

