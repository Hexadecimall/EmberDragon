// ember-gen seed=4 — intent-labeled corpus program (function name prefix = intent)
int is_power_of_two__unsignedlong_4(unsigned long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsigned_4(unsigned v){ return v && !(v & (v-1)); }
unsigned find_max__unsigned_4(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int max_subarray__int_4(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
void to_upper__char_4(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int factorial__int_4(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
int binary_search__long_4(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__unsigned_4(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int linear_search__double_4(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void insertion_sort__float_4(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_4(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
long clamp__long_4(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned long reverse_bits__unsignedlong_4(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned reverse_bits__unsigned_4(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int quick_partition__int_4(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__long_4(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
double find_min__double_4(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
long find_min__long_4(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float abs_val__float_4(float v){ return v<0 ? -v : v; }
unsigned abs_val__unsigned_4(unsigned v){ return v<0 ? -v : v; }
