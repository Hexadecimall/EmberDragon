// ember-gen seed=21 — intent-labeled corpus program (function name prefix = intent)
int count_occurrences__unsigned_21(const unsigned* a, int n, unsigned key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
int count_occurrences__long_21(const long* a, int n, long key){ int c=0; for(int i=0;i<n;i++) if(a[i]==key) c++; return c; }
unsigned char crc8__char_21(const unsigned char* d, int n){ unsigned char c=0; for(int i=0;i<n;i++){ c^=d[i]; for(int b=0;b<8;b++) c = (c&0x80)? (c<<1)^0x07 : c<<1; } return c; }
unsigned reverse_bits__unsigned_21(unsigned v){ unsigned r=0; for(int i=0;i<32;i++){ r=(r<<1)|(v&1); v>>=1; } return r; }
void to_upper__char_21(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
unsigned sum_to_n__unsigned_21(unsigned k){ unsigned s=0; for(unsigned i=1;i<=k;i++) s+=i; return s; }
long sum_to_n__long_21(long k){ long s=0; for(long i=1;i<=k;i++) s+=i; return s; }
double distance2__double_21(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
unsigned fnv_hash__char_21(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
int is_prime__int_21(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__unsigned_21(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
struct M_list_sum__int_21{ int v; M_list_sum__int_21* next; }; int list_sum__int_21(M_list_sum__int_21* h){ int s=0; while(h){ s+=h->v; h=h->next; } return s; }
int collatz_steps__long_21(long v){ int s=0; while(v!=1){ v = (v&1)? 3*v+1 : v/2; s++; } return s; }
void bubble_sort__double_21(double* a, int n){ for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++) if(a[j]>a[j+1]){ double t=a[j]; a[j]=a[j+1]; a[j+1]=t; } }
unsigned djb2_hash__char_21(const char* s){ unsigned h=5381; while(*s){ h=((h<<5)+h)+(unsigned char)*s++; } return h; }
