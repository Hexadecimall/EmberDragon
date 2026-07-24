static long g_1000000d6[8192];   // unresolved data global — real bytes pending (lifter)


class RobinMap {
public:
    long value;
    char _pad8[16];
    long count;
    long data;
    long RobinMap::insert(unsigned long long a, int b) {
        long v23;
        long* obj;
        long v21;
        long v22;
        long v24;
        long* v12;
        long v16;
        long* v17;
        long v13;
        long v14;
        long v15;
        long v18;
        long v19;
        int v8;
        int v11;
        int v0;
        int v3;
        v23 = obj->value;
        v21 = b;
        v22 = a;
        if (!((10 + obj->count * 10) < (((obj->f8 - obj->value) >> 2) + ((obj->f8 - obj->value) >> 3)))) {
            grow(this);
            v23 = this->value;
        }
        v24 = (this->data & ((((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ (((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb)));
        v12 = (v23 + (this->data & ((((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ (((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb))) * 24);
        v16 = this->data;
        v17 = (v23 + (this->data & ((((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ (((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb))) * 24);
        if (!(*((v23 + (this->data & ((((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ (((((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ (((v22 + 0x9e3779b97f4a7c15) ^ (v22 + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb))) * 24) + 16) != 1)) {
            v13 = 0;
            v14 = 1;
            v15 = 24;
                v18 = v12->value;
                if (v12->value == v22) goto loc_100000974;
                v19 = v12->f12;
                if (v12->f12 >= v13) continue;
                v8 = v12->f17;
                v11 = v12->f20;
                v12->value = v22;
                v12->f8 = v21;
                v12->f12 = v13;
                v17->value = v14;
                v12->f17 = v0;
                v12->f20 = v3;
                v0 = v8;
                v3 = v11;
                v23 = this->value;
                v13 = v19;
                v16 = this->data;
                v21 = v12->f8;
                v22 = v18;
            }
        }
        v13 = 0;
        v12->value = v22;
        v12->f8 = v21;
        v12->f12 = v13;
        v12->f16 = 1;
        v12->f17 = v0;
        v12->f20 = v3;
        this->count++;
        goto loc_100000978;
        loc_100000974:
        v12->f8 = v21;
        loc_100000978:
        return obj;
    }
    long RobinMap::grow() {
        long* obj;
        long v8;
        long v34;
        long v35;
        long v30;
        long v32;
        long* v25;
        long v26;
        long v27;
        long* v28;
        long v29;
        long* v33;
        obj->f8 = 0;
        obj->f16 = 0;
        obj->value = 0;
        assign((((obj->f8 - obj->value) >> 3) * 0x5555555555555556), &v8);
        v34 = obj->value;
        v35 = obj->f8;
        v30 = obj->value;
        v32 = obj->f8;
        if (obj->value != obj->f8) {
            v25 = v34;
            v26 = ((v35 - v34) - 24);
            if (!(((v35 - v34) - 24) < 72)) {
                v25 = (v34 + ((((v26 * 0xaaaaaaaaaaaaaaab) >> 4) + 1) & 0x1ffffffffffffffc) * 24);
                v26 = (((v26 * 0xaaaaaaaaaaaaaaab) >> 4) + 1);
                v27 = ((((v26 * 0xaaaaaaaaaaaaaaab) >> 4) + 1) & 0x1ffffffffffffffc);
                v28 = (v34 + 60);
                v29 = ((((v26 * 0xaaaaaaaaaaaaaaab) >> 4) + 1) & 0x1ffffffffffffffc);
                v28->fm44 = 0;
                v28->fm20 = 0;
                v28->f4 = 0;
                v28->f28 = 0;
                v28->fm48 = 0;
                v28->fm24 = 0;
                v28->value = 0;
                v28->count = 0;
                v28 += 96;
                v29 -= 4;
                while (v29 != 4) {
                }
                if (v26 == v27) goto loc_100000aec;
            }
            v25->f16 = 0;
            v25->f12 = 0;
            v25 += 24;
            while ((v25 + 24) != v35) {
            }
        }
        loc_100000aec:
        this->count = 0;
        this->data = (-1 + ((v35 - v34) >> 3) * 0xaaaaaaaaaaaaaaab);
        if (v30 != v32) {
            v33 = v30;
                if (v33->f16 != 1) continue;
                insert(this, v33->value, v33->f8);
            }
        }
        if (v30 != 0) {
            operator_delete(v30);
        }
        return obj;
};

struct Pair {
    long value;
    char _pad8[4];
    int data;
};
struct Struct0 {
    long value;
    int data;
    int item;
};
struct Struct1 {
    long value;
    long data;
    long item;
    long member;
    long value32;
    long data40;
    long item48;
    long member56;
    long value64;
    long data72;
    long item80;
    long member88;
};
int main(int argc, char** argv) {
    struct Struct1* t0;
    long obj;
    long v83;
    long v85;
    long v89;
    long v77;
    long v72;
    long v78;
    long v79;
    long v80;
    long v81;
    long v82;
    struct Struct0* obj3;
    struct Pair* obj2;
    t0 = operator_new(192);
    t0->item = t0;
    t0->member = t0;
    t0->value64 = t0;
    t0->data72 = t0;
    t0->value = t0;
    t0->data = t0;
    t0->value32 = t0;
    t0->data40 = t0;
    t0->item48 = t0;
    t0->member56 = t0;
    t0->item80 = t0;
    t0->member88 = t0;
    obj = t0;
    *(*g_1000000d6 + 12) = 0;
    *(*g_1000000d6 + 36) = 0;
    *(*g_1000000d6 + 60) = 0;
    *(*g_1000000d6 + 84) = 0;
    *(*g_1000000d6 + 108) = 0;
    *(*g_1000000d6 + 132) = 0;
    *(*g_1000000d6 + 156) = 0;
    *(*g_1000000d6 + 180) = 0;
    v83 = 0;
    v85 = 0xc122b80c908;
    insert(&obj, v83, (v83 >> 13));
    v83 += 0x9e3779b1;
    while ((v83 + 0x9e3779b1) != v85) {
    }
    v83 = 0;
    v85 = 0;
    insert(&obj, v83, v85);
    v83 += 0x9e3779b1;
    v85--;
    while ((v85 - 1) != 100) {
    }
    v89 = 0;
    v77 = v72;
    v78 = obj;
    v79 = 0;
    v80 = 24;
    v81 = 0xf4243;
    v82 = 5000;
    v83 = 0;
    v85 = 0;
    goto L2;
    L1:
    v83++;
    v79++;
    while (!((v79 + 1) == v82)) {
        L2:
        if (*((v78 + (v77 & (((((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ ((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ ((((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ ((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb))) * v80) + 16) != 1) goto L1;
        while (obj3->value != argc) {
            if (obj3->item < (obj2 + 1)) goto L1;
            if ((*((v78 + ((argv + 1) & v77) * v80) + 16)) != 0) continue;
        }
        v89 = (obj3->data + v89 * v81);
        v85++;
    }
    v79 = 5000;
    v80 = 24;
    v81 = 5500;
    goto L4;
        L3:
        v79++;
        while (!((v79 + 1) == v81)) {
            L4:
            v82 = (v79 * 0x9e3779b1);
            if (*((v78 + (v77 & (((((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ ((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb) ^ ((((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9) ^ ((((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15) ^ ((v79 * 0x9e3779b1) + 0x9e3779b97f4a7c15)) * 0xbf58476d1ce4e5b9)) * 0x94d049bb133111eb))) * v80) + 16) != 1) continue;
        L5:
    } while (obj2->value == v82);
    v74 = (argv + 1);
    if (obj2->data < (argv + 1)) goto loc_100000700;
    v73 = ((argc + 1) & v77);
    v84 = (v78 + ((argc + 1) & v77) * v80);
    if ((*((v78 + ((argc + 1) & v77) * v80) + 16)) != 0) goto loc_100000740;
    goto loc_100000700;

