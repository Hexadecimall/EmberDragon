// ember-gen seed=27 — intent-labeled corpus program (function name prefix = intent)
void to_upper__char_27(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int popcount__unsignedlonglong_27(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsigned_27(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
unsigned long reverse_bits__unsignedlong_27(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned reverse_bits__unsigned_27(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
long power__long_27(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int power__int_27(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int linear_search__double_27(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__long_27(const long* a, int n, long key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void matrix_transpose__int_27(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
unsigned abs_val__unsigned_27(unsigned v){ return v<0 ? -v : v; }
int abs_val__int_27(int v){ return v<0 ? -v : v; }
struct N_list_length__int_27{ int v; N_list_length__int_27* next; }; int list_length__int_27(N_list_length__int_27* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int count_trailing_zeros__unsignedlonglong_27(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsigned_27(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
struct M_list_sum__int_27{ int v; M_list_sum__int_27* next; }; int list_sum__int_27(M_list_sum__int_27* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__long_27{ long v; M_list_sum__long_27* next; }; long list_sum__long_27(M_list_sum__long_27* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
float dot_product__float_27(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int count_occurrences__long_27(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned char crc8__char_27(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
double distance2__double_27(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
float distance2__float_27(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
int binary_search__int_27(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__long_27(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
unsigned find_min__unsigned_27(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int palindrome_check__char_27(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int collatz_steps__unsigned_27(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__int_27(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void selection_sort__long_27(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
int parity__unsignedlonglong_27(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
