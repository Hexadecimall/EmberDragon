// ember-gen seed=37 — intent-labeled corpus program (function name prefix = intent)
double distance2__double_37(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
int binary_search__unsigned_37(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__long_37(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
double clamp__double_37(double v, double lo, double hi){ return v<lo ? lo : (v>hi ? hi : v); }
int clamp__int_37(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned fnv_hash__char_37(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void selection_sort__int_37(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
double dot_product__double_37(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int strlen2__char_37(const char* s){ int i=0; while(s[i]) i++; return i; }
void xor_cipher__char_37(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
unsigned find_max__unsigned_37(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
long find_max__long_37(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
int palindrome_check__char_37(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
unsigned long long reverse_bits__unsignedlonglong_37(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned char crc8__char_37(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
unsigned power__unsigned_37(unsigned base, int e){ unsigned r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int power__int_37(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned matrix_trace__unsigned_37(const unsigned* m, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
long matrix_trace__long_37(const long* m, int n){ long s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int sum_to_n__int_37(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
unsigned sum_to_n__unsigned_37(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
unsigned isqrt__unsigned_37(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int isqrt__int_37(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int linear_search__float_37(const float* a, int n, float key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__int_37(const int* a, int n, int key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
void insertion_sort__unsigned_37(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void matrix_transpose__int_37(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void reverse_array__long_37(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__unsigned_37(unsigned* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ unsigned t=a[i]; a[i]=a[j]; a[j]=t; } }
