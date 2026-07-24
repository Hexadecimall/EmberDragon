// ember-gen seed=52 — intent-labeled corpus program (function name prefix = intent)
int quick_partition__unsigned_52(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
unsigned fnv_hash__char_52(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void to_upper__char_52(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
long dot_product__long_52(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int count_occurrences__long_52(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
void selection_sort__unsigned_52(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
long gcd__long_52(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_52(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
int fibonacci__int_52(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
void matrix_transpose__int_52(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__float_52(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int is_prime__unsigned_52(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__int_52(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
float array_sum__float_52(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
long isqrt__long_52(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int parity__unsignedlong_52(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
void bubble_sort__float_52(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
