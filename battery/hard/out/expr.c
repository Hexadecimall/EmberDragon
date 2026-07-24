#include <cstring>

static const long data[6] = {
    4503599627374880, 4503599627374890, 4503599627374902, 4503599627374912, 4503599627374924, 4503599627374935 
};

class Parser {
public:
    long value;
    long data;
    char item;
    long calc::Parser::expr() {
        long v4;
        long* obj;
        long* v5;
        long v0;
        long v1;
        term();
        v4 = obj->data;
        v5 = obj->value;
                v0 = v5->f23;
                if (!((v5->f23 & (1<<63)) != 0)) {
                    if (v4 < v0) goto loc_100000754;
                    goto loc_10000077c;
                }
                if (v4 >= v5->data) goto loc_10000077c;
                loc_100000754:
                v1 = v5;
                if (!((v0 & (1<<31)) == 0)) {
                    v1 = v5->value;
                }
            } while (*(v1 + v4) == 32);
            v1 = v5;
            if ((v0 & (1<<31)) == 0) continue;
            v1 = v5->value;
        }
        loc_10000077c:
        if (!((v0 & (1<<31)) != 0)) {
            if (v4 >= v0) goto loc_10000079c;
        } else {
            if (v4 >= v5->data) {
                loc_10000079c:
                v3 = 0;
                goto loc_1000007b0;
            }
        }
        while (!((v0 & (1<<31)) == 0)) {
            v5 = v5->value;
            v3 = *(v5 + v4);
            loc_1000007b0:
            while (v3 != 43) {
                if (v3 != 45) return obj;
                this->data = (v4 + 1);
                term(this);
                v4 = this->data;
                v5 = this->value;
                        v0 = v5->f23;
                        if (!((v5->f23 & (1<<63)) != 0)) {
                            if (v4 < v0) goto loc_100000820;
                            goto loc_100000848;
                        }
                        if (v4 >= v5->data) goto loc_100000848;
                        loc_100000820:
                        v1 = v5;
                        if (!((v0 & (1<<31)) == 0)) {
                            v1 = v5->value;
                        }
                    } while (*(v1 + v4) == 32);
                    v1 = v5;
                    if ((v0 & (1<<31)) == 0) continue;
                    v1 = v5->value;
                }
                loc_100000848:
                if (!((v0 & (1<<31)) != 0)) {
                    v1 = v0;
                } else {
                    v1 = v5->data;
                }
                v3 = 0;
                if (v4 >= v1) continue;
    long calc::Parser::term() {
        long v6;
        long* obj;
        long* v7;
        long v0;
        long v1;
        unary();
        v6 = obj->data;
        v7 = obj->value;
                v0 = v7->f23;
                if (!((v7->f23 & (1<<63)) != 0)) {
                    if (v6 < v0) goto loc_1000008dc;
                    goto loc_100000904;
                }
                if (v6 >= v7->data) goto loc_100000904;
                loc_1000008dc:
                v1 = v7;
                if (!((v0 & (1<<31)) == 0)) {
                    v1 = v7->value;
                }
            } while (*(v1 + v6) == 32);
            v1 = v7;
            if ((v0 & (1<<31)) == 0) continue;
            v1 = v7->value;
        loc_100000904:
        if (!((v0 & (1<<31)) != 0)) {
            if (v6 >= v0) goto loc_100000930;
            loc_100000914:
            if (!((v0 & (1<<31)) == 0)) {
                v7 = v7->value;
            }
            v5 = *(v7 + v6);
        } else {
            if (v6 < v7->data) goto loc_100000914;
            loc_100000930:
            v5 = 0;
        }
        v3 = 1;
        goto loc_100000948;
        while (!((v0 & (1<<31)) != 0)) {
            loc_100000944:
            v5 = *(v7 + v6);
            loc_100000948:
            while (!((v5 & 255) >= 47)) {
                v7 = (v3 << (v5 & 255));
                if ((v5 & 255) == 47) break;
                this->data = (v6 + 1);
                unary(this);
                v6 = (v5 & 255);
                if (!((v5 & 255) == 47)) {
                    if (v6 != 42) goto loc_1000009a0;
                } else {
                    if (v6 != 42) {
                        loc_1000009a0:
                        if (v6 != 0) {
                            goto loc_1000009c0;
                        }
                    } else {
                        this->item = v3;
                    }
                }
                loc_1000009c0:
                v6 = this->data;
                v7 = this->value;
                        v0 = v7->f23;
                        if (!((v7->f23 & (1<<63)) != 0)) {
                            if (v6 < v0) goto loc_100000a00;
                            goto loc_100000a28;
                        }
                        if (v6 >= v7->data) goto loc_100000a28;
                        loc_100000a00:
                        v1 = v7;
                        if (!((v0 & (1<<31)) == 0)) {
                            v1 = v7->value;
                        }
                    } while (*(v1 + v6) == 32);
                    v1 = v7;
                    if ((v0 & (1<<31)) == 0) continue;
                    v1 = v7->value;
                }
                loc_100000a28:
                if (!((v0 & (1<<31)) != 0)) {
                    v1 = v0;
                } else {
                    v1 = v7->data;
                }
                v5 = 0;
                if (v6 >= v1) continue;
        v7 = v7->value;
        goto loc_100000944;
    long calc::Parser::unary() {
        long v3;
        long* obj;
        long* v4;
        long v0;
        long v1;
        v3 = obj->data;
        v4 = obj->value;
                    v0 = v4->f23;
                    if (!((v4->f23 & (1<<63)) != 0)) {
                        if (v3 < v0) goto loc_100000abc;
                        goto loc_100000aec;
                    }
                    if (v3 >= v4->data) goto loc_100000aec;
                    loc_100000abc:
                    v1 = v4;
                    if (!((v0 & (1<<31)) == 0)) {
                        v1 = v4->value;
                    }
                } while (*(v1 + v3) == 32);
                v1 = v4;
                if (!((v0 & (1<<31)) == 0)) {
                    v1 = v4->value;
                }
            } while (*(v1 + v3) == 9);
            loc_100000aec:
            if (!((v0 & (1<<31)) != 0)) {
                if (v3 < v0) goto loc_100000b0c;
                goto loc_100000b3c;
            }
            if (v3 >= v4->data) goto loc_100000b3c;
            loc_100000b0c:
            v1 = v4;
            if ((v0 & (1<<31)) == 0) continue;
            v1 = v4->value;
        if (v0 == 45) {
            this->data = (v3 + 1);
            unary(this);
        } else {
            loc_100000b3c:
            atom(this);
            v3 = this->data;
            v4 = this->value;
                    v0 = v4->f23;
                    if (!((v4->f23 & (1<<63)) != 0)) {
                        if (v3 < v0) goto loc_100000b84;
                        goto loc_100000bac;
                    }
                    if (v3 >= v4->data) goto loc_100000bac;
                    loc_100000b84:
                    v1 = v4;
                    if (!((v0 & (1<<31)) == 0)) {
                        v1 = v4->value;
                    }
                } while (*(v1 + v3) == 32);
                v1 = v4;
                if ((v0 & (1<<31)) == 0) continue;
                v1 = v4->value;
            loc_100000bac:
            if (!((v0 & (1<<31)) != 0)) {
                if (v3 < v0) goto loc_100000bcc;
            } else {
                if (v3 < v4->data) {
                    loc_100000bcc:
                    if (!((v0 & (1<<31)) == 0)) {
                        v4 = v4->value;
                    }
                    if (!(*(v4 + v3) != 94)) {
                        this->data = (v3 + 1);
                        unary(this);
                        v3++;
                        if (!((v3 + 1) == 0)) {
                            v4 = ((v3 >= 0) ? v3 : (-v3));
                            v4--;
                            while (v4 != 1) {
                            }
                        }
                    }
                }
            }
        return obj;
    long calc::Parser::atom() {
        long* v36;
        long* obj;
        long v37;
        long v38;
        long v39;
        long v32;
        v36 = obj->value;
        v37 = obj->data;
                v38 = (v36->f23 & 255);
                v39 = v36->f23;
                v32 = (v36->f23 & 255);
                if (!((v36->f23 & (1<<31)) == 0)) {
                    v32 = v36->data;
                }
                if (v37 >= v32) goto loc_100000cb4;
                v32 = v36;
                if (!((v39 & (1<<31)) == 0)) {
                    v32 = v36->value;
                }
            } while (*(v32 + v37) == 32);
            v32 = v36;
            if ((v39 & (1<<31)) == 0) continue;
            v32 = v36->value;
        loc_100000cb4:
        v32 = v38;
        if (!((v39 & (1<<31)) != 0)) {
            if (v37 < v32) goto loc_100000cd4;
        } else {
            if (v37 < v36->data) {
                loc_100000cd4:
                v32 = v36;
                if (!((v39 & (1<<31)) == 0)) {
                    v32 = v36->value;
                }
                if (!(*(v32 + v37) != 40)) {
                    obj->data = (v37 + 1);
                    expr();
                    v38 = obj->value;
                    v39 = obj->data;
                            v32 = v38->f23;
                            if (!((v38->f23 & (1<<63)) != 0)) {
                                if (v39 < v32) goto loc_100000d3c;
                                goto loc_100000e1c;
                            }
                            if (v39 >= v38->data) goto loc_100000e1c;
                            loc_100000d3c:
                            v33 = v38;
                            if (!((v32 & (1<<31)) == 0)) {
                                v33 = v38->value;
                            }
                        } while (*(v33 + v39) == 32);
                        v33 = v38;
                        if ((v32 & (1<<31)) == 0) continue;
                        v33 = v38->value;
                    }
                }
        v39 = 0;
        goto loc_100000d98;
            loc_100000d74:
            if (*((v33 + v39) + v37) != 46) goto loc_100000df8;
            loc_100000d84:
            obj->data = ((v37 + v39) + 1);
            v38 = v36->f23;
            v39++;
            loc_100000d98:
            v33 = v38;
            if (!((v38 & (1<<7)) == 0)) {
                v33 = v36->data;
            }
            v32 = v38;
            if ((v37 + v39) >= v33) goto loc_100000df8;
            v33 = v36;
            if (!((v32 & (1<<31)) == 0)) {
                v33 = v36->value;
            }
            if (*((v33 + v39) + v37) < 48) continue;
            v33 = v36;
            if (!((v32 & (1<<31)) == 0)) {
                v33 = v36->value;
            }
            if (*((v33 + v39) + v37) >= 58) continue;
        v33 = v36->value;
        goto loc_100000d74;
        loc_100000df8:
        if (v39 != 0) {
            if ((v32 & (1<<31)) != 0) goto loc_100000e2c;
            if (v37 < v38) goto loc_100000e3c;
        } else {
            obj->item = 1;
            goto loc_100000f08;
            loc_100000e1c:
            if (!((v32 & (1<<31)) != 0)) {
                if (v39 < v32) goto loc_100000ee0;
                goto loc_100000f00;
                loc_100000e2c:
                v38 = v36->data;
                if (v37 >= v36->data) goto loc_100000f24;
                v36 = v36->value;
                loc_100000e3c:
                this = (((v38 - v37) < v39) ? (v38 - v37) : v39);
                if ((((v38 - v37) < v39) ? (v38 - v37) : v39) >= 0x7ffffffffffffff7) goto loc_100000f28;
                if (this < 22) {
                    v31 = this;
                } else {
                    ((((this & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((this & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000;
                    v35 = (operator_new((((this & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((this & 0x7ffffffffffffff8) + 8)));
                }
                memmove(v35, (v36 + v37), this);
                *(v35 + this) = 0;
                stod(&v8, 0);
                if ((v31 & (1<<31)) == 0) goto loc_100000f08;
                operator_delete(v8);
            } else {
                if (v39 < v38->data) {
                    loc_100000ee0:
                    this->data = (v39 + 1);
                    if (!((v38->f23 & (1<<31)) == 0)) {
                        v38 = v38->value;
                    }
                    if (*(v38 + v39) == 41) goto loc_100000f08;
                }
                loc_100000f00:
                this->item = 1;
            }
            loc_100000f08:
            return obj;
        }
        loc_100000f24:
        __throw_out_of_range();
        loc_100000f28:
        this = (__throw_length_error());
        if (!((v31 & (1<<31)) == 0)) {
            operator_delete(v8);
        }
        _Unwind_Resume(this);
        goto loc_100000d84;
};

struct Struct0 {
    long value;
    long data;
    char _pad16[7];
    char item;
};
int main(int argc, char** argv) {
    long v59;
    long v55;
    long v51;
    long v53;
    long v0;
    long v60;
    long v50;
    v59 = 0x408f400000000000;
    v55 = 0;
        v51 = *(data + v55);
        if ((strlen(*(data + v55))) >= 0x7ffffffffffffff7) goto loc_1000006d8;
        if (argc < 22) {
        } else {
            ((((argc & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((argc & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000;
            v59 = (((((argc & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((argc & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000);
            v53 = (operator_new((((argc & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((argc & 0x7ffffffffffffff8) + 8)));
        }
        memmove(v53, v51, argc);
        *(v53 + argc) = 0;
        expr(&v0);
        v60 = 0->item;
                v50 = v60;
                if (!((v60 & (1<<7)) == 0)) {
                    v50 = 0->data;
                }
                if (v59 >= v50) goto loc_100000680;
                v50 = 0;
                if (!((v60 & (1<<7)) == 0)) {
                    v50 = 0->value;
                }
            } while (*(v50 + v59) == 32);
            v50 = 0;
            if ((v60 & (1<<7)) == 0) continue;
            v50 = 0->value;
        loc_100000680:
        if (!((v60 & (1<<7)) == 0)) {
            v60 = 0->data;
        v59 = argc;
        v51 = ((v60 == v59) ? 0 : (0 + 1));
        if ((argc & (1<<31)) == 0) continue;
        operator_delete(v24);
    loc_1000006d8:
    v51 = (__throw_length_error());
    if (!((argc & (1<<31)) == 0)) {
        operator_delete(v24);
    _Unwind_Resume(v51);

