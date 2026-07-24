#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;

long MinHeap(long a, long b);
long push(long a, long b, long c);
long decrease(long a, long b, long c);
int empty(long a);
int pop_min(long a);
int size(long a);
long MinHeap_dtor(long a);
long sift_up(long a, long b);
long swap_nodes(long a, long b, long c);
long sift_down(long a, long b);


static const int matrix_init[16] = {
    50, 13, 27, 99, 4, 61, 8, 7, 42, 31, 19, 2, 88, 71, 33, 60 
};

class Record0 {
public:
    long value;
    long data;
    long item;
    // MinHeap::Node& std::vector<MinHeap::Node>::emplace_back<MinHeap::Node>(MinHeap::Node&&)::'lambda'()::operator_call() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = (*obj->item + 8);
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // MinHeap::Node& std::vector<MinHeap::Node>::emplace_back<MinHeap::Node>(MinHeap::Node&&)::'lambda0'()::operator_call() const
    long operator_call() {
        long* obj;
        long t40;
        obj = this;
        t40 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = t40;
        return t40;
    }
};

int main(int argc, char** argv) {
    int ret;
    long v112;
    int matrix[16];
    int i;
    long v80;
    int n;
    int v60;
    int min;
    int v68;
    int v64;
    int v36;
    long v40;
    int v52;
    ret = 0;
    MinHeap(&v112, 16);
    memcpy(matrix, matrix_init, 64);
    i = 0;
    while (i < 16) {
        push(&v112, i, matrix[i]);
        i++;
    }
    decrease(&v112, 3, 1);
    decrease(&v112, 13, 5);
    decrease(&v112, 8, 6);
    v80 = 0;
    n = 0xfff0bdc0;
    L1:
    v60 = (empty(&v112));
    if (!((v60) != 0)) {
        min = (pop_min(&v112));
        v68 = min;
        v64 = matrix[v68];
        if (v68 == 3) {
            v64 = 1;
        }
        if (v68 == 13) {
            v64 = 5;
        }
        if (v68 == 8) {
            v64 = 6;
        }
        if (v64 < n) {
        }
        n = v64;
        v80 = (((v80 * 31) + (v68 * 100)) + v64);
        goto L1;
    }
    v36 = ((v64 == n) ? 0 : (0 + 1));
    v40 = v80;
    v52 = (size(&v112));
    printf("sorted=%d checksum=%lld size=%d\n", v36, v40, v52);
    ret = ((v64 != n) ? 0 : (0 + 1));
    return ret;
}

MinHeap::MinHeap(int b) {
    long result;
    result = a;
    MinHeap(a, b);
    return result;
}

long MinHeap::push(int b, int c) {
    long v8;
    int v24;
    int v20;
    int v4;
    v8 = a;
    v24 = b;
    push_back(a, &v24);
    v20 = ((size(v8)) - 1);
    v4 = v20;
    (operator_index((v8 + 24), b))->f0 = v4;
    return (sift_up(v8, v20));
}

long MinHeap::decrease(int b, int c) {
    long v16;
    int v28;
    int v12;
    int v8;
    v16 = a;
    v28 = (operator_index((a + 24), b))->f0;
    if (!((v28 & (1<<31)) != 0)) {
        v12 = c;
        if (v12 < (operator_index(v16, v28))->f4) goto L1;
    }
    return a;
    L1:
    v8 = c;
    (operator_index(v16, v28))->f4 = v8;
    sift_up(v16, v28);
}

int MinHeap::empty() const {
    return (((size(a)) >= 1) ? 0 : (0 + 1));
}

int MinHeap::pop_min() {
    long v8;
    int v20;
    int v16;
    v8 = a;
    v20 = (operator_index(a, 1))->f0;
    (operator_index((v8 + 24), v20))->f0 = -1;
    v16 = ((size(v8)) - 1);
    if (v16 > 1) {
        swap_nodes(v8, 1, v16);
        pop_back(v8);
        sift_down(v8, 1);
    } else {
        pop_back(v8);
    }
}

int MinHeap::size() const {
    return ((size(a)) - 1);
}

long MinHeap::MinHeap_dtor() {
    return a;
}

MinHeap::MinHeap(int b) {
    long v0;
    v0 = a;
    vector(a);
    vector((v0 + 24), b, &-1);
    push_back(v0, &-1);
}

long MinHeap::sift_up(int b) {
    int v36;
    long v24;
    int v20;
    int v16;
    v36 = b;
    v24 = a;
    v20 = 0;
    while (v36 > 1) {
        v16 = (operator_index(v24, v36))->f4;
        v20 = ((v16 >= (operator_index(v24, (v36 / 2)))->f4) ? 0 : (0 + 1));
        if (v20) {
            swap_nodes(v24, v36, (v36 / 2));
            v36 /= 2;
        }
    }
}

long MinHeap::swap_nodes(int b, int c) {
    long v16;
    long index;
    int v12;
    int v28;
    long* t50;
    v16 = a;
    index = (operator_index(a, b));
    swap(index, (operator_index(v16, c)));
    v12 = b;
    (operator_index((v16 + 24), (operator_index(v16, b))->f0))->f0 = v12;
    v28 = c;
    t50 = operator_index((v16 + 24), (operator_index(v16, c))->f0);
    t50->f0 = v28;
    return t50;
}

long MinHeap::sift_down(int b) {
    int v36;
    long v8;
    int n;
    int v28;
    int v24;
    int v20;
    int v4;
    int v0;
    v36 = b;
    v8 = a;
    n = (size(a));
    v28 = (2 * v36);
    v24 = ((2 * v36) + 1);
    v20 = v36;
    while (v28 < n) {
        v4 = (operator_index(v8, v28))->f4;
        if (v4 < (operator_index(v8, v20))->f4) {
            v20 = v28;
        }
        if (v24 < n) {
            v0 = (operator_index(v8, v24))->f4;
            if (v0 < (operator_index(v8, v20))->f4) {
                v20 = v24;
            }
        }
        if (v20 == v36) {
            break;
        } else {
            swap_nodes(v8, v36, v20);
            v36 = v20;
        }
    }
}

long MinHeap::MinHeap_dtor() {
    return a;
}

