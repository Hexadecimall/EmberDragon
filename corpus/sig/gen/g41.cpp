// ember-gen seed=41 — intent-labeled corpus program (function name prefix = intent)
int count_trailing_zeros__unsignedlonglong_41(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int isqrt__int_41(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long isqrt__long_41(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int fibonacci__int_41(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
double mean__double_41(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long matrix_trace__long_41(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int matrix_trace__int_41(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
unsigned long long reverse_bits__unsignedlonglong_41(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_41(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
struct M_list_sum__int_41{ int v; M_list_sum__int_41* next; }; int list_sum__int_41(M_list_sum__int_41* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
int popcount__unsigned_41(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
float distance2__float_41(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
void bubble_sort__unsigned_41(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int collatz_steps__int_41(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__unsigned_41(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned char crc8__char_41(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int count_occurrences__long_41(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned fnv_hash__char_41(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
unsigned checksum_sum__char_41(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
int linear_search__int_41(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__float_41(const float* a, int n, float key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
