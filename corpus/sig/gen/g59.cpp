// ember-gen seed=59 — intent-labeled corpus program (function name prefix = intent)
int binary_search__unsigned_59(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int binary_search__int_59(const int* a, int n, int key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int palindrome_check__char_59(const char* s, int n){ for(int i=0,j=n-1;i<j;i++,j--) if(s[i]!=s[j]) return 0; return 1; }
void insertion_sort__int_59(int* a, int n){ for(int i=1;i<n;i++){ int k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
void insertion_sort__unsigned_59(unsigned* a, int n){ for(int i=1;i<n;i++){ unsigned k=a[i]; int j=i-1; while(j>=0&&a[j]>k){ a[j+1]=a[j]; j--; } a[j+1]=k; } }
int gcd__int_59(int a, int b){ while(b){ int t=b; b=a%b; a=t; } return a; }
unsigned gcd__unsigned_59(unsigned a, unsigned b){ while(b){ unsigned t=b; b=a%b; a=t; } return a; }
int is_power_of_two__unsignedlonglong_59(unsigned long long v){ return v && !(v & (v-1)); }
double clamp__double_59(double v, double lo, double hi){ return v<lo ? lo : (v>hi ? hi : v); }
void bubble_sort__unsigned_59(unsigned* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ unsigned t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned djb2_hash__char_59(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
double mean__double_59(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
struct N_list_length__long_59{ long v; N_list_length__long_59* next; }; int list_length__long_59(N_list_length__long_59* h){ int c=0; while(h){ c++; h=h->next; } return c; }
void matrix_transpose__int_59(int* d, const int* s, int n){ for(int i=0;i<n;i++) for(int j=0;j<n;j++) d[j*n+i]=s[i*n+j]; }
void xor_cipher__char_59(char* s, int n, char k){ for(int i=0;i<n;i++) s[i]^=k; }
unsigned char crc8__char_59(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
