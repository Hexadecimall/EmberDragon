#include <cstdio>
#include <vector>

using namespace std;

long MatrixGraph(long a, long b);
long connect(struct Struct0* obj3, long a, long b);
long bfs_order(long a, long b);
long dfs_iter_order(long a, long b);
long dfs_rec(long a, long b, long c, long d);
int bfs_dist(long a, long b, long c);
long MatrixGraph_dtor(long a);
long size(struct Struct0* obj2);
int edge(struct Struct0* obj2, long a, long b);

const char* s_main = "main";
const char* s_bfs_dfs_cpp = "bfs_dfs.cpp";

class Record0 {
public:
    long value;
    long data;
    long item;
    // int& std::vector<int>::emplace_back<int const&>(int const&)::'lambda'()::operator_call() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = (*obj->item + 4);
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // int& std::vector<int>::emplace_back<int const&>(int const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long* obj;
        long t131;
        obj = this;
        t131 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = t131;
        return t131;
    }
};

int main(int argc, char** argv) {
    int result;
    long v360;
    long v320;
    long v352;
    int j;
    int v100;
    char v288;
    long v256;
    long v232;
    long v200;
    int i;
    int v84;
    long v160;
    int dist5;
    int dist4;
    int dist3;
    long v120;
    int dist2;
    long v40;
    long v48;
    long v56;
    int dist;
    long __addr0;
    result = 0;
    MatrixGraph(&v360, 7);
    connect((&v360), 0, 1);
    connect(&v360, 0, 2);
    connect(&v360, 1, 3);
    connect(&v360, 2, 4);
    connect(&v360, 2, 5);
    connect(&v360, 3, 6);
    connect(&v360, 4, 6);
    connect(&v360, 5, 6);
    bfs_order(&v360, 0);
    if ((size(&v320)) != 7) {
        v397 = (__assert_rtn(&s_main, &s_bfs_dfs_cpp, 124, "bfs.size() == 7"));
        __builtin_trap();
        v352 = argc;
        goto L1;
    } else {
        j = 0;
        while (j < 7) {
            v100 = (operator_index(&v320, j))->f0;
            if (v100 != (operator_index(&v288, j))->f0) {
                __assert_rtn(&s_main, &s_bfs_dfs_cpp, 125, "bfs[i] == bfs_expected[i]");
                __builtin_trap();
            }
            j++;
        }
        dfs_iter_order(&v360, 0);
        if ((size(&v256)) != 7) {
            v397 = (__assert_rtn(&s_main, &s_bfs_dfs_cpp, 128, "dfs_it.size() == 7"));
            __builtin_trap();
            v352 = argc;
        } else {
            if ((front(&v256))->f0 != 0) {
                __assert_rtn(&s_main, &s_bfs_dfs_cpp, 129, "dfs_it.front() == 0");
                __builtin_trap();
            }
            vector(&v232, 7, &__addr0);
            vector(&v200);
            dfs_rec(&v360, 0, &v232, (&v200));
            if ((size(&v200)) != 7) {
                v397 = (__assert_rtn(&s_main, &s_bfs_dfs_cpp, 135, "dfs_r.size() == 7"));
                __builtin_trap();
                v352 = argc;
            } else {
                i = 0;
                while (i < 7) {
                    v84 = (operator_index(&v200, i))->f0;
                    if (v84 != (operator_index(&v160, i))->f0) {
                        __assert_rtn(&s_main, &s_bfs_dfs_cpp, 136, "dfs_r[i] == dfs_expected[i]");
                        __builtin_trap();
                    }
                    i++;
                }
                dist5 = (bfs_dist(&v360, 0, 6));
                if (dist5 != 3) {
                    __assert_rtn(&s_main, &s_bfs_dfs_cpp, 138, "bfs_dist(g, 0, 6) == 3");
                    __builtin_trap();
                }
                dist4 = (bfs_dist(&v360, 0, 0));
                if (dist4 != 0) {
                    __assert_rtn(&s_main, &s_bfs_dfs_cpp, 139, "bfs_dist(g, 0, 0) == 0");
                    __builtin_trap();
                }
                dist3 = (bfs_dist(&v360, 1, 5));
                if (dist3 != 3) {
                    __assert_rtn(&s_main, &s_bfs_dfs_cpp, 140, "bfs_dist(g, 1, 5) == 3");
                    __builtin_trap();
                }
                MatrixGraph(&v120, 3);
                connect(&v120, 0, 1);
                dist2 = (bfs_dist(&v120, 0, 2));
                if (dist2 != 1) {
                    v397 = (__assert_rtn(&s_main, &s_bfs_dfs_cpp, 145, "bfs_dist(d, 0, 2) == -1"));
                    __builtin_trap();
                    v352 = argc;
                } else {
                    v40 = (size(&v320));
                    v48 = (size(&v256));
                    v56 = (size(&v200));
                    dist = (bfs_dist(&v360, 0, 6));
                    printf("bfs/dfs ok: bfs=%zu dfs_iter=%zu dfs_rec=%zu d(0,6)=%d\n", v40, v48, v56, dist);
                    return 0;
                }
            }
        }
    }
    L1:
    _Unwind_Resume(v352);
}

// (anonymous namespace)::MatrixGraph::MatrixGraph(int)
long MatrixGraph(long a, long b) {
    long result;
    result = a;
    MatrixGraph(a, b);
    return result;
}

