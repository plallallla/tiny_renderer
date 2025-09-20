#include "BaseShader.h"
#include "geometry.hpp"
#include "model.hpp"
#include "rasterize.hpp"
#include "tgaimage.hpp"
#include <array>

class TextureShader : public BaseShader
{
    Model _m;
    vec4 _l{1,1,1,0};
    std::array<vec2, 3> _uv;
public:
    TextureShader(Model m) : _m{m}
    {
    }
    TGAColor sample2D(const TGAImage &img, const vec2 &uv) const
    {
        return img.get(uv.x * img.width(), uv.y * img.height());
    }
    virtual vec4 vertex(const int face, const int vert) override
    {
        _uv[vert] = _m.uv(face, vert);
        return Perspective * ModelView * _m.vert(face, vert);
    }
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const override
    {
        vec2 uv = _uv[0] * bar[0] + _uv[1] * bar[1] + _uv[2] * bar[2];
        vec4 n = normalized(ModelView.invert_transpose() * _m.normal(uv));
        vec4 v = vec4{0,0,1,0};
        vec4 h = normalized(v + _l);
        double ambient = .3;
        double diffuse = std::max(0., n * _l);
        double specular = (.5+1.2*sample2D(_m.specular(), uv)[0]/255.) * std::pow(std::max(n*h, 0.), 30);
        TGAColor gl_FragColor = sample2D(_m.diffuse(), uv);
        for (int channel : {0, 1, 2}) 
        {
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        }
        return {false, gl_FragColor};
    }

};