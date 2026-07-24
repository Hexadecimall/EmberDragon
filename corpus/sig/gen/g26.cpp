// ember-gen seed=26 — intent-labeled corpus program (function name prefix = intent)
int power__int_26(int base, int e){ int r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
int sum_to_n__int_26(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
void reverse_array__float_26(float* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ float t=a[i]; a[i]=a[j]; a[j]=t; } }
void reverse_array__unsigned_26(unsigned* a, int n){ for(int i=0,j=n-1;i<j;i++,j--){ unsigned t=a[i]; a[i]=a[j]; a[j]=t; } }
float distance2__float_26(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
struct N_list_length__long_26{ long v; N_list_length__long_26* next; }; int list_length__long_26(N_list_length__long_26* h){ int c=0; while(h){ c++; h=h->next; } return c; }
int quick_partition__unsigned_26(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
unsigned gcd__unsigned_26(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
void to_upper__char_26(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
void selection_sort__unsigned_26(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__float_26(float* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; float t=a[i]; a[i]=a[m]; a[m]=t; } }
unsigned fnv_hash__char_26(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
long fibonacci__long_26(int k){ long a=0,b=1; for(int i=0;i<k;i++){ long t=a+b; a=b; b=t; } return a; }
int parity__unsignedlonglong_26(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
unsigned checksum_sum__char_26(const unsigned char* d, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=d[i]; return s; }
int find_min__int_26(const int* a, int n){ int m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int dot_product__int_26(const int* a, const int* b, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
int palindrome_check__char_26(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
float array_sum__float_26(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
unsigned find_max__unsigned_26(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
unsigned max_subarray__unsigned_26(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
void bubble_sort__int_26(int* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ int t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
void bubble_sort__float_26(float* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ float t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
int count_trailing_zeros__unsignedlong_26(unsigned long v){ if(!v) return 32; int c=0; while(!(v&1)){ v>>=1; c++; } return c; }
