// ember-gen seed=24 — intent-labeled corpus program (function name prefix = intent)
double mean__double_24(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
float mean__float_24(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
void matrix_transpose__unsigned_24(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int is_prime__unsigned_24(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
long sum_to_n__long_24(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int sum_to_n__int_24(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
unsigned fibonacci__unsigned_24(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
int fibonacci__int_24(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
long power__long_24(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned checksum_sum__char_24(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
int binary_search__int_24(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__unsigned_24(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
long factorial__long_24(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
unsigned long reverse_bits__unsignedlong_24(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long long reverse_bits__unsignedlonglong_24(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void bubble_sort__int_24(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int isqrt__int_24(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_24(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
void to_upper__char_24(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void reverse_array__double_24(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
float dot_product__float_24(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int parity__unsignedlonglong_24(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsigned_24(unsigned v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int quick_partition__int_24(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
unsigned djb2_hash__char_24(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
double abs_val__double_24(double v){ return v<0 ? -v : v; }
long abs_val__long_24(long v){ return v<0 ? -v : v; }
struct N_list_length__int_24{ int v; N_list_length__int_24* next; }; int list_length__int_24(N_list_length__int_24* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__long_24{ long v; N_list_length__long_24* next; }; int list_length__long_24(N_list_length__long_24* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int collatz_steps__int_24(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_24(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
