// ember-gen seed=17 — intent-labeled corpus program (function name prefix = intent)
int count_occurrences__int_17(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__unsigned_17(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int is_power_of_two__unsigned_17(unsigned v){ return v && !(v & (v-1)); }
double dot_product__double_17(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
unsigned long reverse_bits__unsignedlong_17(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int is_prime__unsigned_17(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
struct N_list_length__long_17{ long v; N_list_length__long_17* next; }; int list_length__long_17(N_list_length__long_17* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int popcount__unsigned_17(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlong_17(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
float find_min__float_17(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
void insertion_sort__int_17(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__float_17(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int collatz_steps__unsigned_17(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__int_17(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
float mean__float_17(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_17(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
unsigned char crc8__char_17(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
long clamp__long_17(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int clamp__int_17(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
int sum_to_n__int_17(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
int gcd__int_17(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
long gcd__long_17(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
unsigned abs_val__unsigned_17(unsigned v){ return v<0 ? -v : v; }
unsigned max_subarray__unsigned_17(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
long fibonacci__long_17(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
