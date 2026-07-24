// ember-gen seed=36 — intent-labeled corpus program (function name prefix = intent)
int quick_partition__long_36(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__unsigned_36(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void insertion_sort__float_36(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_36(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
unsigned reverse_bits__unsigned_36(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long long reverse_bits__unsignedlonglong_36(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int binary_search__unsigned_36(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_36(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
void bubble_sort__long_36(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_36(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned fnv_hash__char_36(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int is_prime__unsigned_36(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int collatz_steps__long_36(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__int_36(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int popcount__unsignedlong_36(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
void reverse_array__long_36(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__float_36(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
double dot_product__double_36(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int dot_product__int_36(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
void xor_cipher__char_36(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
long gcd__long_36(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
int gcd__int_36(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
void to_upper__char_36(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void selection_sort__double_36(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
