#include "BaseShader.h"
#include "Rasterize.hpp"
#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"
extern std::array<int, 2> screen_xy;

std::vector<double> shadowmap=std::vector<double>(800 * 800, -1000.);

class ShadowShader : public BaseShader
{
    Model _m;
    vec4 _l{ModelView*vec4{1,1,1,0.}};              // light direction in eye coordinates
    vec2 varying_uv[3]; // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    vec4 varying_nrm[3]; // normal per vertex to be interpolated by the fragment shader
    vec4 tri[3];         // triangle in view coordinates
    mat<4, 4> _N;   //perspective * MV_light
    vec4 lig[3];    //光源视角下的空间坐标
    std::vector<double> _shadowmap;

public:
    ShadowShader(const Model& m, const std::vector<double>& shadowmap, const mat<4, 4>& N) :
    _m{m}, _shadowmap{shadowmap}, _N{N}
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
        lig[vert] = _N * _m.vert(face, vert);
        return Perspective * ModelView * _m.vert(face, vert);
    }

    TGAColor phongColor(const vec3 bc) const
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
        {
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        }
        return gl_FragColor;
    }

    TGAColor shadowColor(const vec3 bc, TGAColor origin) const
    {
        vec2 screen[3] = 
        {
            (Viewport * lig[0]).xy(), 
            (Viewport * lig[1]).xy(), 
            (Viewport * lig[2]).xy()
        };
        int x = (int)(bc*vec3{screen[0].x,screen[1].x,screen[2].x});
        int y = (int)(bc*vec3{screen[0].y,screen[1].y,screen[2].y});
        double z_real = bc * vec3{lig[0].z,lig[1].z,lig[2].z};
        if ((0 <= x && x < 800 && 0<= y && y < 800) && z_real < _shadowmap[x + y*800] - .03)
        {
            vec3 a = {(double)origin[0], (double)origin[1], (double)origin[2]};
            if (norm(a) >= 80)
            {
                a = normalized(a) * 80;
                // return {(std::uint8_t)a[0], (std::uint8_t)a[1], (std::uint8_t)a[2], 255};
                return {0,0,0,0};
            }
        }
        return {255,255,255,0};
        return origin;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const
    {
        return {false, shadowColor(bc, phongColor(bc))};     
    }


};