#include "mapping.h"
#include <cassert>
#include <limits>
using namespace scale_contract;
void close(double a,double b) { assert(std::abs(a-b)<1e-9); }
int main() {
    for(double s:{1.,1.25,1.5,2.}) {
        Mapping m{800,600,int(800*s),int(600*s),s,s,1,0};
        auto p=m.documentToBuffer({10,20});close(p.x,34*s);close(p.y,44*s);
        auto q=m.bufferToDocument({34*s,44*s});close(q.x,10);close(q.y,20);
        close(m.layoutWidth(),752);close(m.logicalHeight(),600);
        m.zoom=2;m.scroll=-10;
        p=m.documentToBuffer({10,20});close(p.x,44*s);close(p.y,54*s);
        q=m.windowToDocument({44,54});close(q.x,10);close(q.y,20);
    }
    Mapping rounded{801,601,1001,751,1.25,1.25,1,0};
    close(rounded.documentToBuffer({777,577}).x,1001);
    close(rounded.documentToBuffer({777,577}).y,751);
    State state;assert(state.update(rounded));assert(!state.update(rounded));
    rounded.displayScale=1.5;assert(state.update(rounded)); // unchanged buffer
    close(rounded.logicalWidth(),667.5);
    rounded.bufferWidth=1202;assert(state.update(rounded));
    rounded.zoom=2;assert(state.update(rounded));
    rounded.zoom=1;assert(state.update(rounded));assert(state.generation==5);
    rounded.density=std::numeric_limits<double>::quiet_NaN();
    try {state.update(rounded);assert(false);}catch(const std::runtime_error&){}
    assert(state.generation==5);
}
