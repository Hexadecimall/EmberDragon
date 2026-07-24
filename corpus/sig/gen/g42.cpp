// ember-gen seed=42 — intent-labeled corpus program (function name prefix = intent)
long find_min__long_42(const long* a, int n){ long m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
float find_min__float_42(const float* a, int n){ float m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
void insertion_sort__int_42(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__long_42(long* a, int n){ for(int i=1;i<n;i++){ long k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int popcount__unsignedlong_42(unsigned long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsignedlonglong_42(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int collatz_steps__int_42(int v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
int isqrt__int_42(int v){ int x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
unsigned isqrt__unsigned_42(unsigned v){ unsigned x=v, y=(x+1)/2; while(y<x){ x=y; y=(x+v/x)/2; } return x; }
int gcd__int_42(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
long gcd__long_42(long a, long b){ while(b){ long t=b; b=a%b; a=t; } return a; }
double find_max__double_42(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
unsigned find_max__unsigned_42(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
struct N_list_length__long_42{ long v; N_list_length__long_42* next; }; int list_length__long_42(N_list_length__long_42* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int binary_search__long_42(const long* a, int n, long key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
void reverse_array__double_42(double* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ double t=a[i]; a[i]=a[j]; a[j]=t; } }
void selection_sort__double_42(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__unsigned_42(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
void bubble_sort__int_42(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned djb2_hash__char_42(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
struct M_list_sum__int_42{ int v; M_list_sum__int_42* next; }; int list_sum__int_42(M_list_sum__int_42* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
struct M_list_sum__long_42{ long v; M_list_sum__long_42* next; }; long list_sum__long_42(M_list_sum__long_42* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
unsigned char crc8__char_42(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
float abs_val__float_42(float v){ return v<0 ? -v : v; }
unsigned abs_val__unsigned_42(unsigned v){ return v<0 ? -v : v; }
int is_power_of_two__unsignedlong_42(unsigned long v){ return v && !(v & (v-1)); }
void to_upper__char_42(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int clamp__int_42(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
long clamp__long_42(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int strlen2__char_42(const char* s){ int i=0; while(s[i]) i++; return i; }
