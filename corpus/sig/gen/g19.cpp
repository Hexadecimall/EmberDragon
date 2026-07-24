// ember-gen seed=19 — intent-labeled corpus program (function name prefix = intent)
void selection_sort__double_19(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
void xor_cipher__char_19(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int find_min__int_19(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned fnv_hash__char_19(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int power__int_19(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int max_subarray__int_19(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_19(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
float array_sum__float_19(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
double array_sum__double_19(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned checksum_sum__char_19(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
unsigned reverse_bits__unsigned_19(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long long reverse_bits__unsignedlonglong_19(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
long factorial__long_19(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
long abs_val__long_19(long v){ return v<0 ? -v : v; }
int quick_partition__unsigned_19(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int gcd__int_19(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
long gcd__long_19(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
long sum_to_n__long_19(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
unsigned fibonacci__unsigned_19(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
int fibonacci__int_19(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
int palindrome_check__char_19(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
void to_upper__char_19(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
float mean__float_19(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
void reverse_array__int_19(int* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ int t=a[i]; a[i]=a[j]; a[j]=t; } }
double distance2__double_19(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
