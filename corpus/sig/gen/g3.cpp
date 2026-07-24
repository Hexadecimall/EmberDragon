// ember-gen seed=3 — intent-labeled corpus program (function name prefix = intent)
void matrix_transpose__float_3(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__long_3(long* d, const long* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
struct N_list_length__int_3{ int v; N_list_length__int_3* next; }; int list_length__int_3(N_list_length__int_3* h){ int c=0; while(h){ c++; h=h->next; } return c; }
unsigned gcd__unsigned_3(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
int gcd__int_3(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
long dot_product__long_3(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
void to_upper__char_3(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int linear_search__unsigned_3(const unsigned* a, int n, unsigned key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void bubble_sort__double_3(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_3(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned char crc8__char_3(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
long abs_val__long_3(long v){ return v<0 ? -v : v; }
unsigned abs_val__unsigned_3(unsigned v){ return v<0 ? -v : v; }
int isqrt__int_3(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long power__long_3(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned array_sum__unsigned_3(const unsigned* a, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int collatz_steps__unsigned_3(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__long_3(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
