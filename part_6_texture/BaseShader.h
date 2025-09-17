#pragma once
#include "geometry.hpp"
#include "tgaimage.hpp"

// 着色器基类 完成顶点着色与片元着色
struct BaseShader
{
    virtual vec4 vertex(const int face, const int vert) = 0;
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};