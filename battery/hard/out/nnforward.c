#include <cstring>
#include <cmath>

long dense(long a, long b, long c, long d);
long softmax(long a);
long relu(long a);

static const double coeffs[8] = {
    0.2, -0.15, 0.3, 0.1, -0.25, 0.4, -0.05, 0.22 
};
static const double coeffs2[8] = {
    0.1, -0.2, 0.05, 0, 0.15, -0.07, 0.21, -0.11 
};
static const double coeffs3[8] = {
    0.21, -0.11, 0.34, 0.07, -0.25, 0.18, -0.3, 0.22 
};
static const double coeffs4[8] = {
    0.04, -0.12, 0.08, 0, 0.11, 0.3, -0.18, 0.22 
};
static const double coeffs5[8] = {
    0.3, -0.18, 0.22, 0.1, -0.25, -0.15, 0.28, 0.06 
};
static const double coeffs6[3] = {
    0.02, -0.05, 0.03 
};

struct Struct0 {
    long value;
};
int main(int argc, char** argv) {
    char v193;
    long v144;
    long v104;
    long v56;
    long v80;
    int v52;
    int i;
    int v44;
    dense(v193, coeffs, coeffs2, (1 & 1));
    dense((&v144), coeffs3, coeffs4, (1 & 1));
    dense((&v104), coeffs5, coeffs6, (0 & 1));
    v56 = (softmax(&(&v80)));
    v52 = 0;
    i = 1;
    while (i < 3) {
        operator_index((operator_index(&v56, i))->f0, v52);
        if (i > 3) {
            v52 = i;
        }
        i++;
    }
    operator_index(&v56, v52);
    v44 = 0x4059000000000000;
    if (v44 > 99) {
        v44 = 99;
    }
}

// std::array<double, 6> (anonymous namespace)::dense<6, 4>(std::array<double, 4> const&, std::array<std::array<double, 4>, 6> const&, std::array<double, 6> const&, bool)
long dense(long a, long b, long c, long d) {
    long v73;
    int j;
    long v40;
    int i;
    struct Struct0* v16;
    long v8;
    struct Struct0* obj;
    memset(v73, 0, 48);
    j = 0;
    while (j < 6) {
        v40 = (operator_index(c, j))->f0;
        i = 0;
        while (i < 4) {
            operator_index(b, j);
            v16 = (operator_index(i))->f0;
            operator_index(a, i);
            v40 = v16;
            i++;
        }
        if (d) {
            v8 = (relu(v40));
        } else {
            v8 = v40;
        }
        obj = v8;
        operator_index(v8, j);
        obj->value = obj;
        j++;
    }
}

// std::array<double, 5> (anonymous namespace)::dense<5, 6>(std::array<double, 6> const&, std::array<std::array<double, 6>, 5> const&, std::array<double, 5> const&, bool)
long dense(long a, long b, long c, long d) {
    long v73;
    int j;
    long v40;
    int i;
    struct Struct0* v16;
    long v8;
    struct Struct0* obj;
    memset(v73, 0, 40);
    j = 0;
    while (j < 5) {
        v40 = (operator_index(c, j))->f0;
        i = 0;
        while (i < 6) {
            operator_index(b, j);
            v16 = (operator_index(i))->f0;
            operator_index(a, i);
            v40 = v16;
            i++;
        }
        if (d) {
            v8 = (relu(v40));
        } else {
            v8 = v40;
        }
        obj = v8;
        operator_index(v8, j);
        obj->value = obj;
        j++;
    }
}

// std::array<double, 3> (anonymous namespace)::dense<3, 5>(std::array<double, 5> const&, std::array<std::array<double, 5>, 3> const&, std::array<double, 3> const&, bool)
long dense(long a, long b, long c, long d) {
    int j;
    long v32;
    int i;
    struct Struct0* v16;
    long v8;
    struct Struct0* obj;
    long __addr0;
    j = 0;
    while (j < 3) {
        v32 = (operator_index(c, j))->f0;
        i = 0;
        while (i < 5) {
            operator_index(b, j);
            v16 = (operator_index(i))->f0;
            operator_index(a, i);
            v32 = v16;
            i++;
        }
        if (d) {
            v8 = (relu(v32));
        } else {
            v8 = v32;
        }
        obj = v8;
        operator_index(&__addr0, j);
        obj->value = obj;
        j++;
    }
}

// (anonymous namespace)::softmax(std::array<double, 3ul> const&)
long softmax(long a) {
    long* t27;
    long v56;
    int k;
    long* t29;
    long v40;
    int j;
    struct Struct0* obj;
    long v24;
    int i;
    long* t34;
    long __addr0;
    t27 = operator_index(a, 0);
    v56 = t27->f0;
    k = 1;
    while (k < 3) {
        v89 = (operator_index(a, k))->f0;
        if (k > 3) {
            t29 = operator_index(a, k);
            v56 = t29->f0;
        }
        k++;
    }
    v40 = a;
    j = 0;
    while (j < 3) {
        obj = (exp((operator_index(a, j))->f0, v56));
        v24 = &__addr0;
        operator_index(&__addr0, j);
        obj->value = v24;
        operator_index(v24, j);
        v40 = v40;
        j++;
    }
    i = 0;
    while (i < 3) {
        t34 = operator_index(&__addr0, i);
        *t34->f0 = t34->f0;
        i++;
    }
}

// (anonymous namespace)::relu(double)
long relu(long a) {
    long v32;
    if (v32 > 3) {
    } else {
    }
}

