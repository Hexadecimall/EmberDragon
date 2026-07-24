// ember-gen seed=44 — intent-labeled corpus program (function name prefix = intent)
void xor_cipher__char_44(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
struct N_list_length__long_44{ long v; N_list_length__long_44* next; }; int list_length__long_44(N_list_length__long_44* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__int_44{ int v; N_list_length__int_44* next; }; int list_length__int_44(N_list_length__int_44* h){ int c=0; while(h){ c++; h=h->next; } return c; }
unsigned find_min__unsigned_44(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int is_prime__unsigned_44(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__int_44(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
float mean__float_44(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
long isqrt__long_44(long v){ long x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned reverse_bits__unsigned_44(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_44(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void insertion_sort__unsigned_44(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int matrix_trace__int_44(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
float matrix_trace__float_44(const float* m, int n){ float s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
long clamp__long_44(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
double clamp__double_44(double v, double lo, double hi){ return v<lo ? lo : (v>hi ? hi : v); }
void bubble_sort__unsigned_44(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int quick_partition__int_44(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int binary_search__unsigned_44(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
void matrix_transpose__int_44(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__unsigned_44(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
float abs_val__float_44(float v){ return v<0 ? -v : v; }
double abs_val__double_44(double v){ return v<0 ? -v : v; }
long array_sum__long_44(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
