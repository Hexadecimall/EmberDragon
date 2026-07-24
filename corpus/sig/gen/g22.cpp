// ember-gen seed=22 — intent-labeled corpus program (function name prefix = intent)
int collatz_steps__unsigned_22(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void insertion_sort__int_22(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__double_22(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
unsigned clamp__unsigned_22(unsigned v, unsigned lo, unsigned hi){ return v<lo ? lo : (v>hi ? hi : v); }
float clamp__float_22(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
int strlen2__char_22(const char* s){ int i=0; while(s[i]) i++; return i; }
int is_prime__unsigned_22(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
unsigned find_min__unsigned_22(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned array_sum__unsigned_22(const unsigned* a, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
long array_sum__long_22(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int binary_search__long_22(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_22(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
long isqrt__long_22(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_22(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int popcount__unsigned_22(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlong_22(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int quick_partition__long_22(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
unsigned fibonacci__unsigned_22(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
long fibonacci__long_22(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
