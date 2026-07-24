// ember-gen seed=12 — intent-labeled corpus program (function name prefix = intent)
int popcount__unsigned_12(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlong_12(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
void xor_cipher__char_12(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int array_sum__int_12(const int* a, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned factorial__unsigned_12(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
long factorial__long_12(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
float mean__float_12(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
void bubble_sort__double_12(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int dot_product__int_12(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
void to_upper__char_12(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned char crc8__char_12(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int palindrome_check__char_12(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
unsigned isqrt__unsigned_12(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned checksum_sum__char_12(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
float matrix_trace__float_12(const float* m, int n){ float s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int count_trailing_zeros__unsignedlonglong_12(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsignedlong_12(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int is_power_of_two__unsignedlonglong_12(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlong_12(unsigned long v){ return v && !(v & (v-1)); }
int linear_search__int_12(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
unsigned djb2_hash__char_12(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
