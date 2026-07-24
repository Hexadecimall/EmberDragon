#include <cstring>
#include <vector>

using namespace std;

long run(struct Buffer* obj);

static long g_100000690[8192];   // unresolved data global — real bytes pending (lifter)


static const long data[8] = {
    7168733591794116882, 6278581101166609548, 11169044, 0, 10, 14, 6, 17 
};
static const long data2[8] = {
    0, 10, 14, 6, 17, 0, 16, 0 
};

class VM {
public:
    long value;
    long data;
    char _pad16[8];
    long item;
    long member;
    char _pad40[16];
    long value56;
    long VM::VM_dtor() {
        long v0;
        long* obj;
        v0 = obj->f48;
        if (obj->f48 != 0) {
            this->value56 = obj;
            operator_delete();
        }
        v0 = this->item;
        if (this->item != 0) {
            this->member = obj;
            operator_delete();
        }
        v0 = this->value;
        if (this->value != 0) {
            this->data = obj;
            operator_delete();
        }
        return this;
    }
    // long long& std::vector<long long>::emplace_back<long long const&>(long long const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long v6;
        long* obj;
        long v1;
        long v2;
        long* v3;
        long v4;
        long t17;
        long v5;
        v6 = (((*(obj->f16 + 8) - *obj->f16) >> 3) + 1);
        v1 = *obj->f16;
        v2 = (*(obj->f16 + 8) - *obj->f16);
        v3 = obj->f16;
        v4 = ((*(obj->f16 + 8) - *obj->f16) >> 3);
        if (!(((((*(obj->f16 + 8) - *obj->f16) >> 3) + 1) >> 61) != 0)) {
            v6 = (((v3->f16 - v1) < 0x7ffffffffffffff8) ? ((((v3->f16 - v1) >> 2) >= v6) ? ((v3->f16 - v1) >> 2) : v6) : 0x1fffffffffffffff);
            if (((((v3->f16 - v1) < 0x7ffffffffffffff8) ? ((((v3->f16 - v1) >> 2) >= v6) ? ((v3->f16 - v1) >> 2) : v6) : 0x1fffffffffffffff) >> 61) != 0) goto loc_100000b44;
            t17 = operator_new(v6 << 3);
            *((t17 + v2) + 8) = *obj->data;
            memcpy(((t17 + v2) - (v4 << 3)), v1, v2);
            v3->value = ((t17 + v2) - (v4 << 3));
            v3->data = (t17 + v2);
            v3->f16 = (t17 + (v6 << 3));
            v5 = (t17 + v2);
            if (v1 != 0) {
                operator_delete(v1);
            }
            *this->value = v5;
            return obj;
        }
        __throw_length_error();
        loc_100000b44:
        __throw_bad_array_new_length();
    }
    // Frame& std::vector<Frame>::emplace_back<Frame const&>(Frame const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long v6;
        long v7;
        long* obj;
        long v0;
        long v2;
        long v3;
        long* v5;
        long t22;
        long v4;
        v6 = 0x666666666666666;
        v7 = (1 + ((*(obj->f16 + 8) - *obj->f16) >> 3) * 0xcccccccccccccccd);
        v0 = 0xcccccccccccccccd;
        v2 = *obj->f16;
        v3 = (*(obj->f16 + 8) - *obj->f16);
        v5 = obj->f16;
        if (!((1 + ((*(obj->f16 + 8) - *obj->f16) >> 3) * 0xcccccccccccccccd) >= 0x666666666666666)) {
            v7 = (((((v5->f16 - v2) >> 3) * v0) < 0x333333333333333) ? ((((((v5->f16 - v2) >> 3) * v0) << 1) >= v7) ? ((((v5->f16 - v2) >> 3) * v0) << 1) : v7) : v6);
            if ((((((v5->f16 - v2) >> 3) * v0) < 0x333333333333333) ? ((((((v5->f16 - v2) >> 3) * v0) << 1) >= v7) ? ((((v5->f16 - v2) >> 3) * v0) << 1) : v7) : v6) >= v6) goto loc_100000c40;
            t22 = operator_new((v7 + (v7 << 2)) << 3);
            *(t22 + v3) = *obj->data;
            *((t22 + v3) + 8) = *(obj->data + 8);
            *((t22 + v3) + 32) = *(obj->data + 32);
            memcpy(*(obj->data + 8), v2, v3);
            v5->value = t22;
            v5->data = ((t22 + v3) + 40);
            v5->f16 = (t22 + ((v7 + (v7 << 2)) << 3));
            v4 = ((t22 + v3) + 40);
            if (v2 != 0) {
                operator_delete(v2);
            }
            *this->value = v4;
            return obj;
        }
        __throw_length_error();
        loc_100000c40:
        __throw_bad_array_new_length();
    }
};

struct Struct0 {
    long value;
    long data;
    char _pad16[16];
    long item;
};
int main(int argc, char** argv) {
    long t0;
    struct Struct0* t2;
    long obj;
    long v74;
    long v0;
    long v24;
    long t7;
    t0 = operator_new(352);
    memcpy(data2, 352);
    t0 + 352;
    t2 = operator_new(40);
    t2->item = 0;
    t2->value = t2;
    t2->data = t2;
    obj = t2;
    v74 = t0;
    if ((run(&v0)) == 0x375f00) {
        if (obj != 0) {
            operator_delete();
        }
        if (v24 != 0) {
            operator_delete();
        }
        if (v0 != 0) {
            operator_delete();
        }
        return 0;
    }
    t7 = __assert_rtn("main", "bytecode_vm.cpp", 113, "result == 3628800");
    __builtin_trap();
    operator_delete(v74, t7);
    _Unwind_Resume(_Unwind_Resume(t7));
}

struct Buffer {
    char* data;
    char _pad8[16];
    long value;
    long data2;
    long size;
    char _pad48[8];
    long item;
    long member;
    long count;
};
struct Pair {
    char _pad0[8];
    char value;
    char _pad9[15];
    char data;
};
struct Struct1 {
    long value;
    long data;
    long item;
    char _pad16[16];
    long member;
};
struct Pair2 {
    long value;
    long data;
};
long VM::run() {
    long v74;
    struct Pair2* v77;
    long* v75;
    long indirect;
    struct Struct1* v76;
    long v48;
    long v8;
    obj + 24;
    v74 = data;
    obj->count++;
    v77 = *(obj->data + (obj->count << 4));
    v75 = (obj->data + (obj->count << 4));
    while (!(*(obj->data + (obj->count << 4)) >= 18)) {
        /* br (g_100000690 + (*(v74 + v77) << 2)) (indirect) */
        v76 = v75->f8;
        v48 = obj->data2;
        v77 = obj->data2;
        if (obj->data2 < obj->size) {
            v77->data = v76;
            obj->data2 = v77;
            continue;
        }
        operator_call(&v8);
        obj->data2 = v48;
    }
}

