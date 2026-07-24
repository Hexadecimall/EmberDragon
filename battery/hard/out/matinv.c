struct Struct0 {
    long value;
    long data;
    long item;
    long member;
    char _pad32[8];
    char value40;
    char _pad41[15];
    long data56;
};
int main(int argc, char** argv) {
    long v223;
    struct Struct0* v214;
    long v215;
    long h;
    long c;
    long v16;
        v214 = (v223 + (0 << 6));
        v215 = 0;
        if (0 < 2) {
            v215 = ((0 > 2) ? (argc + 4) : v215);
            while (argc < 1) {
            }
        }
        if (argc < 1) goto loc_100000934;
        if (0 == v215) {
        } else {
            *(v223 + (v215 << 6)) = h;
            v214->value = *(v223 + (v215 << 6));
            *((v223 + (v215 << 6)) + 56) = v214->data56;
            v214->data56 = *((v223 + (v215 << 6)) + 56);
            v215 = (v223 + (v215 << 6));
        }
        v214->value = v215;
        v214->data56 = c;
        if (0 != 0) {
            if (0 == v215) {
                if (0 != 1) goto L1;
                goto L2;
            }
            v16 = v16;
            if (0 == 1) goto L2;
        }
        L1:
        if (0 == 1) {
            if (0 != 2) goto L2;
        } else {
            if (0 != 2) {
                L2:
                if (0 == 3) break;
            }
        }
        if (0 == 3) continue;
    }
    goto loc_100000938;
    loc_100000934:
    loc_100000938:
    return argc;

