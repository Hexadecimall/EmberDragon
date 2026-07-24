// ember-gen seed=28 — intent-labeled corpus program (function name prefix = intent)
int gcd__int_28(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_28(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
struct N_list_length__int_28{ int v; N_list_length__int_28* next; }; int list_length__int_28(N_list_length__int_28* h){ int c=0; while(h){ c++; h=h->next; } return c; }
double distance2__double_28(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
int is_prime__int_28(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
long isqrt__long_28(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned char crc8__char_28(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
int palindrome_check__char_28(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int count_occurrences__unsigned_28(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int sum_to_n__int_28(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
unsigned sum_to_n__unsigned_28(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
void xor_cipher__char_28(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int factorial__int_28(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
long factorial__long_28(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
