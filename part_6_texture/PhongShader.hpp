#include "BaseShader.h"
#include "model.hpp"
#include "rasterize.hpp"
#include "tgaimage.hpp"

struct PhongShader : public BaseShader
{
    Model model;
    PhongShader(const Model& m) : model(m) 
    {
    }
    virtual vec4 vertex(const int face, const int vert) override
    {
        auto v = model.vert(face, vert);
        return Perspective * ModelView * v;
    }
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const override
    {
        return {false,{}};
    }
};