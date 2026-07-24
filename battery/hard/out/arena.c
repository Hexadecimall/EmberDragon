long Arena(long a, long b);
long ArenaScope(long a, long b);
long fill_pattern(long data, long n2, long a);
long ArenaScope_dtor(long a);
long Arena_dtor(long a);
long Buffer(long a, long b);
long Buffer_dtor(long a);

class Arena {
public:
    long value;
    char* data;
    char _pad2[6];
    long data2;
    long item;
    long member;
    long Arena::allocate(unsigned long a, unsigned long b) {
        long v40;
        long v32;
        long* obj2;
        long v24;
        long v16;
        long v56;
        v40 = a;
        v32 = b;
        obj2 = this;
        v24 = ((this->item + (v32 - 1)) & (v32 - 1));
        v16 = (v24 + v40);
        if (v16 >= (size(this))) {
            v56 = 0;
        } else {
            obj2->item = (v24 + v40);
            obj2->member++;
            v56 = ((data(obj2)) + v24);
        }
    }
    long Arena::operator_assign(Arena&& obj2) {
        long v24;
        long a;
        v24 = a;
        this = obj2;
        v24 = this;
        return *(this->data + 1);
    }
    long Arena::used() const {
        return this->item;
    }
    long Arena::allocations() const {
        return this->member;
    }
    ArenaScope::ArenaScope(Arena& a) {
        long v16;
        long* obj2;
        v16 = a;
        obj2 = this;
        this->value = v16;
        obj2->data2 = (used(v16));
        obj2->item = 0;
        return obj2;
    }
    long Buffer::size() const {
        return this->data2;
    }
    long Buffer::data() {
        return this->value;
    }
    Arena::Arena(Arena&& obj2) {
        long v24;
        long a;
        v24 = a;
        this = obj2;
        Buffer(v24, this);
        return *(this->data + 1);
    }
    Buffer::Buffer(Buffer&& obj4) {
        long* obj2;
        long obj3;
        obj2 = obj3;
        this = obj4;
        obj2->value = this->value;
        obj2->data2 = this->data2;
        this->value = 0;
        this->data2 = 0;
        return obj2;
    }
    long Buffer::operator_assign(Buffer&& obj3) {
        long v24;
        long a;
        long* obj2;
        v24 = a;
        this = obj3;
        obj2 = v24;
        if (v24 != this) {
            operator_delete(obj2->value);
            obj2->value = this->value;
            obj2->data2 = this->data2;
            this->value = 0;
            this->data2 = 0;
        }
    }
    long Buffer::Buffer_dtor() {
        long result;
        result = this;
        operator_delete(this->value);
        return result;
    }
};

class ArenaScope {
public:
    char _pad0[16];
    char flag;
    long ArenaScope::commit() {
        long obj2;
        this->flag = 1;
        return obj2;
    }
    long ArenaScope::ArenaScope_dtor() {
        long* obj;
        long v8;
        obj = this;
        if (!((this->flag) != 0)) {
            v8 = (used(obj->f0));
            v8 - obj->f8;
        }
    }
};

int main(int argc, char** argv) {
    int result;
    long v232;
    long sum;
    long v200;
    long v80;
    long v176;
    long v72;
    long v168;
    long pattern3;
    long pattern2;
    long v136;
    long v48;
    long v128;
    long pattern;
    long v24;
    long v0;
    long v8;
    result = 0;
    Arena(&v232, 256);
    sum = 0;
    ArenaScope(&v200, (&v232));
    v80 = (allocate(&v232, 40, 16));
    v176 = v80;
    v72 = (allocate(&v232, 24, 8));
    v168 = v72;
    if (v176 != 0) {
        pattern3 = (fill_pattern(v176, 40, 3));
        sum ^= pattern3;
    } else {
        if (v168 != 0) {
            pattern2 = (fill_pattern(v168, 24, 9));
            sum ^= pattern2;
        }
        commit(&v200);
        Arena(&v136, &v232);
        v48 = (allocate((&v136), 64, 32));
        v128 = v48;
        if (v128 != 0) {
            pattern = (fill_pattern(v128, 64, 17));
            sum += pattern;
        } else {
            Arena(&v136, 64);
            v24 = (used((&v136)));
            v0 = (v24 * 0xf4243);
            v8 = (allocations(&v136));
            sum ^= (v0 + v8);
            return (sum & 127);
        }
    }
}

Arena::Arena(unsigned long b) {
    long result;
    result = a;
    Arena(a, b);
    return result;
}

ArenaScope::ArenaScope(Arena& b) {
    long result;
    result = a;
    ArenaScope(a, b);
    return result;
}

long fill_pattern(unsigned char* data, unsigned long n2, unsigned char a) {
    long buf;
    long n;
    long v16;
    long i;
    buf = data;
    n = n2;
    v16 = 0;
    i = 0;
    while (i < n) {
        *(buf + i) = (a + (i * 31));
        v16 = ((v16 * 131) + *(buf + i));
        i++;
    }
}

long ArenaScope::ArenaScope_dtor() {
    return a;
}

Arena::Arena(Arena&& b) {
    long result;
    result = a;
    Arena(a, b);
    return result;
}

long Arena::Arena_dtor() {
    return a;
}

struct Pair {
    char _pad0[16];
    long value;
    long data;
};
Arena::Arena(unsigned long b) {
    struct Pair* obj;
    obj = a;
    Buffer(a, b);
    obj->value = 0;
    obj->data = 0;
    return obj;
}

Buffer::Buffer(unsigned long b) {
    long result;
    result = a;
    Buffer(a, b);
    return result;
}

struct Pair2 {
    char* data;
    long value;
};
Buffer::Buffer(unsigned long n2) {
    long n;
    struct Pair2* obj;
    long i;
    n = n2;
    obj = a;
    obj->data = (operator_new(n));
    obj->value = n;
    i = 0;
    while (i < n) {
        *(obj->data + i) = 0;
        i++;
    }
}

Buffer::Buffer(Buffer&& b) {
    long result;
    result = a;
    Buffer(a, b);
    return result;
}

long Arena::Arena_dtor() {
    return a;
}

long Buffer::Buffer_dtor() {
    return a;
}

