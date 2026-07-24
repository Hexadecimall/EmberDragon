// ember-gen seed=0 — intent-labeled corpus program (function name prefix = intent)
int count_occurrences__unsigned_0(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__int_0(const int* a, int n, int key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
void selection_sort__int_0(int* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; int t=a[i]; a[i]=a[m]; a[m]=t; } }
void selection_sort__double_0(double* a, int n){ for(int i=0;i<n-1;i++){ int m=i; for(int j=i+1;j<n;j++) if(a[j]<a[m]) m=j; double t=a[i]; a[i]=a[m]; a[m]=t; } }
float abs_val__float_0(float v){ return v<0 ? -v : v; }
int abs_val__int_0(int v){ return v<0 ? -v : v; }
unsigned fnv_hash__char_0(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
unsigned djb2_hash__char_0(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
long max_subarray__long_0(const long* a, int n){ long best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
unsigned max_subarray__unsigned_0(const unsigned* a, int n){ unsigned best=a[0], cur=a[0]; for(int i=1;i<n;i++){ cur = a[i] > cur+a[i] ? a[i] : cur+a[i]; if(cur>best) best=cur; } return best; }
int is_power_of_two__unsignedlonglong_0(unsigned long long v){ return v && !(v & (v-1)); }
void to_upper__char_0(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
int array_sum__int_0(const int* a, int n){ int s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
float array_sum__float_0(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
int sum_to_n__int_0(int k){ int s=0; for(int i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_0(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
int gcd__int_0(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
int factorial__int_0(int k){ int r=1; for(int i=2;i<=k;i++) r*=i; return r; }
int linear_search__double_0(const double* a, int n, double key){ for(int i=0;i<n;i++) if(a[i]==key) return i; return -1; }
float mean__float_0(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_0(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
int quick_partition__unsigned_0(unsigned* a, int lo, int hi){ unsigned p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; unsigned t=a[i]; a[i]=a[j]; a[j]=t; } unsigned t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
int quick_partition__long_0(long* a, int lo, int hi){ long p=a[hi]; int i=lo-1; for(int j=lo;j<hi;j++) if(a[j]<p){ i++; long t=a[i]; a[i]=a[j]; a[j]=t; } long t=a[i+1]; a[i+1]=a[hi]; a[hi]=t; return i+1; }
struct M_list_sum__long_0{ long v; M_list_sum__long_0* next; }; long list_sum__long_0(M_list_sum__long_0* h){ long s=0; while(h){ s+=h->v; h=h->next; } return s; }
