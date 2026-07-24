// ember-brain — EmberDragon's own neural net. From scratch in C++: no PyTorch,
// no BLAS, no deps. v1: a single-layer LSTM character language model (gated
// memory: forget/input/output gates + cell state), trained with Adam and BPTT,
// all forward/backward hand-written. Trains on a raw code corpus (ember-harvest)
// or the named+commented targets in corpus.jsonl.
//
//   build:  clang++ -std=c++17 -O3 ember-brain.cpp -o ember-brain
//   train:  ember-brain train corpus/code.txt [iters] [out.bin]   (raw text)
//           ember-brain train corpus/corpus.jsonl [iters]         (jsonl targets)
//   sample: ember-brain sample corpus/ember-brain.bin [nchars]
//   prompt: ember-brain prompt corpus/ember-brain.bin "int add(" [n]   (prime + continue)
//   chat:   ember-brain chat   corpus/ember-brain.bin                  (interactive REPL)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
using std::string; using std::vector; using std::unordered_map;

static int H=256, V=0, SEQ=64;
static double rnd(){ return ((double)rand()/RAND_MAX*2-1); }
static double sigm(double x){ return 1.0/(1.0+exp(-x)); }

// a trainable parameter block: weights + grad + Adam moments
struct P { vector<double> w,g,m,v; void init(int n,double s){ w.resize(n); for(auto&x:w)x=rnd()*s; g.assign(n,0);m.assign(n,0);v.assign(n,0);} void zero(){ std::fill(g.begin(),g.end(),0.0);} };
static P Wf,Wi,Wc,Wo, Uf,Ui,Uc,Uo, bf,bi,bc,bo, Why,by;   // LSTM + output head
static long adamT=0;
static void adam(P&p,double lr){
    double b1=0.9,b2=0.999,e=1e-8, c1=1-pow(b1,adamT), c2=1-pow(b2,adamT);
    for(size_t i=0;i<p.w.size();i++){ double gi=p.g[i]; if(gi>5)gi=5; if(gi<-5)gi=-5;
        p.m[i]=b1*p.m[i]+(1-b1)*gi; p.v[i]=b2*p.v[i]+(1-b2)*gi*gi;
        p.w[i]-=lr*(p.m[i]/c1)/(sqrt(p.v[i]/c2)+e); }
}
static void initModel(){
    double s=1.0/sqrt((double)H);
    Wf.init(H*V,s);Wi.init(H*V,s);Wc.init(H*V,s);Wo.init(H*V,s);
    Uf.init(H*H,s);Ui.init(H*H,s);Uc.init(H*H,s);Uo.init(H*H,s);
    bf.init(H,0);bi.init(H,0);bc.init(H,0);bo.init(H,0); for(auto&x:bf.w)x=1.0;   // forget-bias 1: remember early
    Why.init(V*H,s); by.init(V,0);
}

