#pragma once
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace scale_contract {
struct Point { double x, y; };
struct Mapping {
    int windowWidth=800, windowHeight=600, bufferWidth=800, bufferHeight=600;
    double density=1, displayScale=1, zoom=1, scroll=0;
    bool operator==(const Mapping&) const = default;
    void validate() const {
        if(windowWidth<=0 || windowHeight<=0 || bufferWidth<=0 || bufferHeight<=0
           || !std::isfinite(density) || density<=0 || !std::isfinite(displayScale)
           || displayScale<=0 || !std::isfinite(zoom) || zoom<=0 || !std::isfinite(scroll))
            throw std::runtime_error("invalid scale mapping");
    }
    double contentScale() const { return displayScale/density; }
    double logicalWidth() const { return windowWidth/contentScale(); }
    double logicalHeight() const { return windowHeight/contentScale(); }
    double layoutWidth() const { return (logicalWidth()-48)/zoom; }
    Point surfaceToBuffer(Point p) const {
        return {p.x*bufferWidth/windowWidth,p.y*bufferHeight/windowHeight};
    }
    Point documentToWindow(Point p) const {
        return {(24+p.x*zoom)*contentScale(),(24+scroll+p.y*zoom)*contentScale()};
    }
    Point windowToDocument(Point p) const {
        return {(p.x/contentScale()-24)/zoom,(p.y/contentScale()-24-scroll)/zoom};
    }
    Point documentToBuffer(Point p) const { return surfaceToBuffer(documentToWindow(p)); }
    Point bufferToDocument(Point p) const {
        return windowToDocument({p.x*windowWidth/bufferWidth,p.y*windowHeight/bufferHeight});
    }
};
struct State {
    Mapping mapping;
    uint64_t generation=0;
    bool update(const Mapping& next) {
        next.validate();
        if(generation && next==mapping)return false;
        mapping=next;++generation;return true;
    }
};
}
