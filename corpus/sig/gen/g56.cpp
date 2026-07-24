// ember-gen seed=56 — intent-labeled corpus program (function name prefix = intent)
void bubble_sort__int_56(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void to_upper__char_56(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void xor_cipher__char_56(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int is_power_of_two__unsigned_56(unsigned v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlonglong_56(unsigned long long v){ return v && !(v & (v-1)); }
struct N_list_length__int_56{ int v; N_list_length__int_56* next; }; int list_length__int_56(N_list_length__int_56* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int count_occurrences__long_56(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned isqrt__unsigned_56(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long find_max__long_56(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int find_max__int_56(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
double dot_product__double_56(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int linear_search__long_56(const long* a, int n, long key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int count_trailing_zeros__unsigned_56(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int parity__unsignedlonglong_56(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsignedlong_56(unsigned long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
unsigned gcd__unsigned_56(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
unsigned long long reverse_bits__unsignedlonglong_56(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void matrix_transpose__int_56(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__double_56(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
unsigned char crc8__char_56(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
double mean__double_56(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
float mean__float_56(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
int strlen2__char_56(const char* s){ int i=0; while(s[i]) i++; return i; }
