// ember-gen seed=33 — intent-labeled corpus program (function name prefix = intent)
int fibonacci__int_33(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
long fibonacci__long_33(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
unsigned checksum_sum__char_33(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
int isqrt__int_33(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int popcount__unsignedlong_33(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
void to_upper__char_33(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
struct N_list_length__long_33{ long v; N_list_length__long_33* next; }; int list_length__long_33(N_list_length__long_33* h){ int c=0; while(h){ c++; h=h->next; } return c; }
void xor_cipher__char_33(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int is_prime__unsigned_33(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
unsigned fnv_hash__char_33(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int parity__unsignedlong_33(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
void matrix_transpose__double_33(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__float_33(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
long clamp__long_33(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int clamp__int_33(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
long factorial__long_33(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
int count_trailing_zeros__unsigned_33(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
struct M_list_sum__int_33{ int v; M_list_sum__int_33* next; }; int list_sum__int_33(M_list_sum__int_33* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
void selection_sort__unsigned_33(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
int palindrome_check__char_33(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
long sum_to_n__long_33(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
unsigned sum_to_n__unsigned_33(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
int max_subarray__int_33(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
