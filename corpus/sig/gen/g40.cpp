// ember-gen seed=40 — intent-labeled corpus program (function name prefix = intent)
void matrix_transpose__double_40(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void xor_cipher__char_40(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
void selection_sort__double_40(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
int collatz_steps__int_40(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_40(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned reverse_bits__unsigned_40(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long long reverse_bits__unsignedlonglong_40(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int sum_to_n__int_40(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_40(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int gcd__int_40(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_40(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
long array_sum__long_40(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned array_sum__unsigned_40(const unsigned* a, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int popcount__unsignedlong_40(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsigned_40(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int strlen2__char_40(const char* s){ int i=0; while(s[i]) i++; return i; }
void insertion_sort__double_40(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__int_40(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
double abs_val__double_40(double v){ return v<0 ? -v : v; }
unsigned fnv_hash__char_40(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
float dot_product__float_40(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int linear_search__int_40(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__long_40(const long* a, int n, long key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
unsigned djb2_hash__char_40(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
int parity__unsigned_40(unsigned v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsignedlong_40(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