struct Struct0 {
    int value;
};
// (anonymous namespace)::MatrixGraph::connect(int, int)
long connect(struct Struct0* obj3, long a, long b) {
    struct Struct0* obj2;
    struct Struct0* obj;
    long* t58;
    obj2 = obj3;
    obj = obj2;
    (operator_index((obj2 + 8), ((a * obj2->value) + b)))->f0 = 1;
    t58 = operator_index((obj + 8), ((b * obj->value) + a));
    t58->f0 = 1;
    return t58;
}

// (anonymous namespace)::bfs_order((anonymous namespace)::MatrixGraph const&, int)
long bfs_order(long a, long b) {
    int n;
    long v185;
    long v136;
    long v64;
    int v28;
    struct Struct0* obj;
    int v60;
    int i;
    int v12;
    long __addr0;
    n = (size(a));
    vector(v185);
    vector(&v136, n, &__addr0);
    queue(&v64);
    (operator_index(&v136, b))->f0 = 1;
    push((&v64), (&b));
    L1:
    v28 = (empty(&v64));
    if (!((v28) != 0)) {
        obj = (front(&v64));
        v60 = obj->value;
        pop(&v64);
        push_back(v185, &v60);
        i = 0;
        while (i < n) {
            v12 = (edge(a, v60, i));
            if (v12) {
                if ((operator_index(&v136, i))->f0 == 0) {
                    (operator_index(&v136, i))->f0 = 1;
                    push(&v64, (&i));
                }
            }
            i++;
        }
        goto L1;
    }
}

// (anonymous namespace)::dfs_iter_order((anonymous namespace)::MatrixGraph const&, int)
long dfs_iter_order(long a, long b) {
    int v152;
    long v169;
    long v120;
    long v48;
    int v20;
    struct Struct0* obj;
    int v44;
    int v40;
    long __addr0;
    v152 = (size(a));
    vector(v169);
    vector(&v120, v152, &__addr0);
    stack(&v48);
    push((&v48), &b);
    L1:
    v20 = (empty(&v48));
    if (!((v20) != 0)) {
        obj = (top(&v48));
        v44 = obj->value;
        pop(&v48);
        if ((operator_index(&v120, v44))->f0 != 0) {
            goto L1;
        }
        (operator_index(&v120, v44))->f0 = 1;
        push_back(v169, (&v44));
        v40 = (v152 - 1);
        while (!((v40 & (1<<31)) != 0)) {
            if (edge(a, v44, v40)) {
                if ((operator_index(&v120, v40))->f0 == 0) {
                    push(&v48, &v40);
                }
            }
            v40--;
        }
        goto L1;
    }
}

// (anonymous namespace)::dfs_rec((anonymous namespace)::MatrixGraph const&, int, std::vector<char>&, std::vector<int>&)
long dfs_rec(long a, long b, long c, long d) {
    (operator_index(c, b))->f0 = 1;
    push_back(d, (&b));
    while (0 < (size(a))) {
        if (edge(a, b, 0)) {
            if ((operator_index(c, 0))->f0 == 0) {
                dfs_rec(a, 0, c, d);
            }
        }
        0++;
    }
}

// (anonymous namespace)::bfs_dist((anonymous namespace)::MatrixGraph const&, int, int)
int bfs_dist(long a, long b, long c) {
    int n;
    long v136;
    long v80;
    int v64;
    int result;
    int i;
    int v12;
    n = (size(a));
    vector(&v136, n, &-1);
    queue(&v80);
    (operator_index((&v136), b))->f0 = 0;
    push((&v80), (&b));
    while (!((empty(&v80)) != 0)) {
        v64 = (front(&v80))->f0;
        pop((&v80));
        if (v64 == c) {
            result = (operator_index(&v136, v64))->f0;
            return result;
        }
        i = 0;
        while (i < n) {
            if (edge(a, v64, i)) {
                if ((operator_index(&v136, i))->f0 == 1) {
                    v12 = ((operator_index(&v136, v64))->f0 + 1);
                    (operator_index((&v136), i))->f0 = v12;
                    push(&v80, (&i));
                }
            }
            i++;
        }
    }
    result = (operator_index(&v136, c))->f0;
}

// (anonymous namespace)::MatrixGraph::MatrixGraph_dtor()
long MatrixGraph_dtor(long a) {
    return a;
}

// (anonymous namespace)::MatrixGraph::MatrixGraph(int)
long MatrixGraph(struct Struct0* obj2, long x) {
    struct Struct0* obj;
    long result;
    long __addr0;
    obj = obj2;
    result = obj;
    obj->value = x;
    vector((obj + 8), (x * x), &__addr0);
    return result;
}

// (anonymous namespace)::MatrixGraph::size() const
long size(struct Struct0* obj2) {
    struct Struct0* obj;
    obj = obj2;
    return obj->value;
}

// (anonymous namespace)::MatrixGraph::edge(int, int) const
int edge(struct Struct0* obj2, long a, long b) {
    struct Struct0* obj;
    obj = obj2;
    return (((operator_index((obj + 8), ((a * obj->value) + b)))->f0 == 0) ? 0 : (0 + 1));
}

// (anonymous namespace)::MatrixGraph::MatrixGraph_dtor()
long MatrixGraph_dtor(long a) {
    return a;
}

