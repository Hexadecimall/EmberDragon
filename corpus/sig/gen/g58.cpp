// ember-gen seed=58 — intent-labeled corpus program (function name prefix = intent)
long power__long_58(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
long sum_to_n__long_58(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
unsigned char crc8__char_58(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
void selection_sort__int_58(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
unsigned clamp__unsigned_58(unsigned v, unsigned lo, unsigned hi){ return v<lo ? lo : (v>hi ? hi : v); }
int strlen2__char_58(const char* s){ int i=0; while(s[i]) i++; return i; }
long max_subarray__long_58(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int max_subarray__int_58(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int array_sum__int_58(const int* a, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned reverse_bits__unsigned_58(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
long abs_val__long_58(long v){ return v<0 ? -v : v; }
double abs_val__double_58(double v){ return v<0 ? -v : v; }
long matrix_trace__long_58(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
struct M_list_sum__long_58{ long v; M_list_sum__long_58* next; }; long list_sum__long_58(M_list_sum__long_58* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
int is_power_of_two__unsignedlonglong_58(unsigned long long v){ return v && !(v & (v-1)); }
int is_power_of_two__unsigned_58(unsigned v){ return v && !(v & (v-1)); }
unsigned checksum_sum__char_58(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
unsigned factorial__unsigned_58(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
long factorial__long_58(long k){ long r=1; for(long i=2;i<=k;i++) r*=i; return r; }
void xor_cipher__char_58(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int quick_partition__int_58(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__long_58(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void bubble_sort__double_58(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int collatz_steps__long_58(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
