// ember-gen seed=32 — intent-labeled corpus program (function name prefix = intent)
unsigned sum_to_n__unsigned_32(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
long factorial__long_32(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
int factorial__int_32(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
int is_power_of_two__unsignedlonglong_32(unsigned long long v){ return v && !(v & (v-1)); }
int collatz_steps__long_32(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned djb2_hash__char_32(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
void insertion_sort__float_32(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void selection_sort__unsigned_32(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
int find_max__int_32(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int parity__unsignedlonglong_32(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
unsigned fnv_hash__char_32(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int count_trailing_zeros__unsignedlong_32(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