// per-timestep activations kept for BPTT
struct Step { vector<double> f,i,g,o,c,h,tc; };
// forward+backward one sequence; accumulates grads, returns loss, updates h/c carry
static double run(const vector<int>& xs,const vector<int>& ts,vector<double>& h0,vector<double>& c0,bool train,double& outLoss){
    int T=(int)xs.size();
    vector<Step> S(T); vector<vector<double>> ps(T,vector<double>(V));
    vector<double> hp=h0, cp=c0; double loss=0;
    for(int t=0;t<T;t++){ int x=xs[t]; Step& st=S[t];
        st.f.resize(H);st.i.resize(H);st.g.resize(H);st.o.resize(H);st.c.resize(H);st.h.resize(H);st.tc.resize(H);
        for(int k=0;k<H;k++){
            double af=bf.w[k]+Wf.w[k*V+x], ai=bi.w[k]+Wi.w[k*V+x], ag=bc.w[k]+Wc.w[k*V+x], ao=bo.w[k]+Wo.w[k*V+x];
            const double *uf=&Uf.w[k*H],*ui=&Ui.w[k*H],*uc=&Uc.w[k*H],*uo=&Uo.w[k*H];
            for(int j=0;j<H;j++){ double hj=hp[j]; af+=uf[j]*hj; ai+=ui[j]*hj; ag+=uc[j]*hj; ao+=uo[j]*hj; }
            st.f[k]=sigm(af); st.i[k]=sigm(ai); st.g[k]=tanh(ag); st.o[k]=sigm(ao);
            st.c[k]=st.f[k]*cp[k]+st.i[k]*st.g[k]; st.tc[k]=tanh(st.c[k]); st.h[k]=st.o[k]*st.tc[k];
        }
        double mx=-1e30; for(int q=0;q<V;q++){ double y=by.w[q]; const double* w=&Why.w[q*H]; for(int k=0;k<H;k++) y+=w[k]*st.h[k]; ps[t][q]=y; if(y>mx)mx=y; }
        double sum=0; for(int q=0;q<V;q++){ ps[t][q]=exp(ps[t][q]-mx); sum+=ps[t][q]; }
        for(int q=0;q<V;q++) ps[t][q]/=sum;
        loss += -log(ps[t][ts[t]]+1e-12);
        hp=st.h; cp=st.c;
    }
    outLoss=loss;
    if(!train){ h0=hp; c0=cp; return loss; }
    vector<double> dhn(H,0), dcn(H,0);
    for(int t=T-1;t>=0;t--){ Step& st=S[t]; int x=xs[t];
        const vector<double>& hpprev = (t? S[t-1].h : h0);
        const vector<double>& cpprev = (t? S[t-1].c : c0);
        vector<double> dy=ps[t]; dy[ts[t]]-=1.0;
        for(int q=0;q<V;q++){ double d=dy[q]; double* w=&Why.g[q*H]; const double* hh=st.h.data(); for(int k=0;k<H;k++) w[k]+=d*hh[k]; by.g[q]+=d; }
        vector<double> dh(H);
        for(int k=0;k<H;k++){ double s=dhn[k]; for(int q=0;q<V;q++) s+=Why.w[q*H+k]*dy[q]; dh[k]=s; }
        vector<double> dc(H), df(H),di(H),dg(H),doo(H);
        for(int k=0;k<H;k++){
            double dch = dh[k]*st.o[k]*(1-st.tc[k]*st.tc[k]) + dcn[k];
            doo[k]=dh[k]*st.tc[k]*st.o[k]*(1-st.o[k]);
            df[k]=dch*cpprev[k]*st.f[k]*(1-st.f[k]);
            di[k]=dch*st.g[k]*st.i[k]*(1-st.i[k]);
            dg[k]=dch*st.i[k]*(1-st.g[k]*st.g[k]);
            dc[k]=dch*st.f[k];   // -> dcn for previous step
        }
        for(int k=0;k<H;k++){
            bf.g[k]+=df[k]; bi.g[k]+=di[k]; bc.g[k]+=dg[k]; bo.g[k]+=doo[k];
            Wf.g[k*V+x]+=df[k]; Wi.g[k*V+x]+=di[k]; Wc.g[k*V+x]+=dg[k]; Wo.g[k*V+x]+=doo[k];
            double *gf=&Uf.g[k*H],*gi=&Ui.g[k*H],*gc=&Uc.g[k*H],*go=&Uo.g[k*H];
            double a=df[k],b=di[k],cc=dg[k],d=doo[k];
            for(int j=0;j<H;j++){ double hj=hpprev[j]; gf[j]+=a*hj; gi[j]+=b*hj; gc[j]+=cc*hj; go[j]+=d*hj; }
        }
        vector<double> dhp(H,0);
        for(int j=0;j<H;j++){ double s=0; for(int k=0;k<H;k++) s+=Uf.w[k*H+j]*df[k]+Ui.w[k*H+j]*di[k]+Uc.w[k*H+j]*dg[k]+Uo.w[k*H+j]*doo[k]; dhp[j]=s; }
        dhn=dhp; dcn=dc;
    }
    h0=S[T-1].h; c0=S[T-1].c;
    return loss;
}

