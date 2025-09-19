#include "BaseShader.h"
#include "geometry.hpp"
#include "model.hpp"
#include "rasterize.hpp"
#include "tgaimage.hpp"
#include <algorithm>
#include <array>

struct smoothShader : public BaseShader
{
    Model model;
    vec3 l{1, 1, 1};
    std::array<vec3, 3> tri;
    vec3 varying_nrm[3]; // normal per vertex to be interpolated by the fragment shader
    smoothShader(const Model& m) : model(m) 
    {
    }
    virtual vec4 vertex(const int face, const int vert) override
    {
        auto mv_ver = ModelView * model.vert(face, vert);
        tri[vert] = mv_ver.xyz();
        vec4 n = model.normal(face, vert);
        varying_nrm[vert] = (ModelView.invert_transpose() * vec4{n.x, n.y, n.z, 0.}).xyz();
        return Perspective *  mv_ver;
    }
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const override
    {
        TGAColor base_color{255, 255, 255, 255};
        // vec3 n = normalized(cross(tri[1] - tri[0], tri[2] - tri[0]));
        vec3 n = normalized(varying_nrm[0] * bar[0] +
                            varying_nrm[1] * bar[1] +
                            varying_nrm[2] * bar[2]);             // per-vertex normal interpolation
        vec3 r = normalized(n * (n * l) * 2 - l);//反射光向量
        vec3 v = vec3{0,0,1};
        vec3 h = normalized(v + l);
        double ambient = .3;
        double diff = std::max(0., n * l);
        double spec = std::pow(std::max(n*h, 0.), 35);//半程向量计算
        for (int channel : {0, 1, 2}) 
        {
            base_color[channel] *= std::min(1., ambient + .4*diff + .9*spec);
        }
        return {false, base_color};
    }
};