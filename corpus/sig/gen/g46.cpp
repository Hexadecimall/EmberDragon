// ember-gen seed=46 — intent-labeled corpus program (function name prefix = intent)
unsigned max_subarray__unsigned_46(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int max_subarray__int_46(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
void selection_sort__long_46(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
void matrix_transpose__double_46(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__unsigned_46(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
float clamp__float_46(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
long isqrt__long_46(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_46(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
void xor_cipher__char_46(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
long factorial__long_46(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
struct N_list_length__long_46{ long v; N_list_length__long_46* next; }; int list_length__long_46(N_list_length__long_46* h){ int c=0; while(h){ c++; h=h->next; } return c; }
unsigned checksum_sum__char_46(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
void insertion_sort__unsigned_46(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_46(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int linear_search__double_46(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
