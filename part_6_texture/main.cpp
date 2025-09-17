#include "PhongShader.hpp"


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

int main()
{

    constexpr int width  = 800;
    constexpr int height = 800;
    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{ 0, 0, 0};
    constexpr vec3 up{ 0, 1, 0};

    constexpr vec3 light{ 1, 1, 1};

    ModelView = lookat(eye, center, up);
    Perspective = init_perspective(norm(eye-center));
    Viewport = init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

    zbuffer = init_zbuffer(width, height);

    Model model("../obj/african_head/african_head.obj");
    PhongShader shader{ model };

    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    for (int f=0; f<model.nfaces(); f++) 
    {
        Triangle clip = { shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2) };
        rasterize(clip, shader, framebuffer);
    }
    
    framebuffer.write_tga_file("framebuffer.tga");
    system("open framebuffer.tga");

    return 0;
}