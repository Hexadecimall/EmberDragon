// ember-gen seed=55 — intent-labeled corpus program (function name prefix = intent)
long power__long_55(long base, int e){ long r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned power__unsigned_55(unsigned base, int e){ unsigned r=1; while(e>0){ if(e&1) r*=base; base*=base; e>>=1; } return r; }
unsigned factorial__unsigned_55(unsigned k){ unsigned r=1; for(unsigned i=2;i<=k;i++) r*=i; return r; }
int is_power_of_two__unsigned_55(unsigned v){ return v && !(v & (v-1)); }
int is_power_of_two__unsignedlonglong_55(unsigned long long v){ return v && !(v & (v-1)); }
int binary_search__unsigned_55(const unsigned* a, int n, unsigned key){ int lo=0, hi=n-1; while(lo<=hi){ int m=(lo+hi)/2; if(a[m]==key) return m; if(a[m]<key) lo=m+1; else hi=m-1; } return -1; }
int is_prime__int_55(int v){ if(v<2) return 0; for(int i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
int is_prime__unsigned_55(unsigned v){ if(v<2) return 0; for(unsigned i=2;i*i<=v;i++) if(v%i==0) return 0; return 1; }
long clamp__long_55(long v, long lo, long hi){ return v<lo ? lo : (v>hi ? hi : v); }
int clamp__int_55(int v, int lo, int hi){ return v<lo ? lo : (v>hi ? hi : v); }
float distance2__float_55(float x1, float y1, float x2, float y2){ float dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
double distance2__double_55(double x1, double y1, double x2, double y2){ double dx=x2-x1, dy=y2-y1; return dx*dx+dy*dy; }
float array_sum__float_55(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s; }
void to_upper__char_55(char* s){ for(int i=0;s[i];i++) if(s[i]>='a'&&s[i]<='z') s[i]-=32; }
float mean__float_55(const float* a, int n){ float s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
double mean__double_55(const double* a, int n){ double s=0; for(int i=0;i<n;i++) s+=a[i]; return s/n; }
unsigned fnv_hash__char_55(const char* s){ unsigned h=2166136261u; while(*s){ h=(h^(unsigned char)*s++)*16777619u; } return h; }
