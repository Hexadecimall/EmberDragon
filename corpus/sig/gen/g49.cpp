// ember-gen seed=49 — intent-labeled corpus program (function name prefix = intent)
double array_sum__double_49(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int count_occurrences__unsigned_49(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
double find_min__double_49(const double* a, int n){ double m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
unsigned find_min__unsigned_49(const unsigned* a, int n){ unsigned m=a[0]; for(int i=1;i<n;i++) if(a[i]<m) m=a[i]; return m; }
int parity__unsigned_49(unsigned v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
int parity__unsignedlonglong_49(unsigned long long v){ int p=0; while(v){ p^=1; v&=v-1; } return p; }
struct M_list_sum__int_49{ int v; M_list_sum__int_49* next; }; int list_sum__int_49(M_list_sum__int_49* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
unsigned fnv_hash__char_49(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int quick_partition__long_49(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__unsigned_49(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
void selection_sort__double_49(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__int_49(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
int popcount__unsignedlonglong_49(unsigned long long v){ int c=0; while(v){ v&=v-1; c++; } return c; }
int popcount__unsigned_49(unsigned v){ int c=0; while(v){ v&=v-1; c++; } return c; }
unsigned factorial__unsigned_49(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int palindrome_check__char_49(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
