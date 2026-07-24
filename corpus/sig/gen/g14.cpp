// ember-gen seed=14 — intent-labeled corpus program (function name prefix = intent)
int clamp__int_14(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned clamp__unsigned_14(unsigned v, unsigned lo, unsigned hi){ return v<lo ? lo : (v>hi ? hi : v); }
void xor_cipher__char_14(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int collatz_steps__long_14(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int collatz_steps__unsigned_14(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int popcount__unsigned_14(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
long abs_val__long_14(long v){ return v<0 ? -v : v; }
int abs_val__int_14(int v){ return v<0 ? -v : v; }
unsigned reverse_bits__unsigned_14(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int linear_search__int_14(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__double_14(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int strlen2__char_14(const char* s){ int i=0; while(s[i]) i++; return i; }
int is_power_of_two__unsignedlong_14(unsigned long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlonglong_14(unsigned long long v){ return v && !(v & (v-1)); }
int palindrome_check__char_14(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int max_subarray__int_14(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_14(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
