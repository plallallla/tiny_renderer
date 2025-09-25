#include "BaseShader.h"
#include "Rasterize.hpp"
#include "model.hpp"

class ShadowMapShader : public BaseShader
{
    Model _m;
    std::vector<double> shadowmap;

public:

    ShadowMapShader(const Model& m) : _m(m)
    {
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        return Perspective * ModelView * _m.vert(face, vert);
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const
    {
        return {false, {}};     
    }

};