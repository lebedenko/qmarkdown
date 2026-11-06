#include "ScanBound.h"
#include <iostream>
#include <vector>
#include <random>
#include <cstring>
#include <stdexcept>
struct Entry { float trimmed,lower; };
static size_t tested=0;
static void check(bool yes) { if(!yes)throw std::runtime_error("numeric counterexample"); }
// Endpoint selection including legal preference, first indivisible and emergency.
static std::pair<size_t,bool> select(const std::vector<Entry>& e,const std::vector<float>& pad,
        unsigned eligible,unsigned legal,float width,bool bound) {
    size_t fallback=0,preferred=0,first=0;
    for(size_t i=0;i<e.size();++i)if((eligible>>i)&1){first=i+1;break;}
    for(size_t i=0;i<e.size();++i) {
        if(bound && scan_bound::skip(e[i].lower,width)) {
            for(size_t j=i;j<e.size();++j)check(!(e[j].trimmed+pad[j]<=width));
            break;
        }
        if(e[i].trimmed+pad[i]<=width && ((eligible>>i)&1)) {
            fallback=i+1;if((legal>>i)&1)preferred=i+1;
        }
    }
    return {preferred?preferred:fallback?fallback:first?first:e.size(),!preferred};
}
static void run(const std::vector<float>& widths,unsigned whitespace,unsigned eligible,unsigned legal,
                const std::vector<float>& pad,float available) {
    std::vector<Entry> e;float sum=0;bool safe=scan_bound::platform<float>()==scan_bound::safe && std::isfinite(available);
    for(size_t i=0;i<widths.size();++i) {
        safe &= std::isfinite(widths[i]) && std::isfinite(pad[i]) && pad[i]>=0;
        sum+=widths[i];float trimmed=sum;safe &= std::isfinite(sum);
        for(size_t t=i+1;t && ((whitespace>>(t-1))&1);) {--t;trimmed-=widths[t];safe &= std::isfinite(trimmed);}
        safe &= std::isfinite(trimmed+pad[i]);e.push_back({trimmed,trimmed});
    }
    if(safe)scan_bound::suffix(e);
    check(select(e,pad,eligible,legal,available,false)==select(e,pad,eligible,legal,available,safe));
    ++tested;
}
int main() {
    try {
        check(scan_bound::platform<float>()==scan_bound::safe);
        check(scan_bound::platform<double>()==scan_bound::representation);
        check(!scan_bound::skip(1.f,1.f));
        check(scan_bound::skip(std::nextafter(1.f,2.f),1.f));
        // All short widths, whitespace, eligibility AND legal endpoint patterns.
        // Equality and both adjacent representable values at every distinct cost.
        for(unsigned n=1;n<=4;++n)for(unsigned code=0;code<(1u<<(2*n));++code) {
            std::vector<float>w(n),p(n);unsigned v=code;
            for(auto& x:w){x=float(int(v&3)-1);v>>=2;}
            for(unsigned ws=0;ws<(1u<<n);++ws)for(unsigned pattern=0;pattern<3;++pattern) {
                for(unsigned i=0;i<n;++i)p[i]=pattern==0?0:pattern==1?2:float((i%2)?0:4);
                std::vector<float> boundaries={-1,0,1,2,4};float sum=0;
                for(unsigned i=0;i<n;++i){sum+=w[i];float t=sum;for(unsigned j=i+1;j && ((ws>>(j-1))&1);){--j;t-=w[j];}boundaries.push_back(t+p[i]);}
                std::sort(boundaries.begin(),boundaries.end());boundaries.erase(std::unique(boundaries.begin(),boundaries.end()),boundaries.end());
                for(unsigned eligibility=0;eligibility<(1u<<n);++eligibility)
                    for(unsigned legal=0;legal<(1u<<n);++legal)for(float x:boundaries)
                        for(float a:{std::nextafter(x,-INFINITY),x,std::nextafter(x,INFINITY)})
                            run(w,ws,eligibility,legal,p,a);
            }
        }
        // Non-monotonic padded costs: overflow then lower padding can fit.
        run({1,1,1},0,7,7,{8,0,0},3);
        // Trimming can reveal a lower suffix, including signed/zero widths.
        run({2,-1,0,1},6,15,5,{4,0,2,0},2);
        auto exhaustive=tested;
        std::mt19937 random(73074);
        for(unsigned k=0;k<10000;++k) {
            unsigned n=1+random()%4;std::vector<float>w(n),p(n);
            auto value=[&](){uint32_t bits=random();float x;std::memcpy(&x,&bits,4);return x;};
            for(unsigned i=0;i<n;++i){w[i]=value();p[i]=std::abs(value());}
            float a=value();
            if(k%17==0)p[0]=-1;
            if(k%19==0)w[0]=INFINITY;
            if(k%23==0)p[0]=NAN;
            int old=std::fegetround();
            if(k%29==0){std::fesetround(FE_DOWNWARD);check(scan_bound::platform<float>()==scan_bound::environment);}
            run(w,random()%16,random()%16,random()%16,p,a);std::fesetround(old);
        }
        auto csr=_mm_getcsr();_mm_setcsr(csr|(1u<<15));check(scan_bound::platform<float>()==scan_bound::environment);
        _mm_setcsr((csr & ~(3u<<13)) | (1u<<13));check(scan_bound::platform<float>()==scan_bound::environment);
        _mm_setcsr(csr|(1u<<6));check(scan_bound::platform<float>()==scan_bound::environment);_mm_setcsr(csr);
        std::cout<<"exhaustive="<<exhaustive<<" numeric="<<tested-exhaustive<<" seed=73074 passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
