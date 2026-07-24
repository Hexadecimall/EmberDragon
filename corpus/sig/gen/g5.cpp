// ember-gen seed=5 — intent-labeled corpus program (function name prefix = intent)
float abs_val__float_5(float v){ return v<0 ? -v : v; }
long array_sum__long_5(const long* a, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
float array_sum__float_5(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
void xor_cipher__char_5(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
void insertion_sort__double_5(double* a, int n){ for(int i=1;i<n;i++){ double k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int palindrome_check__char_5(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
int collatz_steps__int_5(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int quick_partition__int_5(int* a, int lo, int hi){ int p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; int t=a[i]; a[i]=a[j]; a[j]=t; } int t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int fibonacci__int_5(int k){ int a=0,b=1; for(int i=0;i<k;i++){ int t=a+b; a=b; b=t; } return a; }
unsigned find_min__unsigned_5(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
long dot_product__long_5(const long* a, const long* b, int n){ long s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
double dot_product__double_5(const double* a, const double* b, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
void to_upper__char_5(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void reverse_array__unsigned_5(unsigned* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ unsigned t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__int_5(int* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ int t=a[i]; a[i]=a[j]; a[j]=t; } }
float clamp__float_5(float v, float lo, float hi){ return v<lo ? lo : (v>hi ? hi : v); }
long clamp__long_5(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
long power__long_5(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int popcount__unsignedlong_5(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
struct M_list_sum__int_5{ int v; M_list_sum__int_5* next; }; int list_sum__int_5(M_list_sum__int_5* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__long_5{ long v; M_list_sum__long_5* next; }; long list_sum__long_5(M_list_sum__long_5* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
unsigned long reverse_bits__unsignedlong_5(unsigned long v){ unsigned long r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void matrix_transpose__unsigned_5(unsigned* d, const unsigned* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void matrix_transpose__float_5(float* d, const float* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void bubble_sort__long_5(long* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ long t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_5(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
