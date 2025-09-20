#include "BaseShader.h"
#include "Rasterize.hpp"
#include "model.hpp"

class TangentShader : public BaseShader
{
    Model _m;
    vec4 _l{ModelView*vec4{0,0,1,0.}};              // light direction in eye coordinates
    vec2 varying_uv[3]; // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    vec4 varying_nrm[3]; // normal per vertex to be interpolated by the fragment shader
    vec4 tri[3];         // triangle in view coordinates

public:
    TangentShader(const Model& m) : _m(m)
    {
    }

    TGAColor sample2D(const TGAImage &img, const vec2 &uv) const
    {
        return img.get(uv.x * img.width(), uv.y * img.height());
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        varying_uv[vert] = _m.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * _m.normal(face, vert);
        vec4 gl_Position = ModelView * _m.vert(face, vert);
        tri[vert] = gl_Position;
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const
    {
        mat<2,4> E = { tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U = { varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        mat<4,4> D = {normalized(T[0]),  // tangent vector
                      normalized(T[1]),  // bitangent vector
                      normalized(varying_nrm[0]*bc[0] + varying_nrm[1]*bc[1] + varying_nrm[2]*bc[2]), // interpolated normal
                      {0,0,0,1}}; // Darboux frame
        vec2 uv = varying_uv[0] * bc[0] + varying_uv[1] * bc[1] + varying_uv[2] * bc[2];
        vec4 n = normalized(D.transpose() * _m.tangent_normal(uv));
        vec4 r = normalized(n * (n * _l)*2 - _l);                   // reflected light direction
        double ambient  = .4;                                     // ambient light intensity
        double diffuse  = 1.*std::max(0., n * _l);                 // diffuse light intensity
        double specular = (3.*sample2D(_m.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);  // specular intensity, note that the camera lies on the z-axis (in eye coordinates), therefore simple r.z, since (0,0,1)*(r.x, r.y, r.z) = r.z
        TGAColor gl_FragColor = sample2D(_m.diffuse(), uv);
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        return {false, gl_FragColor};     
    }


};