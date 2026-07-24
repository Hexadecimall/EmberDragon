// ember-gen seed=7 — intent-labeled corpus program (function name prefix = intent)
int factorial__int_7(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
int max_subarray__int_7(const int* a, int n){ int best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_7(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int quick_partition__unsigned_7(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int linear_search__double_7(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
struct N_list_length__int_7{ int v; N_list_length__int_7* next; }; int list_length__int_7(N_list_length__int_7* h){ int c=0; while(h){ c++; h=h->next; } return c; }
struct N_list_length__long_7{ long v; N_list_length__long_7* next; }; int list_length__long_7(N_list_length__long_7* h){ int c=0; while(h){ c++; h=h->next; } return c; }
double find_max__double_7(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]>m) m=a[i]; return m; }
unsigned dot_product__unsigned_7(const unsigned* a, const unsigned* b, int n){ unsigned s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s; }
long clamp__long_7(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int clamp__int_7(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
unsigned fnv_hash__char_7(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
unsigned sum_to_n__unsigned_7(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
void selection_sort__unsigned_7(unsigned* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; unsigned t=a[i]; a[i]=a[m]; a[m]=t; } }
unsigned djb2_hash__char_7(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
void xor_cipher__char_7(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
int palindrome_check__char_7(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
void to_upper__char_7(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
