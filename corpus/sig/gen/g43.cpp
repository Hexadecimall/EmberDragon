// ember-gen seed=43 — intent-labeled corpus program (function name prefix = intent)
unsigned long long reverse_bits__unsignedlonglong_43(unsigned long long v){ unsigned long long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned long reverse_bits__unsignedlong_43(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int factorial__int_43(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
unsigned factorial__unsigned_43(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int palindrome_check__char_43(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
long dot_product__long_43(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int dot_product__int_43(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
double find_max__double_43(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
long clamp__long_43(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
float clamp__float_43(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned djb2_hash__char_43(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
void insertion_sort__int_43(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__double_43(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
unsigned fnv_hash__char_43(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
void reverse_array__long_43(long* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ long t=a[i]; a[i]=a[j]; a[j]=t; } }
