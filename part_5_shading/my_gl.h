#pragma once
#include "tgaimage.hpp"
#include "geometry.hpp"
#include <array>

struct IShader {
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

typedef std::array<vec4, 3> Triangle; // a triangle primitive is made of three ordered points
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);