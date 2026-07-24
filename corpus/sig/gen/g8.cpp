// ember-gen seed=8 — intent-labeled corpus program (function name prefix = intent)
float dot_product__float_8(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
double dot_product__double_8(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
double mean__double_8(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
float mean__float_8(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
int gcd__int_8(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
long gcd__long_8(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
int power__int_8(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
void selection_sort__double_8(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__float_8(float* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; float t=a[i]; a[i]=a[m]; a[m]=t; } }
int binary_search__long_8(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_8(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int clamp__int_8(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
double clamp__double_8(double v, double lo, double hi){ return v<lo ? lo : (v>hi ? hi : v); }
int collatz_steps__unsigned_8(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_8(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned fnv_hash__char_8(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int sum_to_n__int_8(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
unsigned sum_to_n__unsigned_8(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
unsigned max_subarray__unsigned_8(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int max_subarray__int_8(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int parity__unsignedlonglong_8(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
void bubble_sort__double_8(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
