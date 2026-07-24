// ember-gen seed=45 — intent-labeled corpus program (function name prefix = intent)
int count_occurrences__unsigned_45(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__int_45(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned djb2_hash__char_45(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
long abs_val__long_45(long v){ return v<0 ? -v : v; }
double abs_val__double_45(double v){ return v<0 ? -v : v; }
int binary_search__int_45(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__long_45(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int is_power_of_two__unsignedlonglong_45(unsigned long long v){ return v && !(v & (v-1)); }
int count_trailing_zeros__unsigned_45(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
void insertion_sort__unsigned_45(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int linear_search__unsigned_45(const unsigned* a, int n, unsigned key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__double_45(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void to_upper__char_45(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void bubble_sort__int_45(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned find_min__unsigned_45(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float find_min__float_45(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned factorial__unsigned_45(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int factorial__int_45(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
long max_subarray__long_45(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int max_subarray__int_45(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int quick_partition__int_45(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__unsigned_45(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
