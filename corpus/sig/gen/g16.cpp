// ember-gen seed=16 — intent-labeled corpus program (function name prefix = intent)
void to_upper__char_16(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned char crc8__char_16(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
unsigned long long reverse_bits__unsignedlonglong_16(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int count_occurrences__int_16(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__unsigned_16(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
long isqrt__long_16(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int strlen2__char_16(const char* s){ int i=0; while(s[i]) i++; return i; }
void bubble_sort__unsigned_16(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
double distance2__double_16(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
float distance2__float_16(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
unsigned abs_val__unsigned_16(unsigned v){ return v<0 ? -v : v; }
int collatz_steps__long_16(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__int_16(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void matrix_transpose__double_16(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__int_16(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
struct M_list_sum__int_16{ int v; M_list_sum__int_16* next; }; int list_sum__int_16(M_list_sum__int_16* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__long_16{ long v; M_list_sum__long_16* next; }; long list_sum__long_16(M_list_sum__long_16* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
int factorial__int_16(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
long factorial__long_16(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
int palindrome_check__char_16(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
struct N_list_length__long_16{ long v; N_list_length__long_16* next; }; int list_length__long_16(N_list_length__long_16* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__int_16{ int v; N_list_length__int_16* next; }; int list_length__int_16(N_list_length__int_16* h){ int c=0; while(h){ c++; h=h->next; } return c; }
