long Rectangle_dtor(long a);
long area(struct Struct3* obj);
long perimeter(struct Struct3* obj);
long sides();
long bounding_radius(struct Struct3* obj);

static long g_1000000bb[8192];   // unresolved data global — real bytes pending (lifter)
static long g_1000000bd[8192];   // unresolved data global — real bytes pending (lifter)

const char* s_str = "@\n";

struct Struct0 {
    char* data;
};
struct Struct1 {
    long value;
    long data;
    long item;
};
struct Pair {
    char value;
    char _pad1[7];
    long data;
};
struct Struct2 {
    char* data;
    char* data2;
    char* data3;
    long value;
};
struct Pair2 {
    long value;
    long data;
};
int main(int argc, char** argv) {
    long t0;
    long* t1;
    long v104;
    struct Struct1* t2;
    struct Pair* t3;
    long v112;
    long* t5;
    struct Struct2* t6;
    long* t8;
    long v131;
    long v123;
    long v124;
    long v126;
    long v127;
    long v128;
    long* v129;
    struct Pair2* v130;
    long v125;
    long v122;
    t0 = operator_new(32);
    **(*g_1000000bb + 8) = &s_str;
    *(*(*g_1000000bb + 8) + 24) = 0x4008000000000000;
    t1 = operator_new(8);
    t1->f8 = t0;
    v104 = t1;
    t2 = operator_new(40);
    t2->item = 0;
    t2->value = "@\n";
    t2->data = 0;
    t3 = operator_new(16);
    t3->value = t0;
    t3->data = t2;
    v104 = t3;
    operator_delete(t1);
    v112 = (t3 + 16);
    t5 = operator_new(48);
    *t5->f8 = "@\n";
    *(*(*g_1000000bd + 24) + 40) = 0x4014000000000000;
    t6 = operator_new(32);
    t6->data3 = t5;
    v104 = *t3->value;
    operator_delete(t3);
    v112 = (t6 + 24);
    t8 = operator_new(32);
    *t8->f8 = &s_str;
    *(t8->f8 + 24) = 0x3ff8000000000000;
    t6->value = t8->f8;
    v112 = (t6 + 32);
    (**(*t6->data + 16))();
    (**(*t6->data + 24))();
    (**(*t6->data + 40))();
    (**(**(t6->data + 8) + 16))();
    (**(*t6->data2 + 24))();
    (**(*t6->data2 + 40))();
    (**(**(t6->data2 + 8) + 16))();
    (**(*t6->data3 + 24))();
    (**(*t6->data3 + 40))();
    (**(*t6->value + 16))();
    (**(*t6->value + 24))();
    (**(*t6->value + 40))();
    v131 = t6->value;
    v123 = *(t6->data3 + 8);
    v124 = ((**(*t6->data + 32))());
    v126 = ((**(*t6->data2 + 32))());
    v127 = ((**(*t6->data3 + 32))());
    v128 = ((**(*t6->value + 32))());
    v129 = (t3 + 16);
    if (!((t3 + 16) == v130)) {
        v125 = 0;
        v131 = *(*v129->f8 + 32);
        v125 = (((**(*v129->f8 + 32))()) + v125);
        while (v129 != v130) {
        }
        v129 = v104;
        if (v104 != 0) goto L1;
    } else {
        v125 = 0;
        if (v129 != 0) {
            L1:
            v130 = v112;
            if (v129 == v112) {
                goto L4;
                L2:
                if (v130 == v129) goto L3;
            }
            v130->data = 0;
            if (v130->value == 0) goto L2;
            (**(argc->data + 8))();
            v131 = *(argc->data + 8);
            goto L2;
            L3:
            L4:
            v112 = v129;
            operator_delete();
        }
    }
    return (v125 ^ (((((v128 + (v127 + (v126 + v124))) + v122) + ((v127 + (v126 + v124)) + ((v126 + v124) + v131))) ^ ((((((v122 ^ 0x739d0383) * 435) ^ 435) * 51) ^ v123) * 51)) & 127));
}

long Rectangle::Rectangle_dtor() {
    return a;
}

long Rectangle::Rectangle_dtor() {
    long tail;
    long call;
    long unrecovered;
    long target;
    ; // -> loc_100000b90 (tail-call / unrecovered target)
}

struct Struct3 {
    char _pad0[24];
    long value;
};
long Circle::area() const {
    return obj->value;
}

long Circle::perimeter() const {
    return obj->value;
}

long Circle::sides() const {
    return 0;
}

long Circle::bounding_radius() const {
    return obj->value;
}

long Shape::bounding_radius() const {
    return ((**(obj->data + 16))());
}

struct Struct4 {
    char _pad0[12];
    int value;
};
long Rectangle::area() const {
    return obj->value;
}

long Rectangle::perimeter() const {
    return obj->value;
}

long Rectangle::sides() const {
    return 4;
}

long Rectangle::bounding_radius() const {
    return obj->value;
}

struct Struct5 {
    char* data;
    char _pad8[4];
    int value;
    int data2;
    char _pad20[20];
    long item;
};
long Triangle::area() const {
    return ((**(obj->data + 24))());
}

long Triangle::perimeter() const {
    return obj->value;
}

long Triangle::sides() const {
    return 3;
}

