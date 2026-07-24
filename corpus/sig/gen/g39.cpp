// ember-gen seed=39 — intent-labeled corpus program (function name prefix = intent)
unsigned abs_val__unsigned_39(unsigned v){ return v<0 ? -v : v; }
float mean__float_39(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
void insertion_sort__unsigned_39(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__float_39(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
long power__long_39(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
long isqrt__long_39(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_39(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long max_subarray__long_39(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_39(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
void bubble_sort__double_39(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__int_39(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned char crc8__char_39(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
double array_sum__double_39(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
float array_sum__float_39(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int is_prime__long_39(long v){ if(v<2) return 0; for(long i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__unsigned_39(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
float clamp__float_39(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
long clamp__long_39(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int factorial__int_39(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
unsigned factorial__unsigned_39(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int count_trailing_zeros__unsignedlonglong_39(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsignedlong_39(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
