// ember-gen seed=6 — intent-labeled corpus program (function name prefix = intent)
double matrix_trace__double_6(const double* m, int n){ double s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int matrix_trace__int_6(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int binary_search__long_6(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_6(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
unsigned djb2_hash__char_6(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
float abs_val__float_6(float v){ return v<0 ? -v : v; }
int abs_val__int_6(int v){ return v<0 ? -v : v; }
void selection_sort__unsigned_6(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
void bubble_sort__int_6(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned factorial__unsigned_6(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
void to_upper__char_6(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
double dot_product__double_6(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int parity__unsignedlonglong_6(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
void insertion_sort__long_6(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__float_6(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
long gcd__long_6(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
int gcd__int_6(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
int collatz_steps__unsigned_6(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int max_subarray__int_6(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned find_max__unsigned_6(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
long find_max__long_6(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
void xor_cipher__char_6(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int sum_to_n__int_6(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
int popcount__unsignedlonglong_6(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
float mean__float_6(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
int count_trailing_zeros__unsigned_6(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int quick_partition__unsigned_6(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__long_6(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void reverse_array__float_6(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__double_6(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
