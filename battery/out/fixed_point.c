#include <cstdio>

int fx_div(int a, int b);
int fx_from_int(int arr);
int fx_mul(int a, int b);
int fx_sqrt(int a);
int fx_to_int(int a);


int main(int argc, char** argv) {
    int sum;
    int i;
    int from;
    int div;
    int mul;
    int root;
    int area_int;
    sum = 0;
    i = 1;
    while (i <= 50) {
        from = fx_from_int(i);
        div = fx_div(from, fx_from_int(7));
        mul = fx_mul(205887, fx_mul(div, div));
        sum += mul;
        i++;
    }
    root = fx_sqrt(sum);
    area_int = fx_to_int(sum);
    printf("area_int=%d side_int=%d frac16=%d\n", area_int, fx_to_int(root), root);
    return (fx_to_int(root) & 127);
}

int fx_div(int a, int b) {
    return (((long)(a) << 16) / b);
}

int fx_from_int(int arr) {
    return (arr << 16);
}

int fx_mul(int a, int b) {
    return ((long)(a) * (long)(b) >> 16);
}

int fx_sqrt(int a) {
    int result;
    int v8;
    int ret;
    int i;
    int v4;
    if (a <= 0) {
        return 0;
    } else {
        if (a > fx_from_int(1)) {
            v8 = a;
        } else {
            v8 = fx_from_int(1);
        }
        ret = v8;
        i = 0;
        while (i < 24) {
            v4 = ret;
            ret = v4 + fx_div(a, ret) >> 1;
            i++;
        }
        return ret;
    }
    return result;
}

int fx_to_int(int a) {
    return (a >> 16);
}

