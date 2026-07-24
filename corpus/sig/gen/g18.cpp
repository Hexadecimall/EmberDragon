// ember-gen seed=18 — intent-labeled corpus program (function name prefix = intent)
long find_min__long_18(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned find_min__unsigned_18(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int strlen2__char_18(const char* s){ int i=0; while(s[i]) i++; return i; }
int count_trailing_zeros__unsigned_18(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int collatz_steps__int_18(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned power__unsigned_18(unsigned base, int e){ unsigned r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned djb2_hash__char_18(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
long matrix_trace__long_18(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
unsigned matrix_trace__unsigned_18(const unsigned* m, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int is_prime__int_18(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__long_18(long v){ if(v<2) return 0; for(long i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
void to_upper__char_18(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned long long reverse_bits__unsignedlonglong_18(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_18(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int palindrome_check__char_18(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
unsigned checksum_sum__char_18(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
