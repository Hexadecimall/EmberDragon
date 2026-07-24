// ember-gen seed=11 — intent-labeled corpus program (function name prefix = intent)
struct M_list_sum__long_11{ long v; M_list_sum__long_11* next; }; long list_sum__long_11(M_list_sum__long_11* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
int palindrome_check__char_11(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int strlen2__char_11(const char* s){ int i=0; while(s[i]) i++; return i; }
unsigned fnv_hash__char_11(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void matrix_transpose__long_11(long* d, const long* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
long power__long_11(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int is_prime__long_11(long v){ if(v<2) return 0; for(long i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__int_11(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
double distance2__double_11(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
void to_upper__char_11(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned char crc8__char_11(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
float find_max__float_11(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int find_max__int_11(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
long isqrt__long_11(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_11(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int factorial__int_11(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
unsigned factorial__unsigned_11(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int linear_search__float_11(const float* a, int n, float key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void selection_sort__unsigned_11(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
int popcount__unsigned_11(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlonglong_11(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int max_subarray__int_11(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
