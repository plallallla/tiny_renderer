#include <cstdlib>
#include "my_gl.h"

#include "PhoneShader.hpp"

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

int main(int argc, char** argv) {

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    constexpr vec3  light{ 1, 1, 1}; // light source

    ModelView = lookat(eye, center, up);// 构建MV矩阵
    Perspective = init_perspective(norm(eye-center));//构建透视矩阵
    Viewport = init_viewport(width/16, height/16, width*7/8, height*7/8);// 构建视窗矩阵
    zbuffer = init_zbuffer(width, height);// 深度测试缓存
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});// 绘制缓存

    // Model model{"../obj/diablo3_pose/diablo3_pose.obj"};// load the data
    // RandomShader shader(model);
    Model model("../obj/african_head/african_head.obj");
    PhongShader shader{light,model};
    for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets

        // assemble the primitive 将顶点组装成图元
        Triangle clip = { shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2) };

        // rasterize the primitive 将图元进行光栅化
        // TGAColor rnd;
        // for (int c=0; c<3; c++)
        // {
        //     shader.color[c] = std::rand() % 255;
        // }
        rasterize(clip, shader, framebuffer);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    system("open framebuffer.tga");
    return 0;
}