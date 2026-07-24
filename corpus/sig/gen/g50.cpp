// ember-gen seed=50 — intent-labeled corpus program (function name prefix = intent)
int popcount__unsigned_50(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
double dot_product__double_50(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
float dot_product__float_50(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int collatz_steps__int_50(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void to_upper__char_50(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int count_trailing_zeros__unsigned_50(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int binary_search__long_50(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__unsigned_50(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
struct N_list_length__int_50{ int v; N_list_length__int_50* next; }; int list_length__int_50(N_list_length__int_50* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int parity__unsignedlong_50(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsignedlonglong_50(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
long isqrt__long_50(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
double matrix_trace__double_50(const double* m, int n){ double s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
long factorial__long_50(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
int is_prime__unsigned_50(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int count_occurrences__long_50(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__int_50(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int quick_partition__unsigned_50(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__long_50(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
double distance2__double_50(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
int fibonacci__int_50(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
int find_min__int_50(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float find_min__float_50(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
