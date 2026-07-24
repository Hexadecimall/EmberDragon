// ember-gen seed=54 — intent-labeled corpus program (function name prefix = intent)
int strlen2__char_54(const char* s){ int i=0; while(s[i]) i++; return i; }
struct M_list_sum__long_54{ long v; M_list_sum__long_54* next; }; long list_sum__long_54(M_list_sum__long_54* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__int_54{ int v; M_list_sum__int_54* next; }; int list_sum__int_54(M_list_sum__int_54* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
int is_power_of_two__unsignedlonglong_54(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlong_54(unsigned long v){ return v && !(v & (v-1)); }
void to_upper__char_54(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned djb2_hash__char_54(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
double mean__double_54(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
float mean__float_54(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long isqrt__long_54(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned char crc8__char_54(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int count_trailing_zeros__unsigned_54(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsignedlonglong_54(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
long matrix_trace__long_54(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
long sum_to_n__long_54(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int dot_product__int_54(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
