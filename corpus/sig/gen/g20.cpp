// ember-gen seed=20 — intent-labeled corpus program (function name prefix = intent)
long factorial__long_20(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
unsigned factorial__unsigned_20(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
double abs_val__double_20(double v){ return v<0 ? -v : v; }
float find_max__float_20(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int find_max__int_20(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int parity__unsignedlonglong_20(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int matrix_trace__int_20(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int fibonacci__int_20(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
void insertion_sort__long_20(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int count_occurrences__unsigned_20(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__long_20(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
struct N_list_length__long_20{ long v; N_list_length__long_20* next; }; int list_length__long_20(N_list_length__long_20* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__int_20{ int v; N_list_length__int_20* next; }; int list_length__int_20(N_list_length__int_20* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int linear_search__int_20(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
long clamp__long_20(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
double clamp__double_20(double v, double lo, double hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned fnv_hash__char_20(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void matrix_transpose__double_20(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int collatz_steps__long_20(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__int_20(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned long long reverse_bits__unsignedlonglong_20(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_20(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned isqrt__unsigned_20(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned sum_to_n__unsigned_20(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_20(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
unsigned djb2_hash__char_20(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
float mean__float_20(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_20(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
int palindrome_check__char_20(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
float distance2__float_20(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
