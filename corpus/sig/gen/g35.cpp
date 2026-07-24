// ember-gen seed=35 — intent-labeled corpus program (function name prefix = intent)
int count_trailing_zeros__unsigned_35(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
long gcd__long_35(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
int gcd__int_35(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
unsigned char crc8__char_35(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int factorial__int_35(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
unsigned long reverse_bits__unsignedlong_35(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void reverse_array__long_35(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
long abs_val__long_35(long v){ return v<0 ? -v : v; }
int abs_val__int_35(int v){ return v<0 ? -v : v; }
int quick_partition__long_35(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__int_35(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void xor_cipher__char_35(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int popcount__unsignedlonglong_35(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
double dot_product__double_35(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
long max_subarray__long_35(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_35(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
double array_sum__double_35(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
float array_sum__float_35(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
void matrix_transpose__unsigned_35(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
float mean__float_35(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_35(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
struct N_list_length__long_35{ long v; N_list_length__long_35* next; }; int list_length__long_35(N_list_length__long_35* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__int_35{ int v; N_list_length__int_35* next; }; int list_length__int_35(N_list_length__int_35* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int is_prime__long_35(long v){ if(v<2) return 0; for(long i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
void bubble_sort__long_35(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_35(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
