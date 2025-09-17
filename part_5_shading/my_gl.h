#pragma once
#include "geometry.hpp"
#include "tgaimage.hpp"
#include <array>

struct IShader
{
    virtual vec4 vertex(const int face, const int vert) = 0;
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

using Triangle = std::array<vec4, 3>; // a triangle primitive is made of three ordered points
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);