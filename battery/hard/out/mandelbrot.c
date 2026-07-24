int main(int argc, char** argv) {
    long arr;
    long arr3;
    long arr2;
        (0xfffffffffffd8000 + ((((((0 << 16) * 0x84210843) >> 36) + ((((0 << 16) * 0x84210843) >> 36) << 2)) << 15) >> 16)) + 81920;
        L1:
        L2:
        *(arr + (arr3 << 3)) = (*(arr + (arr3 << 3)) + 1);
        if ((arr2 + 1) == 48) continue;
    }
    v75 = 0;
    v76 = 0;
    v77 = (0xfffffffffffd8000 + ((((((arr2 << 16) * 0xae4c415d) >> 37) << 18) - ((((arr2 << 16) * 0xae4c415d) >> 37) << 15)) >> 16));
    loc_100000440:
    v78 = ((v75 * v75) >> 16);
    v79 = ((v76 * v76) >> 16);
    if ((((v76 * v76) >> 16) + ((v75 * v75) >> 16)) >= 262144) goto loc_10000048c;
    v83 = (arr3 + 1);
    v84 = (e + 1);
    v75 = ((v77 + v78) - v79);
    v76 = (d + (((v76 * v75) >> 16) << 1));
    if ((arr3 + 1) != 256) goto loc_100000440;
    v84 = (c + 256);
    v85++;
    goto loc_10000049c;
    loc_10000048c:
    v85 = ((arr3 != 256) ? v85 : (v85 + 1));
    if (arr3 < 2) goto loc_1000003f8;
    loc_10000049c:
    v83 = ((arr3 < 4) ? 1 : ((arr3 < 8) ? 2 : ((arr3 < 16) ? 3 : ((arr3 < 32) ? 4 : ((arr3 < 64) ? 5 : ((arr3 < 128) ? argc : ((arr3 < 255) ? argv : (argv + 1))))))));
    goto loc_1000003fc;

