#include "BaseShader.h"
#include "geometry.hpp"
#include "model.hpp"
#include "Rasterize.hpp"
#include "tgaimage.hpp"

class TextureShader : public BaseShader
{
 const Model &model;
    vec4 l{ModelView*vec4{0,0,1,0.}};              // light direction in eye coordinates
    vec2 varying_uv[3];  // triangle uv coordinates, written by the vertex shader, read by the fragment shader

public:
    TextureShader(const Model &m) : model(m) {
    }

    TGAColor sample2D(const TGAImage &img, const vec2 &uv) const
    {
        return img.get(uv.x * img.width(), uv.y * img.height());
    }

    virtual vec4 vertex(const int face, const int vert) {
        varying_uv[vert] = model.uv(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        vec4 n = normalized(ModelView.invert_transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l)*2 - l);                   // reflected light direction
        double ambient  = .4;                                     // ambient light intensity
        double diffuse  = 1.*std::max(0., n * l);                 // diffuse light intensity
        double specular = (3.*sample2D(model.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);  // specular intensity, note that the camera lies on the z-axis (in eye coordinates), therefore simple r.z, since (0,0,1)*(r.x, r.y, r.z) = r.z
        TGAColor gl_FragColor = sample2D(model.diffuse(), uv);
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        return {false, gl_FragColor};                             // do not discard the pixel
    }

};