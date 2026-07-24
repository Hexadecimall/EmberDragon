// ember-gen seed=30 — intent-labeled corpus program (function name prefix = intent)
unsigned long long reverse_bits__unsignedlonglong_30(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_30(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
float clamp__float_30(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
void insertion_sort__long_30(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
long sum_to_n__long_30(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int sum_to_n__int_30(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
double abs_val__double_30(double v){ return v<0 ? -v : v; }
int abs_val__int_30(int v){ return v<0 ? -v : v; }
int quick_partition__long_30(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__int_30(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
long max_subarray__long_30(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_30(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
float mean__float_30(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
unsigned gcd__unsigned_30(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
int binary_search__int_30(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__unsigned_30(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int palindrome_check__char_30(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
void bubble_sort__double_30(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void matrix_transpose__float_30(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
double find_min__double_30(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float find_min__float_30(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
void to_upper__char_30(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void selection_sort__int_30(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
unsigned fnv_hash__char_30(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int is_power_of_two__unsignedlong_30(unsigned long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlonglong_30(unsigned long long v){ return v && !(v & (v-1)); }
