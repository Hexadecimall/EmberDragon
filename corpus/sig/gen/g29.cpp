// ember-gen seed=29 — intent-labeled corpus program (function name prefix = intent)
int linear_search__float_29(const float* a, int n, float key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
double array_sum__double_29(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
float array_sum__float_29(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
double distance2__double_29(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
long clamp__long_29(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned long reverse_bits__unsignedlong_29(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned reverse_bits__unsigned_29(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int binary_search__long_29(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__unsigned_29(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
unsigned fnv_hash__char_29(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
unsigned checksum_sum__char_29(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
long max_subarray__long_29(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_29(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int count_occurrences__unsigned_29(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__int_29(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
void insertion_sort__long_29(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__unsigned_29(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
unsigned djb2_hash__char_29(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
int quick_partition__unsigned_29(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void reverse_array__long_29(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__float_29(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
long isqrt__long_29(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_29(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int parity__unsignedlong_29(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
unsigned find_min__unsigned_29(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int is_prime__int_29(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
