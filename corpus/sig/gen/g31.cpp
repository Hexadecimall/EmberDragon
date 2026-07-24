// ember-gen seed=31 — intent-labeled corpus program (function name prefix = intent)
void to_upper__char_31(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
long find_min__long_31(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned max_subarray__unsigned_31(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned factorial__unsigned_31(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
long factorial__long_31(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
void selection_sort__double_31(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
int gcd__int_31(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
unsigned checksum_sum__char_31(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
struct N_list_length__int_31{ int v; N_list_length__int_31* next; }; int list_length__int_31(N_list_length__int_31* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__long_31{ long v; N_list_length__long_31* next; }; int list_length__long_31(N_list_length__long_31* h){ int c=0; while(h){ c++; h=h->next; } return c; }
long isqrt__long_31(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_31(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long matrix_trace__long_31(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
