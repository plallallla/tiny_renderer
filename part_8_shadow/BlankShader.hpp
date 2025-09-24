#include "BaseShader.h"
#include "Rasterize.hpp"
#include "model.hpp"

struct BlankShader : BaseShader
{
    const Model& model;

    BlankShader(const Model& m) : model(m)
    {
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        return  Perspective * ModelView * model.vert(face, vert);
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
    {
        return {false, {255, 255, 255, 255}};
    }
};