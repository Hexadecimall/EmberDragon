#include <cstdio>
#include <cstring>

static const int table_init[11] = {
    0, 6, 0, 7, 3, 5, 0, 2, 2, 6, 7 
};

int main(int argc, char** argv) {
    int table[11];
    int j;
    int i;
    int v28;
    int op;
    int v20;
    int ptr[16];
    memcpy(table, table_init, 44);
    j = 0;
    i = 0;
    v28 = 1;
    while (v28 != 0) {
        i++;
        op = table[i];
        v20 = op;
        switch (v20) {
        case 0: {
            i++;
            j++;
            ptr[j] = table[i];
            break;
        }
        case 1: {
            ptr[j - 2] = ptr[j - 2] + ptr[j - 1];
            j--;
            break;
        }
        case 2: {
            ptr[j - 2] = ptr[j - 2] - ptr[j - 1];
            j--;
            break;
        }
        case 3: {
            ptr[j - 2] = ptr[j - 2] * ptr[j - 1];
            j--;
            break;
        }
        case 4: {
            ptr[j - 1] = 0 - ptr[j - 1];
            break;
        }
        case 5: {
            ptr[j] = ptr[j - 1];
            j++;
            break;
        }
        case 6: {
            printf("top = %d\n", ptr[j - 1]);
            break;
        }
        case 7: {
            v28 = 0;
            break;
        }
        default: {
            fprintf(stderr, "bad op %d\n", op);
            v28 = 0;
            break;
        }
        }
    }
    printf("final sp = %d\n", j);
    return 0;
}

