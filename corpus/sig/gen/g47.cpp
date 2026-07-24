// ember-gen seed=47 — intent-labeled corpus program (function name prefix = intent)
int linear_search__int_47(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void reverse_array__float_47(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__int_47(int* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ int t=a[i]; a[i]=a[j]; a[j]=t; } }
struct M_list_sum__long_47{ long v; M_list_sum__long_47* next; }; long list_sum__long_47(M_list_sum__long_47* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__int_47{ int v; M_list_sum__int_47* next; }; int list_sum__int_47(M_list_sum__int_47* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
int palindrome_check__char_47(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
long matrix_trace__long_47(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int matrix_trace__int_47(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int count_trailing_zeros__unsignedlonglong_47(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsignedlong_47(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
double abs_val__double_47(double v){ return v<0 ? -v : v; }
long abs_val__long_47(long v){ return v<0 ? -v : v; }
unsigned fnv_hash__char_47(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
double mean__double_47(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long max_subarray__long_47(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
void selection_sort__double_47(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__unsigned_47(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
struct N_list_length__int_47{ int v; N_list_length__int_47* next; }; int list_length__int_47(N_list_length__int_47* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int sum_to_n__int_47(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_47(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
void matrix_transpose__float_47(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void bubble_sort__unsigned_47(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__long_47(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
