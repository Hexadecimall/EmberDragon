#include <cstdio>
#include <cstring>

static const int table_init[13] = {
    0, 2, 0, 3, 1, 0, 4, 3, 0, 1, 2, 4, 5 
};

int main(int argc, char** argv) {
    int ret;
    int table[13];
    int j;
    int i;
    int v52;
    int opcode;
    int v20;
    int buf[16];
    int v44;
    int v40;
    int v36;
    int v32;
    int ptr;
    int v24;
    ret = 0;
    memcpy(table, table_init, 52);
    j = 0;
    i = 0;
    v52 = 1;
    while (v52 != 0) {
        i++;
        opcode = table[i];
        v20 = opcode;
        switch (v20) {
        case 0: {
            i++;
            j++;
            buf[j] = table[i];
            break;
        }
        case 1: {
            j--;
            v44 = buf[j - 1];
            j--;
            v40 = buf[j - 1];
            j++;
            buf[j] = v40 + v44;
            printf("ADD -> %d\n", buf[j - 1]);
            break;
        }
        case 2: {
            j--;
            v36 = buf[j - 1];
            j--;
            v32 = buf[j - 1];
            j++;
            buf[j] = v32 - v36;
            printf("SUB -> %d\n", buf[j - 1]);
            break;
        }
        case 3: {
            j--;
            ptr = buf[j - 1];
            j--;
            v24 = buf[j - 1];
            j++;
            buf[j] = v24 * ptr;
            printf("MUL -> %d\n", buf[j - 1]);
            break;
        }
        case 4: {
            printf("TOP = %d\n", buf[j - 1]);
            break;
        }
        case 5: {
            v52 = 0;
            break;
        }
        default: {
            fprintf(stderr, "bad opcode %d\n", opcode);
            ret = 1;
            goto L7;
        }
        }
    }
    ret = buf[(long)(j - 1)];
    L7:
    return ret;
}

