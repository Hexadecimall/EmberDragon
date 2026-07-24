#include <cstring>

long taskSum(struct Counter* obj2, long a);
long taskProducer(struct Struct1* obj3, struct Struct0* obj4);
long taskConsumer(struct Struct1* obj3, struct Struct0* obj4);
long Scheduler(long a);
int run(long a, long b);

static long g_100000498[8192];   // unresolved data global — real bytes pending (lifter)
static long g_100000574[8192];   // unresolved data global — real bytes pending (lifter)
static long g_100000610[8192];   // unresolved data global — real bytes pending (lifter)

const char* s_main = "main";
const char* s_scheduler_cpp = "scheduler.cpp";

class Scheduler {
public:
    char _pad0[320];
    int value;
    int Scheduler::spawn(void (*)(TaskState&, Scheduler&) a) {
        long v64;
        long* obj;
        int result;
        long v8;
        v64 = a;
        obj = this;
        if (this->value >= 8) {
            __assert_rtn("spawn", &s_scheduler_cpp, 32, "count < MAX");
        }
        obj->value++;
        result = obj->value;
        v8 = v64;
        (operator_index(obj, result))->f0 = v8;
        operator_index((obj + 64), result);
        return result;
    }
};

struct Counter {
    int value;
    char _pad4[4];
    long total;
    long count;
    char data;
    char _pad25[3];
    int count2;
};
long taskSum(TaskState& obj2, Scheduler& a) {
    struct Counter* obj;
    int v12;
    obj = obj2;
    obj->count2++;
    v12 = obj->value;
    if (obj->value != 0) {
        if (!(v12 == 1)) {
        if (!(v12 == 2)) {
    } else {
        obj->count = 1;
        obj->total = 0;
        obj->value = 1;
        }
        obj->total += obj->count;
        if (obj->count >= 5) {
            obj->value = 2;
        } else {
            obj->count++;
        }
            obj->data = 2;
        }
    }
}

struct Struct0 {
    char _pad0[328];
    long value;
};
struct Struct1 {
    int value;
    char _pad4[4];
    long data;
    char _pad16[8];
    char item;
    char _pad25[3];
    int count;
};
long taskProducer(TaskState& obj3, Scheduler& obj4) {
    struct Struct1* obj2;
    struct Struct0* obj;
    int v12;
    obj2 = obj3;
    obj = obj4;
    obj2->count++;
    v12 = obj2->value;
    if (obj2->value != 0) {
        if (!(v12 == 1)) {
        if (v12 == 2) goto L1;
    } else {
        obj2->value = 1;
        }
        obj2->value = 2;
        L1:
        obj->value = 42;
        obj2->data = 42;
        obj2->item = 2;
    }
}

long taskConsumer(TaskState& obj3, Scheduler& obj4) {
    struct Struct1* obj2;
    struct Struct0* obj;
    obj2 = obj3;
    obj = obj4;
    obj2->count++;
    if (obj2->value == 0) {
        if (obj->value == 0) {
            obj2->item = 1;
        } else {
            obj2->data = obj->value;
            obj2->value = 1;
            obj2->item = 2;
        }
    }
}

int main(int argc, char** argv) {
    long v56;
    int v52;
    int v48;
    int v44;
    int v40;
    long v120;
    int i;
    int n;
    long v16;
    Scheduler(&v56);
    v52 = (spawn((&v56), g_100000498));
    v48 = (spawn((&v56), g_100000574));
    v44 = (spawn((&v56), g_100000610));
    v40 = (run((&v56)));
    if ((operator_index(((&v56) + 64), v52))->f8 != 15) {
        __assert_rtn(&s_main, &s_scheduler_cpp, 105, "sch.states[a].acc == 15");
    }
    if ((operator_index(&v120, v48))->f8 != 42) {
        __assert_rtn(&s_main, &s_scheduler_cpp, 107, "sch.states[b].acc == 42");
    }
    if ((operator_index(&v120, v44))->f8 != 42) {
        __assert_rtn(&s_main, &s_scheduler_cpp, 109, "sch.states[c].acc == 42");
    }
    i = 0;
    while (i < n) {
        if ((operator_index(&v120, i))->f24 != 2) {
            __assert_rtn(&s_main, &s_scheduler_cpp, 112, "sch.states[i].status == Status::Done");
        }
        i++;
    }
    if ((operator_index(&v120, v44))->f28 < 2) {
        __assert_rtn(&s_main, &s_scheduler_cpp, 114, "sch.states[c].ticks >= 2");
    }
    if (v40 <= 0) {
        __assert_rtn(&s_main, &s_scheduler_cpp, 115, "total > 0");
    }
    v16 = (operator_index(&v120, v52))->f8;
    return (((v16 + (operator_index(((&v56) + 64), v44))->f8) == 57) ? 0 : (0 + 1));
}

Scheduler::Scheduler() {
    long result;
    result = a;
    Scheduler(a);
    return result;
}

struct Pair {
    char _pad0[320];
    int size;
    char _pad324[4];
    long value;
};
int Scheduler::run() {
    struct Pair* obj;
    char v35;
    int i;
    long ptr;
    obj = a;
    L1:
    v35 = 0;
    i = 0;
    while (i < obj->size) {
        if ((operator_index((obj + 64), i))->f24 == 2) {
        } else {
            v35 = 1;
            if ((operator_index((obj + 64), i))->f24 == 0) {
                ptr = (operator_index(obj, i))->f0;
                operator_index((obj + 64), i);
                (*ptr)();
                0++;
            } else {
                if ((operator_index((obj + 64), i))->f24 == 1) {
                    if (obj->value != 0) {
                        (operator_index((obj + 64), i))->f24 = 0;
                    }
                }
            }
        }
        i++;
    }
    if (!((v35) != 0)) {
    } else {
        goto L1;
    }
}

Scheduler::Scheduler() {
    struct Pair* v0;
    long v16;
    struct Counter* obj;
    v0 = a;
    memset(a, 0, 64);
    v16 = (v0 + 64);
    memset((v0 + 64), 0, 256);
    obj = v16;
    obj->value = 0;
    obj->total = 0;
    obj->count = 0;
    obj->data = 0;
    obj->count2 = 0;
    obj += 32;
    while ((obj + 32) != (v0 + 320)) {
    }
}

