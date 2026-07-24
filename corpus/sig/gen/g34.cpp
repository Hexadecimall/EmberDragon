// ember-gen seed=34 — intent-labeled corpus program (function name prefix = intent)
double array_sum__double_34(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int array_sum__int_34(const int* a, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
void matrix_transpose__double_34(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void insertion_sort__int_34(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
long isqrt__long_34(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_34(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long clamp__long_34(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned clamp__unsigned_34(unsigned v, unsigned lo, unsigned hi){ return v<lo ? lo : (v>hi ? hi : v); }
double mean__double_34(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
float mean__float_34(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long dot_product__long_34(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int linear_search__double_34(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__long_34(const long* a, int n, long key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void reverse_array__double_34(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
int is_power_of_two__unsignedlonglong_34(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlong_34(unsigned long v){ return v && !(v & (v-1)); }
int count_trailing_zeros__unsigned_34(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int quick_partition__unsigned_34(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
unsigned long reverse_bits__unsignedlong_34(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
float abs_val__float_34(float v){ return v<0 ? -v : v; }
int abs_val__int_34(int v){ return v<0 ? -v : v; }
int count_occurrences__int_34(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__unsigned_34(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
struct N_list_length__long_34{ long v; N_list_length__long_34* next; }; int list_length__long_34(N_list_length__long_34* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__int_34{ int v; N_list_length__int_34* next; }; int list_length__int_34(N_list_length__int_34* h){ int c=0; while(h){ c++; h=h->next; } return c; }
void to_upper__char_34(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
long gcd__long_34(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_34(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
