// ember-gen seed=25 — intent-labeled corpus program (function name prefix = intent)
void bubble_sort__long_25(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__unsigned_25(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned sum_to_n__unsigned_25(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
int sum_to_n__int_25(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
int is_power_of_two__unsignedlonglong_25(unsigned long long v){ return v && !(v & (v-1)); }
void to_upper__char_25(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void selection_sort__long_25(long* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; long t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__double_25(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
int abs_val__int_25(int v){ return v<0 ? -v : v; }
double abs_val__double_25(double v){ return v<0 ? -v : v; }
void matrix_transpose__double_25(double* d, const double* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__int_25(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
int clamp__int_25(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
void reverse_array__int_25(int* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ int t=a[i]; a[i]=a[j]; a[j]=t; } }
float find_max__float_25(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
float find_min__float_25(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
double find_min__double_25(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned long reverse_bits__unsignedlong_25(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
unsigned reverse_bits__unsigned_25(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
int count_trailing_zeros__unsignedlong_25(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
int count_trailing_zeros__unsigned_25(unsigned v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
float dot_product__float_25(const float* a, const float* b, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int count_occurrences__unsigned_25(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__long_25(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned fnv_hash__char_25(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
