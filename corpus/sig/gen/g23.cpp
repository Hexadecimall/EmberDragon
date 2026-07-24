// ember-gen seed=23 — intent-labeled corpus program (function name prefix = intent)
unsigned long long reverse_bits__unsignedlonglong_23(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int binary_search__long_23(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_23(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
void insertion_sort__int_23(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void matrix_transpose__long_23(long* d, const long* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__double_23(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int is_power_of_two__unsignedlonglong_23(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlong_23(unsigned long v){ return v && !(v & (v-1)); }
void reverse_array__unsigned_23(unsigned* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ unsigned t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__int_23(int* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ int t=a[i]; a[i]=a[j]; a[j]=t; } }
float mean__float_23(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_23(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
void xor_cipher__char_23(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
long array_sum__long_23(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned gcd__unsigned_23(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
int gcd__int_23(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
int power__int_23(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned fnv_hash__char_23(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
unsigned isqrt__unsigned_23(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
void bubble_sort__double_23(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__unsigned_23(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int find_min__int_23(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned find_min__unsigned_23(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
double distance2__double_23(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
float distance2__float_23(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
int palindrome_check__char_23(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int strlen2__char_23(const char* s){ int i=0; while(s[i]) i++; return i; }
int quick_partition__unsigned_23(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void selection_sort__long_23(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__unsigned_23(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
int collatz_steps__unsigned_23(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
long sum_to_n__long_23(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
