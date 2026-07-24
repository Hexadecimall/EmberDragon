// ember-gen seed=48 — intent-labeled corpus program (function name prefix = intent)
int parity__unsignedlong_48(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int gcd__int_48(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
struct M_list_sum__long_48{ long v; M_list_sum__long_48* next; }; long list_sum__long_48(M_list_sum__long_48* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct N_list_length__int_48{ int v; N_list_length__int_48* next; }; int list_length__int_48(N_list_length__int_48* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int is_power_of_two__unsignedlonglong_48(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsigned_48(unsigned v){ return v && !(v & (v-1)); }
unsigned fnv_hash__char_48(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
long power__long_48(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned power__unsigned_48(unsigned base, int e){ unsigned r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
void reverse_array__long_48(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
long fibonacci__long_48(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
int fibonacci__int_48(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
unsigned factorial__unsigned_48(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int factorial__int_48(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
float find_max__float_48(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
long find_max__long_48(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int collatz_steps__long_48(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned char crc8__char_48(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int find_min__int_48(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned find_min__unsigned_48(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned djb2_hash__char_48(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
float mean__float_48(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long abs_val__long_48(long v){ return v<0 ? -v : v; }
int popcount__unsignedlong_48(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlonglong_48(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
