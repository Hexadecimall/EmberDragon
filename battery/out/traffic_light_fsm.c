#include <cstdio>

long name_for(int a);
void advance(struct Counter* obj2);
long duration_for(int a);


static const char str[] = "?";

int main(int argc, char** argv) {
    int i;
    long v24;
    long v32;
    char __addr0[64];
    i = 0;
    while (i < 12) {
        v24 = i;
        v32 = __addr0;
        printf("t=%2d  %s\n", v24, name_for(0));
        advance(v32);
        i++;
    }
    printf("full cycles completed: %d\n", 0);
    return 0;
}

long name_for(int a) {
    switch (a) {
    case 0: {
        return "RED";
    }
    case 1: {
        return "GREEN";
    }
    case 2: {
        return "YELLOW";
    }
    default: {
        return str;
    }
    }
}

struct Counter {
    int total;
    int count;
    int count2;
};
void advance(struct Counter* obj2) {
    struct Counter* obj;
    int count3;
    obj = obj2;
    obj->count++;
    count3 = obj->count;
    if (count3 >= duration_for(obj->total)) {
        obj->count = 0;
        if (obj->total == 2) {
            obj->count2++;
        }
        obj->total = obj->total + 1 - (obj->total + 1) / 3 * 3;
        return;
    }
    return;
}

long duration_for(int a) {
    switch (a) {
    case 0: {
        return 3;
    }
    case 1: {
        return 2;
    }
    case 2: {
        return 1;
    }
    default: {
        return 1;
    }
    }
}

