// ember-gen seed=38 — intent-labeled corpus program (function name prefix = intent)
int count_occurrences__unsigned_38(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
void reverse_array__double_38(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
long find_max__long_38(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
float find_max__float_38(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int linear_search__int_38(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
unsigned dot_product__unsigned_38(const unsigned* a, const unsigned* b, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int dot_product__int_38(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int palindrome_check__char_38(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
void matrix_transpose__unsigned_38(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void selection_sort__long_38(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
int fibonacci__int_38(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
unsigned fibonacci__unsigned_38(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
unsigned array_sum__unsigned_38(const unsigned* a, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
long array_sum__long_38(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int parity__unsignedlong_38(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsignedlonglong_38(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
long gcd__long_38(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_38(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
unsigned fnv_hash__char_38(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void xor_cipher__char_38(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int is_power_of_two__unsigned_38(unsigned v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlonglong_38(unsigned long long v){ return v && !(v & (v-1)); }
struct M_list_sum__long_38{ long v; M_list_sum__long_38* next; }; long list_sum__long_38(M_list_sum__long_38* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
int factorial__int_38(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
int collatz_steps__int_38(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_38(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned reverse_bits__unsigned_38(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned power__unsigned_38(unsigned base, int e){ unsigned r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int power__int_38(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