static string g_idx2ch; static unordered_map<char,int> g_ch2idx;
// advance the LSTM one char (used by sampling/priming); updates h,c; returns logits->probs
static void fwd1(int x, vector<double>& h, vector<double>& c, vector<double>* probs){
    vector<double> nh(H), nc(H);
    for(int k=0;k<H;k++){ double af=bf.w[k]+Wf.w[k*V+x],ai=bi.w[k]+Wi.w[k*V+x],ag=bc.w[k]+Wc.w[k*V+x],ao=bo.w[k]+Wo.w[k*V+x];
        const double *uf=&Uf.w[k*H],*ui=&Ui.w[k*H],*uc=&Uc.w[k*H],*uo=&Uo.w[k*H];
        for(int j=0;j<H;j++){ double hj=h[j]; af+=uf[j]*hj;ai+=ui[j]*hj;ag+=uc[j]*hj;ao+=uo[j]*hj; }
        double F=sigm(af),I=sigm(ai),G=tanh(ag),O=sigm(ao); nc[k]=F*c[k]+I*G; nh[k]=O*tanh(nc[k]); }
    h=nh; c=nc;
    if(probs){ probs->resize(V); double mx=-1e30; for(int q=0;q<V;q++){ double y=by.w[q]; const double* w=&Why.w[q*H]; for(int k=0;k<H;k++) y+=w[k]*h[k]; (*probs)[q]=y; if(y>mx)mx=y; } double s=0; for(int q=0;q<V;q++){ (*probs)[q]=exp((*probs)[q]-mx); s+=(*probs)[q]; } for(int q=0;q<V;q++) (*probs)[q]/=s; }
}
static string generate(const string& seed,int n,double temp=0.85){
    vector<double> h(H,0), c(H,0); int x = g_ch2idx.count('\n')?g_ch2idx['\n']:0;
    for(char ch:seed){ if(g_ch2idx.count(ch)){ fwd1(x,h,c,nullptr); x=g_ch2idx[ch]; } }
    string out;
    for(int s=0;s<n;s++){ vector<double> p; fwd1(x,h,c,&p);
        double sum=0; for(double& pr:p){ pr=pow(pr,1.0/temp); sum+=pr; }
        double r=((double)rand()/RAND_MAX)*sum, acc=0; int k=0; for(;k<V;k++){ acc+=p[k]; if(acc>=r) break; } if(k>=V)k=V-1;
        out+=g_idx2ch[k]; x=k; }
    return out;
}

static const uint32_t MAGIC=0xE3B7A101;   // bump on format change
static void save(const char* path){ FILE* f=fopen(path,"wb"); if(!f){fprintf(stderr,"save failed\n");return;}
    uint32_t mg=MAGIC; fwrite(&mg,4,1,f); fwrite(&H,4,1,f); fwrite(&V,4,1,f); fwrite(g_idx2ch.data(),1,V,f);
    auto w=[&](P&p){ fwrite(p.w.data(),sizeof(double),p.w.size(),f); };
    w(Wf);w(Wi);w(Wc);w(Wo);w(Uf);w(Ui);w(Uc);w(Uo);w(bf);w(bi);w(bc);w(bo);w(Why);w(by); fclose(f);
    fprintf(stderr,"ember-brain: saved -> %s (LSTM H=%d vocab=%d)\n",path,H,V); }
static bool load(const char* path){ FILE* f=fopen(path,"rb"); if(!f) return false;
    uint32_t mg=0; if(fread(&mg,4,1,f)!=1||mg!=MAGIC){ fclose(f); fprintf(stderr,"ember-brain: %s is not a current-format model (re-train)\n",path); return false; }
    if(fread(&H,4,1,f)!=1||fread(&V,4,1,f)!=1){fclose(f);return false;}
    g_idx2ch.resize(V); if(fread(&g_idx2ch[0],1,V,f)!=(size_t)V){fclose(f);return false;}
    for(int i=0;i<V;i++) g_ch2idx[g_idx2ch[i]]=i;
    auto al=[&](P&p,int n){ p.w.resize(n); fread(p.w.data(),sizeof(double),n,f); };
    al(Wf,H*V);al(Wi,H*V);al(Wc,H*V);al(Wo,H*V);al(Uf,H*H);al(Ui,H*H);al(Uc,H*H);al(Uo,H*H);
    al(bf,H);al(bi,H);al(bc,H);al(bo,H);al(Why,V*H);al(by,V); fclose(f); return true; }

static string unesc(const char* p,const char* end){ string s; while(p<end&&*p!='"'){ if(*p=='\\'&&p+1<end){p++; switch(*p){case 'n':s+='\n';break;case 't':s+='\t';break;case 'r':s+='\r';break;case '"':s+='"';break;case '\\':s+='\\';break;case '/':s+='/';break;default:s+=*p;} p++;} else s+=*p++; } return s; }
static string loadData(const char* path){
    string p=path; size_t n=p.size();
    FILE* f=fopen(path,"rb"); if(!f){ fprintf(stderr,"cannot open %s\n",path); exit(1); }
    if(n>6 && p.substr(n-6)==".jsonl"){
        string text; char* line=nullptr; size_t cap=0; ssize_t r; int progs=0;
        while((r=getline(&line,&cap,f))>0){ const char* k=strstr(line,"\"target\":"); if(!k)continue; const char* q=strchr(k+9,'"'); if(!q)continue; string t=unesc(q+1,line+r); if(t.size()<8)continue; text+=t; text+="\n\n"; progs++; }
        free(line); fclose(f); fprintf(stderr,"ember-brain: %d programs, %zu chars\n",progs,text.size()); return text;
    }
    fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET); string text(sz,0); fread(&text[0],1,sz,f); fclose(f);
    fprintf(stderr,"ember-brain: %ld chars of raw code\n",sz); return text;
}

