// ember-gen seed=10 — intent-labeled corpus program (function name prefix = intent)
void selection_sort__long_10(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__unsigned_10(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
void reverse_array__float_10(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__unsigned_10(unsigned* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ unsigned t=a[i]; a[i]=a[j]; a[j]=t; } }
void to_upper__char_10(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned matrix_trace__unsigned_10(const unsigned* m, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
void bubble_sort__long_10(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_10(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
long sum_to_n__long_10(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int palindrome_check__char_10(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
unsigned djb2_hash__char_10(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
int popcount__unsigned_10(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
unsigned fibonacci__unsigned_10(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
long fibonacci__long_10(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
unsigned char crc8__char_10(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
float abs_val__float_10(float v){ return v<0 ? -v : v; }
double abs_val__double_10(double v){ return v<0 ? -v : v; }
int collatz_steps__unsigned_10(unsigned v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
unsigned fnv_hash__char_10(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
struct N_list_length__long_10{ long v; N_list_length__long_10* next; }; int list_length__long_10(N_list_length__long_10* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int find_min__int_10(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float dot_product__float_10(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int is_prime__unsigned_10(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__int_10(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
void insertion_sort__long_10(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
