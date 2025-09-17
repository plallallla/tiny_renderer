#include <cstdlib>
#include "my_gl.h"
#include "model.hpp"

extern mat<4,4> ModelView, Viewport, Perspective; // "OpenGL" state matrices
extern std::vector<double> zbuffer;     // the depth buffer


mat<4,4> lookat(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    return mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

mat<4,4> init_perspective(const double f)
{
    return {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

mat<4,4> init_viewport(const int x, const int y, const int w, const int h)
{
    return {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

std::vector<double> init_zbuffer(const int width, const int height)
{
    return std::vector<double>(width*height, -1000.);
}

struct RandomShader : IShader {
    const Model &model;
    TGAColor color = {};
    vec3 tri[3];  // triangle in eye coordinates

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position.xyz();                         // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, color};                                    // do not discard the pixel
    }
};

struct PhongShader : IShader {
    const Model &model;
    vec3 l;          // light direction in eye coordinates
    vec3 tri[3];     // triangle in eye coordinates

    PhongShader(const vec3 light, const Model &m) : model(m) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}).xyz()); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec4 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();                            // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        TGAColor gl_FragColor = {255, 255, 255, 255};             // output color of the fragment
        vec3 n = normalized(cross(tri[1]-tri[0], tri[2]-tri[0])); // triangle normal in eye coordinates
        vec3 r = normalized(n * (n * l)*2 - l);                   // reflected light direction
        double ambient = .3;                                      // ambient light intensity
        double diff = std::max(0., n * l);                        // diffuse light intensity
        double spec = std::pow(std::max(r.z, 0.), 35);            // specular intensity, note that the camera lies on the z-axis (in eye coordinates), therefore simple r.z, since (0,0,1)*(r.x, r.y, r.z) = r.z
        for (int channel : {0,1,2})
            gl_FragColor[channel] *= std::min(1., ambient + .4*diff + .9*spec);
        return {false, gl_FragColor};                             // do not discard the pixel
    }
};

int main(int argc, char** argv) {

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    constexpr vec3  light{ 1, 1, 1}; // light source


    ModelView = lookat(eye, center, up);                                   // build the ModelView   matrix
    Perspective = init_perspective(norm(eye-center));                        // build the Perspective matrix
    Viewport = init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    zbuffer = init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    // Model model{"../obj/diablo3_pose/diablo3_pose.obj"};// load the data
    // RandomShader shader(model);
    Model model("../obj/african_head/african_head.obj");
    PhongShader shader{light,model};
    for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
        TGAColor rnd;
        // for (int c=0; c<3; c++)
        // {
        //     shader.color[c] = std::rand() % 255;
        // }
        Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                          shader.vertex(f, 1),
                          shader.vertex(f, 2) };
        rasterize(clip, shader, framebuffer);   // rasterize the primitive
    }

    framebuffer.write_tga_file("framebuffer.tga");
    system("open framebuffer.tga");
    return 0;
}