int main(int argc,char** argv){ srand(1234);
    string cmd = argc>=2?argv[1]:"";
    if(cmd=="train"){
        const char* data=argc>=3?argv[2]:"corpus/code.txt"; long iters=argc>=4?atol(argv[3]):60000;
        const char* outbin=argc>=5?argv[4]:"corpus/ember-brain.bin";
        string text=loadData(data); if(text.size()<500){fprintf(stderr,"too little data\n");return 1;}
        bool seen[256]={false}; for(unsigned char c:text) seen[c]=true; for(int c=0;c<256;c++) if(seen[c]){ g_ch2idx[(char)c]=(int)g_idx2ch.size(); g_idx2ch+=(char)c; }
        V=(int)g_idx2ch.size(); initModel();
        fprintf(stderr,"ember-brain: LSTM H=%d vocab=%d seq=%d — training %ld iters on %zuK chars\n",H,V,SEQ,iters,text.size()/1024);
        vector<double> h(H,0),c(H,0); double smooth=-log(1.0/V)*SEQ;
        for(long it=0;it<iters;it++){
            // RANDOM window across the WHOLE corpus + fresh carry per window (stateless truncated BPTT).
            // The old sequential pos + persistent-carry ground the LOW-ENTROPY header prefix in order ->
            // memorized boilerplate -> ppl collapsed toward 1 and samples just copied #ifndef/headers.
            size_t pos=(size_t)(((double)rand()/((double)RAND_MAX+1.0))*(double)(text.size()-SEQ-1));
            std::fill(h.begin(),h.end(),0.0); std::fill(c.begin(),c.end(),0.0);
            vector<int> xs(SEQ),ts(SEQ); for(int t=0;t<SEQ;t++){ xs[t]=g_ch2idx[text[pos+t]]; ts[t]=g_ch2idx[text[pos+t+1]]; }
            Wf.zero();Wi.zero();Wc.zero();Wo.zero();Uf.zero();Ui.zero();Uc.zero();Uo.zero();bf.zero();bi.zero();bc.zero();bo.zero();Why.zero();by.zero();
            double loss; run(xs,ts,h,c,true,loss); smooth=smooth*0.999+loss*0.001; adamT++;
            double lr=2e-3; adam(Wf,lr);adam(Wi,lr);adam(Wc,lr);adam(Wo,lr);adam(Uf,lr);adam(Ui,lr);adam(Uc,lr);adam(Uo,lr);
            adam(bf,lr);adam(bi,lr);adam(bc,lr);adam(bo,lr);adam(Why,lr);adam(by,lr);
            if(it%2000==0) fprintf(stderr,"  iter %6ld  loss/char %.3f  ppl %.1f\n",it,smooth/SEQ,exp(smooth/SEQ));
            if(it%15000==0 && it>0){ fprintf(stderr,"  ── sample ──\n%s\n  ────\n", generate("\n",260).c_str()); save(outbin); }   // checkpoint: survives a restart/crash
        }
        save(outbin); return 0;
    }
    if(cmd=="sample"||cmd=="prompt"||cmd=="chat"){
        const char* w=argc>=3?argv[2]:"corpus/ember-brain.bin";
        if(!load(w)){ fprintf(stderr,"no weights at %s — train first\n",w); return 1; }
        if(cmd=="sample"){ int n=argc>=4?atoi(argv[3]):600; printf("%s\n",generate("\n",n).c_str()); return 0; }
        if(cmd=="prompt"){ string seed=argc>=4?argv[3]:""; int n=argc>=5?atoi(argv[4]):400; printf("%s%s\n",seed.c_str(),generate(seed,n).c_str()); return 0; }
        // chat: prime with each line, stream a continuation
        fprintf(stderr,"🔥🐉 ember-brain chat — type code/text, it continues. Ctrl-D to quit.\n");
        char* line=nullptr; size_t cap=0; ssize_t r;
        while(fprintf(stderr,"\n> "), (r=getline(&line,&cap,stdin))>0){ string s(line, r>0?r:0); printf("%s",generate(s,300).c_str()); fflush(stdout); }
        free(line); return 0;
    }
    fprintf(stderr,"usage: ember-brain train <corpus> [iters] [out.bin] | sample <bin> [n] | prompt <bin> \"seed\" [n] | chat <bin>\n");
    return 2;
}
