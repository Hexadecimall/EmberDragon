// ember-gen seed=53 — intent-labeled corpus program (function name prefix = intent)
long sum_to_n__long_53(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int palindrome_check__char_53(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
unsigned fnv_hash__char_53(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void to_upper__char_53(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void xor_cipher__char_53(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
unsigned dot_product__unsigned_53(const unsigned* a, const unsigned* b, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
long dot_product__long_53(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
unsigned checksum_sum__char_53(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
void insertion_sort__float_53(float* a, int n){ for(int i=1;i<n;i++){ float k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_53(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void selection_sort__int_53(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
unsigned fibonacci__unsigned_53(int k){ unsigned a=0,b=1; for(int i=0;i<k;i++){ unsigned t=a+b; a=b; b=t; } return a; }
long fibonacci__long_53(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
unsigned array_sum__unsigned_53(const unsigned* a, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
long matrix_trace__long_53(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int linear_search__int_53(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int gcd__int_53(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
double distance2__double_53(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
float distance2__float_53(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
struct N_list_length__int_53{ int v; N_list_length__int_53* next; }; int list_length__int_53(N_list_length__int_53* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__long_53{ long v; N_list_length__long_53* next; }; int list_length__long_53(N_list_length__long_53* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int binary_search__long_53(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_53(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int count_occurrences__int_53(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int is_prime__unsigned_53(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__long_53(long v){ if(v<2) return 0; for(long i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
