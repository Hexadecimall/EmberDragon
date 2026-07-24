#include <iostream>

using namespace std;

long Vec2(long a);
long operator_add(long a, long b);
long scaled(long a);
void x(long a);
void y(long a);
long operator_eq(long a, long b);

const char* s_str = ", ";
const char* s_str_2 = ")\n";

static const char str[] = "\n";

int main(int argc, char** argv) {
    int result;
    char buf3[64];
    char buf2[64];
    long v152;
    char buf4[64];
    long t4;
    long v32;
    long t7;
    long v40;
    long t11;
    long v48;
    long t14;
    long v72;
    char buf[64];
    long t19;
    long v120;
    long cond;
    result = 0;
    // neon:  neon.0x1e6e1000
    // neon:  str    d0, [sp, #0x58]
    // neon:  neon.0x1e601001
    // neon:  str    d1, [sp, #0x18]
    Vec2(buf3);
    // neon:  neon.0x1e611000
    // neon:  neon.0x1e7e1001
    Vec2(buf2);
    buf3 + buf2;
    // neon:  orr    v2.16b, v0.16b, v0.16b
    // neon:  ldr    d0, [sp, #0x18]
    v152 = scaled(buf4);
    t4 = cout << "sum   = (";
    v32 = t4;
    x(buf4, t4);
    operator_lshv32;
    t7 = operator_lsh&s_str;
    v40 = t7;
    y(buf4, t7);
    operator_lshv40;
    operator_lsh&s_str_2;
    t11 = cout << "scale = (";
    v48 = t11;
    x(&v152, t11);
    operator_lshv48;
    t14 = operator_lsh&s_str;
    v72 = t14;
    y(&v152, t14);
    operator_lshv72;
    operator_lsh&s_str_2;
    // neon:  ldr    d1, [sp, #0x58]
    // neon:  neon.0x1e621000
    Vec2(buf);
    t19 = cout << "equal = ";
    v120 = t19;
    operator_eq(buf4, buf, t19);
    v120 << (cond ? "yes" : "no");
    operator_lshstr;
    return result;
}

Vec2::Vec2(double a, double p1) {
    long result;
    // neon:  str    d0, [sp, #0x10]
    // neon:  str    d1, [sp, #0x8]
    result = a;
    // neon:  ldr    d0, [sp, #0x10]
    // neon:  ldr    d1, [sp, #0x8]
    Vec2(a);
    return result;
}

long Vec2::operator+(Vec2 const& b) const {
    char buf[64];
    // neon:  ldr    d0, [v17, #0x0]
    // neon:  ldr    d1, [v18, #0x0]
    // neon:  neon.0x1e612800
    // neon:  ldr    d1, [v17, #0x8]
    // neon:  ldr    d2, [v17, #0x8]
    // neon:  neon.0x1e622821
    // neon:  ldr    d0, [sp, #0x10]
    // neon:  ldr    d1, [sp, #0x18]
    return (Vec2(buf));
}

long Vec2::scaled(double a) const {
    char buf[64];
    // neon:  str    d0, [sp, #0x0]
    // neon:  ldr    d0, [v17, #0x0]
    // neon:  ldr    d1, [sp, #0x0]
    // neon:  neon.0x1e610800
    // neon:  ldr    d1, [v17, #0x8]
    // neon:  ldr    d2, [sp, #0x0]
    // neon:  neon.0x1e620821
    // neon:  ldr    d0, [sp, #0x10]
    // neon:  ldr    d1, [sp, #0x18]
    return (Vec2(buf));
}

void Vec2::x() const {
    // neon:  ldr    d0, [v9, #0x0]
    return;
}

void Vec2::y() const {
    // neon:  ldr    d0, [v9, #0x8]
    return;
}

long Vec2::operator==(Vec2 const& b) const {
    int v12;
    long cond;
    // neon:  ldr    d0, [v25, #0x0]
    // neon:  ldr    d1, [v25, #0x0]
    // neon:  neon.0x1e612000
    v12 = 0;
    if ( == ) {
        // neon:  ldr    d0, [v25, #0x8]
        // neon:  ldr    d1, [v25, #0x8]
        // neon:  neon.0x1e612000
        v12 = (cond ? 0 : 1);
        return (v12 & 1);
    }
    return (v12 & 1);
}

Vec2::Vec2(double a, double p1) {
    long result;
    result = a;
    // neon:  str    d0, [sp, #0x10]
    // neon:  str    d1, [sp, #0x8]
    // neon:  ldr    d0, [sp, #0x10]
    // neon:  str    d0, [v25, #0x0]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  str    d0, [v25, #0x8]
    return result;
}

