#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int isalpha(int a);
int tolower(int a);
long __istype(int a, long b);
long isascii(int a);
void Entry(long a, long b);
long operator_call(long a, long b);


static const char str[] = "\n";

class Entry {
    long value;
    long data;
    long item;
    int member;
    // Entry& std::vector<Entry>::emplace_back<Entry>(Entry&&)::'lambda'()::operator()() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = *obj->item + 32;
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // Entry& std::vector<Entry>::emplace_back<Entry>(Entry&&)::'lambda0'()::operator()() const
    long operator_call() {
        long* obj;
        long value2;
        obj = this;
        value2 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = value2;
        return value2;
    }
    Entry(Entry&& obj3) {
        long* obj2;
        this = obj3;
        obj2 = a;
        std::string a = this;
        obj2->member = this->member;
        return;
    }
    // main::$_0::operator()(Entry const&, Entry const&) const
    long operator_call(long obj3, long obj4) {
        long* obj2;
        char v31;
        obj2 = obj3;
        this = obj4;
        if (obj2->member != this->member) {
            v31 = (obj2->member <= this->member ? 0 : 1) & 1;
            return (v31 & 1);
        } else {
            v31 = obj2 < this & 1;
            return (v31 & 1);
        }
        return (v31 & 1);
    }
    long operator=(Entry&& obj3) {
        long v24;
        long a;
        long* obj2;
        v24 = a;
        this = obj3;
        obj2 = v24;
        v24 = this;
        obj2->member = this->member;
        return obj2;
    }
};

struct Struct0 {
    char _pad0[24];
    int count;
};
int main(int argc, char** argv) {
    int result;
    char buf[64];
    long v224;
    long i;
    long v64;
    long v56;
    int v52;
    char v215;
    int ok;
    int lower;
    char v195;
    long v176;
    long v168;
    struct Struct0* obj;
    long v120;
    long v112;
    long v88;
    long v80;
    struct Struct0* obj2;
    long v24;
    long v16;
    long count2;
    result = 0;
    std::string ptr = "the cat sat on the mat the cat ran the dog sat on the log and the dog and the cat sat together on the warm mat";
    vector(buf);
    basic_string(&v224);
    i = 0;
    v64 = i;
    while (v64 < ptr.size()) {
        v56 = i;
        if (v56 < ptr.size()) {
            v52 = *ptr[i];
        } else {
            v52 = 32;
        }
        v215 = v52;
        ok = isalpha(v215);
        if (ok != 0) {
            lower = tolower(v215);
            operator_adde(&v224, lower);
            goto L1;
        }
        if (!(empty(&v224) != 0)) {
            v195 = 0;
            v176 = begin(buf);
            v168 = end(buf);
            while (v176 != v168) {
                obj = operator_mul&v176;
                if (obj == v224) {
                    obj->count++;
                    v195 = 1;
                    break;
                } else {
                    operator_inc&v176;
                }
            }
            if (!(v195 != 0)) {
                std::string v128 = &v224;
                push_back(buf, &v128);
            }
            clear(&v224);
        }
        L1:
        i++;
    }
    v120 = begin(buf);
    v112 = end(buf);
    sort(v120, v112);
    v88 = begin(buf);
    v80 = end(buf);
    while (v88 != v80) {
        obj2 = operator_mul&v88;
        v24 = cout << obj2;
        v16 = v24 << ": ";
        count2 = v16 << obj2->count;
        operator_inc&v88;
    }
    return 0;
}

int isalpha(int a) {
    return (__istype(a, 256));
}

int tolower(int a) {
    return (__tolower(a));
    long result;
    result = a;
    return;
}

long __istype(int a, unsigned long b) {
    int v12;
    long v56;
    long t4;
    if (isascii(a) != 0) {
        v12 = (v56 == t4 ? 0 : 1);
        return (v12 & 1);
    } else {
        v12 = (__maskrune(a, b) == 0 ? 0 : 1);
        return (v12 & 1);
    }
    return (v12 & 1);
}

long isascii(int a) {
    long t44;
    return (t44 != 0 ? 0 : 1);
    long result;
    result = a;
    return;
}

Entry::Entry(Entry&& b) {
    long result;
    result = a;
    Entry(a, b);
    return;
}

// std::string::basic_string[abi:nqe220100](std::string&&)::'lambda'(std::string&)::operator()(std::string&) const
long operator_call(long a, long b) {
    long result;
    result = b;
    if (!(__is_long(result) != 0)) {
        __annotate_delete(result);
        return result;
    }
    return result;
}

