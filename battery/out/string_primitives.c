#include <cstdio>
#include <cstring>

void my_strcpy(long a, char* obj3);
int my_strlen(long a);
int my_strcmp(char* obj3, char* obj4);


int main(int argc, char** argv) {
    int i;
    char buf[64];
    char v80;
    // neon:  ldr    value0, [v121, #0x0]  // = 0xec06000000001000f206000000001000
    // neon:  ldr    value0, [v121, #0x10]  // = 0xf706000000001000ec06000000001000
    i = 0;
    while (i < 4) {
        my_strcpy(buf, (&v80)[i << 3]);
        strlen = my_strlen(buf);
        strcmp = my_strcmp(buf, "alpha");
        printf("%-6s len=%d cmp_alpha=%d\n", buf, strlen, strcmp);
        i++;
    }
    return 0;
}

void my_strcpy(long a, char* obj3) {
    long result;
    char* obj2;
    result = a;
    obj2 = result;
    do {
        obj3++;
        obj2++;
        *obj2 = *obj3;
    } while (*obj3 != 0);
    return;
}

int my_strlen(long a) {
    while (*a != 0) {
        a++;
    }
    return (a - a);
}

int my_strcmp(char* obj3, char* obj4) {
    int v12;
    v12 = 0;
    while (*obj3 != 0) {
        v12 = (*obj3 != *obj4 ? 0 : 1);
        if (v12) {
            obj3++;
            obj4++;
        }
    }
    return (*obj3 - *obj4);
}

