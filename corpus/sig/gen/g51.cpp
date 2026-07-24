// ember-gen seed=51 — intent-labeled corpus program (function name prefix = intent)
unsigned fnv_hash__char_51(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
struct M_list_sum__int_51{ int v; M_list_sum__int_51* next; }; int list_sum__int_51(M_list_sum__int_51* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
int fibonacci__int_51(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
long fibonacci__long_51(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
int collatz_steps__long_51(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
long isqrt__long_51(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_51(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned checksum_sum__char_51(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
long abs_val__long_51(long v){ return v<0 ? -v : v; }
float abs_val__float_51(float v){ return v<0 ? -v : v; }
int max_subarray__int_51(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int palindrome_check__char_51(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int count_trailing_zeros__unsignedlonglong_51(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int dot_product__int_51(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
float dot_product__float_51(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int power__int_51(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
long power__long_51(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
long gcd__long_51(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
