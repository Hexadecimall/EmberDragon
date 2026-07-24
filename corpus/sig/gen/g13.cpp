// ember-gen seed=13 — intent-labeled corpus program (function name prefix = intent)
unsigned long reverse_bits__unsignedlong_13(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int is_prime__int_13(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
long isqrt__long_13(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_13(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
long factorial__long_13(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
long clamp__long_13(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
float clamp__float_13(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned checksum_sum__char_13(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
unsigned gcd__unsigned_13(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
long gcd__long_13(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
int linear_search__unsigned_13(const unsigned* a, int n, unsigned key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
unsigned sum_to_n__unsigned_13(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_13(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
unsigned char crc8__char_13(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
void insertion_sort__double_13(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_13(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void xor_cipher__char_13(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
struct N_list_length__long_13{ long v; N_list_length__long_13* next; }; int list_length__long_13(N_list_length__long_13* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int count_occurrences__long_13(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__unsigned_13(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
