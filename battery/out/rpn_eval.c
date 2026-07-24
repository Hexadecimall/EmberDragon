#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void eval_rpn(long a, char* obj3);
long apply(long a, char* obj2);


static const char str[] = "\"";
static const char str2[] = "\n";
static const char str3[] = "+";
static const char str4[] = "-";
static const char str5[] = "*";
static const char str6[] = "/";

class Record0 {
    long value;
    long data;
    long item;
    // std::basic_streambuf<char, std::char_traits<char>>::sgetc[abi:nqe220100]()::'lambda'()::operator()() const
    long operator_call() {
        return (__check_invariants(this->value));
    }
    // std::basic_streambuf<char, std::char_traits<char>>::sbumpc[abi:nqe220100]()::'lambda'()::operator()() const
    long operator_call() {
        return (__check_invariants(this->value));
    }
    // double& std::vector<double>::emplace_back<double const&>(double const&)::'lambda'()::operator()() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = *obj->item + 8;
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // double& std::vector<double>::emplace_back<double const&>(double const&)::'lambda0'()::operator()() const
    long operator_call() {
        long* obj;
        long value2;
        obj = this;
        value2 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = value2;
        return value2;
    }
    // double& std::vector<double>::emplace_back<double>(double&&)::'lambda'()::operator()() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = *obj->item + 8;
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // double& std::vector<double>::emplace_back<double>(double&&)::'lambda0'()::operator()() const
    long operator_call() {
        long* obj;
        long value2;
        obj = this;
        value2 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = value2;
        return value2;
        ; // -> loc_100001168 (tail-call / unrecovered target)
    }
};

int main(int argc, char** argv) {
    int n;
    int i;
    char v112;
    char v99;
    char buf[64];
    long dtor;
    long v32;
    long v24;
    long v16;
    // neon:  ldr    value0, [v153, #0x0]  // = 0x54470000000010005a47000000001000
    // neon:  ldr    value0, [v153, #0x10]  // = 0x6c470000000010007647000000001000
    n = 4;
    i = 0;
    while (i < n) {
        std::string v64 = (&v112)[i << 3];
        eval_rpn(&v64, &v99);
        memcpy(buf, "lG", 16);  // data assembled on the stack via NEON
        // neon:  ldr    d0, [sp, #0x28]
        dtor = basic_string_dtor(&v64);
        v32 = cout << str;
        v24 = v32 << (&v112)[i << 3];
        v24 << "\" = ";
        if (v99) {
            v16 = operator_lshcout;
            goto L1;
        }
        cout << "(error)\n";
        L1:
        i++;
    }
    return 0;
}

void eval_rpn(std::string const& a, bool& obj3) {
    long v448;
    char buf[64];
    long v144;
    long v104;
    long* obj2;
    int v44;
    long t21;
    long result;
    long t35;
    long t36;
    long t37;
    char v79;
    char buf3[64];
    char buf2[64];
    long t33;
    v448 = a;
    vector(buf);
    basic_istringstream(&v144, v448, 8);
    basic_string(&v104);
    *obj3 = 1;
    L1:
    obj2 = v144 >> v104;
    v44 = operator_bool(obj2 + (*obj2)[-24]);
    if (v44) {
        if (!(!(v104 == str3 != 0) && !(v104 == str4 != 0) && !(v104 == str5 != 0) && !(v104 == str6))) {
            t21 = size(buf);
            a = t21;
            if (t21 < 2) {
            *obj3 = 0;
            // neon:  mvni   v0.16b, #0x0
            result = a;
            return;
            }
            back(buf);
            // neon:  ldr    d0, [v458, #0x0]
            // neon:  str    d0, [sp, #0x58]
            pop_back(buf);
            back(buf);
            // neon:  ldr    d0, [v458, #0x0]
            // neon:  str    d0, [sp, #0x50]
            pop_back(buf);
            // neon:  ldr    d0, [sp, #0x50]
            // neon:  ldr    d1, [sp, #0x58]
            // neon:  str    d0, [sp, #0x10]
            a = apply(&v104, &v79);
            // neon:  ldr    d0, [sp, #0x10]
            // neon:  str    d0, [sp, #0x40]
            if (!(v79 != 0)) {
            *obj3 = 0;
            // neon:  mvni   v0.16b, #0x0
            result = a;
            return;
            }
            push_back(buf, buf3);
            goto L3;
        }
        atof(c_str(&v104));
        // neon:  str    d0, [sp, #0x8]
        // neon:  ldr    d0, [sp, #0x8]
        // neon:  str    d0, [sp, #0x38]
        push_back(buf, buf2);
        L3:
        goto L1;
    }
    t33 = size(buf);
    a = t33;
    if (t33 != 1) {
        *obj3 = 0;
        // neon:  mvni   v0.16b, #0x0
        result = a;
        return;
    } else {
        // neon:  ldr    d0, [v457, #0x0]
        result = back(buf);
        return;
    }
    t35 = basic_string_dtor(&v104);
    t36 = v144.basic_istringstream_dtor();
    t37 = vector_dtor(buf);
    return;
}

long apply(double a, double obj2, std::string const& p2, bool& p3) {
    long ret;
    long v16;
    long result;
    long t43;
    long t33;
    ret = a;
    // neon:  str    d1, [sp, #0x18]
    v16 = a;
    *obj2 = 1;
    if (v16 == str3) {
        // neon:  ldr    d1, [sp, #0x18]
        // neon:  neon.0x1e612800
        return ret;
    } else {
        if (v16 == str4) {
            // neon:  ldr    d1, [sp, #0x18]
            // neon:  neon.0x1e613800
            return ret;
        } else {
            if (v16 == str5) {
                // neon:  ldr    d1, [sp, #0x18]
                // neon:  neon.0x1e610800
                return ret;
            } else {
                t43 = v16 == str6;
                a = t43;
                if (t43) {
                    // neon:  ldr    d0, [sp, #0x18]
                    // neon:  neon.0x1e602008
                    if (t33 == 1) {
                        *obj2 = 0;
                        // neon:  mvni   v0.16b, #0x0
                        return a;
                    }
                    // neon:  ldr    d1, [sp, #0x18]
                    // neon:  neon.0x1e611800
                    return ret;
                } else {
                    *obj2 = 0;
                    // neon:  mvni   v0.16b, #0x0
                    return a;
                }
            }
        }
    }
    return result;
}

