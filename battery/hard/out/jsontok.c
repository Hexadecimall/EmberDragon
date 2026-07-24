#include <cstring>

long next(struct Pair* obj, long a, long b, long c, long d);
long number(struct Pair* obj);

static long g_100000794[8192];   // unresolved data global — real bytes pending (lifter)

const char* s_name_a_u0062c_vals_1 = "{ \"name\": \"a\\u0062c\", \"vals\": [1, -2.5, 3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
const char* s_als_1_2_5_3e2_true_f = "als\": [1, -2.5, 3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
const char* s_2_5_3e2_true_false_n = ", -2.5, 3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";

static const unsigned char data[64] = {
    0x00, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x2c, 0x4c, 0x49, 0x49, 0x49, 0x49,
    0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x23, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,
    0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,
    0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x11, 0x49, 0x1a, 0x49, 0x49, 0x49, 0x49 
};

class Lexer {
public:
    char* data;
    long value;
    long jtok::Lexer::string() {
        long v24;
        long* obj;
        long v40;
        long v37;
        long* v39;
        long v32;
        long v33;
        v24 = 0;
        obj->value++;
        v40 = (obj->value + 1);
        v37 = v39;
        goto loc_100000914;
            loc_100000910:
            v40 = this->value;
            loc_100000914:
            v39 = this->data;
            v32 = *(this->data + 23);
            if (!((*(this->data + 23) & (1<<63)) != 0)) {
                if (v40 < v32) goto loc_10000093c;
                goto loc_100000b88;
            }
            if (v40 >= v39->value) goto loc_100000b88;
            loc_10000093c:
            v33 = v39;
            if (!((v32 & (1<<31)) == 0)) {
                v33 = v39->data;
            }
            if (*(v33 + v40) == 34) goto loc_100000b88;
            this->value = (v40 + 1);
            v32 = v39;
            v33 = (v40 + 1);
            if (!((v39->f23 & (1<<31)) == 0)) {
                v32 = v39->data;
            }
        } while (*(v32 + v40) != 92);
        v32 = v39->f23;
        if (!((v39->f23 & (1<<63)) == 0)) {
            v32 = v39->value;
        }
        if (v33 < v32) {
            this->value = (v40 + 2);
            v32 = (v40 + 2);
            v34 = v39;
            if (!((v39->f23 & (1<<31)) == 0)) {
                v34 = v39->data;
            }
            v33 = *(v34 + v33);
            if (!(*(v34 + v33) <= 109)) {
                if (v33 > 115) goto loc_1000009ec;
                if (v33 == 110) goto loc_100000a50;
                if (v33 != 114) goto loc_100000bcc;
                v33 = 13;
            } else {
                if (v33 != 34) {
                    if (v33 != 47) {
                        if (v33 != 92) {
                            goto loc_100000bcc;
                            loc_1000009ec:
                            if (v33 != 116) {
                                if (v33 != 117) goto loc_100000bcc;
                                v34 = v39->f23;
                                if (!((v39->f23 & (1<<63)) == 0)) {
                                    v34 = v39->value;
                                }
                                v33 = (v40 + 6);
                                if ((v40 + 6) >= v34) goto loc_100000bcc;
                                this->value = (v40 + 3);
                                v34 = (v40 + 3);
                                v35 = v39;
                                if (!((v39->f23 & (1<<31)) == 0)) {
                                    v35 = v39->data;
                                }
                                v32 = (*(v35 + v32) - 48);
                                v35 = *(v35 + v32);
                                if ((*(v35 + v32) - 48) < 9) goto loc_100000a7c;
                                if ((v35 - 97) >= 5) goto loc_100000a6c;
                                v32 = (v35 - 87);
                                goto loc_100000a7c;
                                loc_100000a50:
                                v33 = 10;
                            } else {
                                v33 = 9;
                            }
                        }
                    }
                }
            }
            push_back(&v8, v33);
            goto loc_100000910;
            loc_100000a6c:
            if (!((v35 - 65) >= 5)) {
                v32 = (v35 - 55);
                loc_100000a7c:
                if (!((v32 & (1<<31)) != 0)) {
                    this->value = (v40 + 4);
                    v35 = v39;
                    v36 = (v40 + 4);
                    if (!((v39->f23 & (1<<31)) == 0)) {
                        v35 = v39->data;
                    }
                    v34 = (*(v35 + v34) - 48);
                    v35 = *(v35 + v34);
                    if (!((*(v35 + v34) - 48) < 10)) {
                        if (!((v35 - 97) >= 6)) {
                            v34 = (v35 - 87);
                        } else {
                            if ((v35 - 65) >= 6) goto loc_100000bcc;
                            v34 = (v35 - 55);
                        }
                    }
                    if (!((v34 & (1<<31)) != 0)) {
                        this->value = (v40 + 5);
                        v40 = v39;
                        v35 = (v40 + 5);
                        if (!((v39->f23 & (1<<31)) == 0)) {
                            v40 = v39->data;
                        }
                        v40 = (*(v40 + v36) - 48);
                        v36 = *(v40 + v36);
                        if (!((*(v40 + v36) - 48) < 10)) {
                            if (!((v36 - 97) >= 6)) {
                                v40 = (v36 - 87);
                            } else {
                                if ((v36 - 65) >= 6) goto loc_100000bcc;
                                v40 = (v36 - 55);
                            }
                        }
                        if (!((v40 & (1<<31)) != 0)) {
                            this->value = v33;
                            if (!((v39->f23 & (1<<31)) == 0)) {
                                v39 = v39->data;
                            }
                            v39 = (*(v39 + v35) - 48);
                            v33 = *(v39 + v35);
                            if (!((*(v39 + v35) - 48) < 10)) {
                                if (!((v33 - 97) >= 6)) {
                                    v39 = (v33 - 87);
                                } else {
                                    if ((v33 - 65) >= 6) goto loc_100000bcc;
                                    v39 = (v33 - 55);
                                }
                            }
                            if (!((v39 & (1<<31)) != 0)) {
                                push_back(&v8, (v39 + ((v40 + ((v32 << 8) + (v34 << 4))) << 4)));
                                goto loc_100000910;
                                loc_100000b88:
                                if (!((v32 & (1<<31)) != 0)) {
                                    if (v40 >= v32) goto loc_100000bcc;
                                    loc_100000b94:
                                    this->value = (v40 + 1);
                                    v37->value = 6;
                                    if ((v31 & (1<<31)) != 0) goto loc_100000bfc;
                                    v37->f16 = v24;
                                    goto loc_100000bdc;
                                }
                                if (v40 < v39->value) goto loc_100000b94;
                            }
                        }
                    }
                }
            }
        }
        loc_100000bcc:
        v37->data = 11;
        v37->f31 = 0;
        v37->value = 0;
        loc_100000bdc:
        while (!((v31 & (1<<31)) == 0)) {
            operator_delete(v8);
            return obj;
            loc_100000bfc:
            __init_copy_ctor_external(v37);
        }
    }
    long jtok::Lexer::literal(char const* a, jtok::Kind b) {
        long v10;
        long* obj;
        long v1;
        long* v2;
        long v3;
        long v5;
        long v6;
        long v7;
        long* v8;
        long v9;
        long ccmp;
        long chained;
        long compare;
        long t18;
        v10 = *(obj->data + 23);
        v1 = a;
        v2 = v10;
        v3 = b;
        v5 = (strlen(a));
        v6 = (strlen(a));
        v7 = obj->value;
        v8 = obj->data;
        if (!((*(obj->data + 23) & (1<<63)) != 0)) {
            if (v6 == 1) goto loc_100000d94;
            v10 -= v7;
            if (v10 < v7) goto loc_100000d94;
            v9 = ((v10 < v5) ? v10 : v5);
        } else {
            if (v6 == 1) goto loc_100000d94;
            v10 = (v8->value - v7);
            if (v8->value < v7) goto loc_100000d94;
            v8 = v8->data;
            v9 = ((v10 < v5) ? v10 : v5);
        }
        /* ccmp v9, v6  (chained == compare) */
        if ((memcmp((v8 + v7), v1, ((v6 < v9) ? v6 : v9))) != 0) {
            this->value = (v7 + 1);
            v2->data = 11;
            v2->f31 = 0;
            v2->value = 0;
        } else {
            this->value = (v7 + v5);
            v2->data = v3;
            if ((strlen(v1)) >= 0x7ffffffffffffff7) goto loc_100000d98;
            v3 = obj;
            if (obj < 22) {
                v2->f31 = v3;
                this = (v2 + 8);
            } else {
                t18 = operator_new((((v3 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v3 & 0x7ffffffffffffff8) + 8));
                v2->value = t18;
                v2->f16 = v3;
                v2->f24 = (((((v3 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v3 & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000);
                this = t18;
            }
            memcpy(this, v1, v3);
            *(this + v3) = 0;
        }
        return obj;
        loc_100000d94:
        __throw_out_of_range();
        loc_100000d98:
        __throw_length_error();
    }
};

int main(int argc, char** argv) {
    long obj;
    long v139;
    long v141;
    int v8;
    long v144;
    char v39;
    long* v145;
    long v40;
    long v140;
    long v137;
    long v138;
    obj = (operator_new(104));
    *(s_2_5_3e2_true_false_n + 32) = s_2_5_3e2_true_false_n;
    *(s_2_5_3e2_true_false_n + 40) = "3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
    *(s_2_5_3e2_true_false_n + 95) = "eak\" } }";
    *s_name_a_u0062c_vals_1 = s_name_a_u0062c_vals_1;
    *(s_name_a_u0062c_vals_1 + 8) = ": \"a\\u0062c\", \"vals\": [1, -2.5, 3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
    *(s_als_1_2_5_3e2_true_f + 16) = "62c\", \"vals\": [1, -2.5, 3e2, true, false, null],  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
    *(s_als_1_2_5_3e2_true_f + 24) = s_als_1_2_5_3e2_true_f;
    *(s_als_1_2_5_3e2_true_f + 103) = 0;
    v139 = 0x739d0383;
        *(v141 + (v8 << 2)) = (*(v141 + (v8 << 2)) + 1);
        v144 = v39;
        v145 = ((v39 < 0) ? (next(&v40)) : v140);
        v137 = ((v39 < 0) ? v138 : v39);
        if (((v39 >= 0) ? v138 : v39) == 0) {
            v137--;
            v138 = (v139 ^ v145->f1);
            v139 = ((v139 ^ v145->f1) * 435);
            while (v137 != 1) {
            }
        }
        if ((v144 & (1<<31)) == 0) continue;
        operator_delete();
    }
    if (!((v79 & (1<<31)) == 0)) {
        operator_delete(obj);
    }
    if (!((v79 & (1<<31)) == 0)) {
        operator_delete(obj);
    }
    _Unwind_Resume(v139);

struct Pair {
    long value;
    long data;
};
struct ListNode {
    ListNode* next;
    long size;
    char _pad16[7];
    char flag;
};
struct Struct0 {
    int value;
    char _pad4[4];
    short data;
    char _pad10[21];
    char item;
};
long jtok::Lexer::next() {
    long v4;
    struct ListNode* v0;
    long v1;
    v4 = obj->data;
    v0 = obj->value;
                    v1 = v0->flag;
                    if (!((v0->flag & (1<<63)) != 0)) {
                        if (v4 < v1) goto L1;
                        goto loc_100000754;
                    }
                    if (v4 >= v0->size) goto loc_100000754;
                    L1:
                    if (!((v1 & (1<<31)) == 0)) {
                    }
                } while (*(v2 + v4) == 32);
                v2 = v0;
                if (!((v1 & (1<<31)) == 0)) {
                    v2 = v0->next;
                }
            } while (*(v2 + v4) == 9);
            v2 = v0;
            if (!((v1 & (1<<31)) == 0)) {
                v2 = v0->next;
            }
        } while (*(v2 + v4) == 10);
        v2 = v0;
        if ((v1 & (1<<31)) == 0) continue;
        v2 = v0->next;
    loc_100000754:
    if (!((v1 & (1<<31)) != 0)) {
        if (v4 >= v1) goto loc_1000007a4;
        loc_100000764:
        if (!((v1 & (1<<31)) == 0)) {
            v0 = v0->next;
        }
        v0 = *(v0 + v4);
        v1 = (*(v0 + v4) - 34);
        if ((*(v0 + v4) - 34) >= 91) goto loc_1000008b8;
        /* br (g_100000794 + (data[v1] << 2)) (indirect) */
        goto loc_1000008d8;
    }
    if (v4 < v0->size) goto loc_100000764;
    loc_1000007a4:
    v4 = 12;
    loc_1000007a8:
    v3->value = v4;
    v3->item = 0;
    v3->data = 0;
    return obj;
    loc_1000008b8:
    if (!((v0 - 48) >= 9)) {
        goto loc_100000d9c;
    }
    obj->data = (v4 + 1);
    v4 = 11;
    goto loc_1000007a8;
    loc_1000008d8:
    ; // -> loc_1000008d8 (tail-call / unrecovered target)
    loc_100000d9c:
    ; // -> loc_100000d9c (tail-call / unrecovered target)

struct Struct1 {
    char _pad0[8];
    long value;
    long data;
    long item;
    char member;
};
long jtok::Lexer::number() {
    long v7;
    long v8;
    long v5;
    struct ListNode* v6;
    long v0;
    v7 = obj->data;
    v8 = obj->value;
    v5 = obj->data;
    v6 = obj->value;
    if (!((*(obj->value + 23) & (1<<31)) == 0)) {
        v8 = v6->next;
    }
    if (!(*(v8 + v7) != 45)) {
        v7 = (v5 + 1);
        L1:
        obj->data = v7;
    }
    v8 = v6->flag;
    if (!((v6->flag & (1<<63)) != 0)) {
        if (v7 < v8) goto L2;
    } else {
        if (v7 < v6->size) {
            L2:
            v0 = v6;
            if (!((v8 & (1<<31)) == 0)) {
                v0 = v6->next;
            }
            if (!((*(v0 + v7) - 48) >= 9)) {
                v7++;
                goto L1;
            }
        }
    }
    if (!((v8 & (1<<31)) != 0)) {
        if (v7 >= v8) goto loc_100000eb4;
        L3:
        v0 = v6;
        if (!((v8 & (1<<31)) == 0)) {
            v0 = v6->next;
        }
        if (*(v0 + v7) != 46) goto loc_100000ecc;
        v0 = (v7 + 1);
            obj->data = v0;
            v7 = v0;
            v8 = v6->flag;
            if (!((v6->flag & (1<<63)) != 0)) {
                if (v7 < v8) goto L4;
                break;
            }
            if (v7 >= v6->size) break;
            L4:
            v0 = v6;
            if ((v8 & (1<<31)) == 0) continue;
            v0 = v6->next;
        }
    }
    if (v7 < v6->size) goto loc_100000e38;
    loc_100000eb4:
    if (!((v8 & (1<<31)) != 0)) {
        while (v7 >= v8) {
            goto loc_100000fbc;
            loc_100000ecc:
            if ((v8 & (1<<31)) == 0) continue;
        }
    }
    if (v7 < v6->size) {
        v0 = v6;
        if (!((v8 & (1<<31)) == 0)) {
            v0 = v6->next;
        }
        if (!(*(v0 + v7) == 101)) {
            v0 = v6;
            if (!((v8 & (1<<31)) == 0)) {
                v0 = v6->next;
            }
            if (*(v0 + v7) != 69) goto loc_100000fbc;
        }
        obj->data = (v7 + 1);
        v8 = v6->flag;
        v0 = (v7 + 1);
        if (!((v6->flag & (1<<63)) != 0)) {
            if (v0 < v8) goto loc_100000f38;
        } else {
            if (v0 < v6->size) {
                loc_100000f38:
                v1 = v6;
                if (!((v8 & (1<<31)) == 0)) {
                    v1 = v6->next;
                }
                if (!(*(v1 + v0) == 43)) {
                    v1 = v6;
                    if (!((v8 & (1<<31)) == 0)) {
                        v1 = v6->next;
                    }
                    if (*(v1 + v0) != 45) goto loc_100000f70;
                }
                v0 = (v7 + 2);
                loc_100000f6c:
                obj->data = v0;
            }
        }
        loc_100000f70:
        v8 = v6->flag;
        if (!((v6->flag & (1<<63)) != 0)) {
            if (v0 < v8) goto loc_100000f94;
        } else {
            if (v0 < v6->size) {
                loc_100000f94:
                v7 = v6;
                if (!((v8 & (1<<31)) == 0)) {
                    v7 = v6->next;
                }
                if (!((*(v7 + v0) - 48) >= 9)) {
                    v0++;
                    goto loc_100000f6c;
                }
            }
        }
        v7 = v0;
    }
    loc_100000fbc:
    v2->value = 7;
    v3 = v2;
    if (!((v8 & (1<<31)) != 0)) {
        if (v5 < v8) goto loc_100000fec;
    } else {
        v8 = v6->size;
        if (v5 < v6->size) {
            v6 = v6->next;
            loc_100000fec:
            v4 = (((v8 - v5) < (v7 - v5)) ? (v8 - v5) : (v7 - v5));
            if ((((v8 - v5) < (v7 - v5)) ? (v8 - v5) : (v7 - v5)) >= 0x7ffffffffffffff7) goto loc_100001074;
            if (v4 < 22) {
                v2->member = v4;
            } else {
                t22 = operator_new((((v4 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v4 & 0x7ffffffffffffff8) + 8));
                v2->value = t22;
                v2->data = v4;
                v2->item = (((((v4 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v4 & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000);
                v3 = t22;
            *(v3 + v4) = 0;
            return (memmove(v3, (v6 + v5), v4));
    __throw_out_of_range();
    loc_100001074:
    __throw_length_error();

