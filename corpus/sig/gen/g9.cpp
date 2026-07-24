// ember-gen seed=9 — intent-labeled corpus program (function name prefix = intent)
long clamp__long_9(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
float clamp__float_9(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
long dot_product__long_9(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
double dot_product__double_9(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int popcount__unsigned_9(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int gcd__int_9(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
int is_prime__unsigned_9(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
void bubble_sort__int_9(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__unsigned_9(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int count_trailing_zeros__unsignedlonglong_9(unsigned long long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
unsigned fnv_hash__char_9(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int palindrome_check__char_9(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int binary_search__long_9(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
struct M_list_sum__long_9{ long v; M_list_sum__long_9* next; }; long list_sum__long_9(M_list_sum__long_9* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__int_9{ int v; M_list_sum__int_9* next; }; int list_sum__int_9(M_list_sum__int_9* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
unsigned char crc8__char_9(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
void xor_cipher__char_9(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int is_power_of_two__unsignedlong_9(unsigned long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsigned_9(unsigned v){ return v && !(v & (v-1)); }
long array_sum__long_9(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
void insertion_sort__int_9(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__double_9(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
unsigned checksum_sum__char_9(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
