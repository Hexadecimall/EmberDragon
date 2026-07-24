// ember-gen seed=15 — intent-labeled corpus program (function name prefix = intent)
void bubble_sort__unsigned_15(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_15(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void xor_cipher__char_15(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
void selection_sort__long_15(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
long fibonacci__long_15(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
int fibonacci__int_15(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
int collatz_steps__unsigned_15(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_15(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void insertion_sort__double_15(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int quick_partition__long_15(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__unsigned_15(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
long factorial__long_15(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
float dot_product__float_15(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
unsigned dot_product__unsigned_15(const unsigned* a, const unsigned* b, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
void matrix_transpose__float_15(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__int_15(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int find_min__int_15(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
long find_min__long_15(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float distance2__float_15(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
long isqrt__long_15(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
