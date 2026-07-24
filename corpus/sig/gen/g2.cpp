// ember-gen seed=2 — intent-labeled corpus program (function name prefix = intent)
int binary_search__unsigned_2(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__long_2(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
unsigned char crc8__char_2(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int dot_product__int_2(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
long fibonacci__long_2(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
unsigned fibonacci__unsigned_2(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
int is_power_of_two__unsignedlong_2(unsigned long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsigned_2(unsigned v){ return v && !(v & (v-1)); }
long abs_val__long_2(long v){ return v<0 ? -v : v; }
int sum_to_n__int_2(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
void selection_sort__unsigned_2(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
float distance2__float_2(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
void reverse_array__double_2(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__long_2(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
