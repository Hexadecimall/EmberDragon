// ember-gen seed=1 — intent-labeled corpus program (function name prefix = intent)
float matrix_trace__float_1(const float* m, int n){ float s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int matrix_trace__int_1(const int* m, int n){ int s=0; for(int i=0;i<n;i++) s+=m[i*n+i]; return s; }
int linear_search__unsigned_1(const unsigned* a, int n, unsigned key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
int linear_search__long_1(const long* a, int n, long key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
unsigned abs_val__unsigned_1(unsigned v){ return v<0 ? -v : v; }
int find_min__int_1(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned djb2_hash__char_1(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
int strlen2__char_1(const char* s){ int i=0; while(s[i]) i++; return i; }
void to_upper__char_1(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
double mean__double_1(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
unsigned sum_to_n__unsigned_1(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
int sum_to_n__int_1(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
long find_max__long_1(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
double find_max__double_1(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
void matrix_transpose__unsigned_1(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__int_1(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void bubble_sort__double_1(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__int_1(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
