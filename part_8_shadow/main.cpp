#include "BaseShader.h"
#include "BlankShader.hpp"
#include "Rasterize.hpp"
#include "TangentShader.hpp"
#include "ShadowMapShader.hpp"
#include "ShadowShader.hpp"

extern mat<4, 4> ModelView, Viewport, Perspective; // "OpenGL" state matrices
extern std::vector<double> zbuffer;                // the depth buffer

extern std::vector<double> shadowmap;


mat<4, 4> lookat(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 n = normalized(eye - center);
    vec3 l = normalized(cross(up, n));
    vec3 m = normalized(cross(n, l));
    return mat<4, 4>{{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0, 0, 0, 1}}} *
           mat<4, 4>{{{1, 0, 0, -center.x}, {0, 1, 0, -center.y}, {0, 0, 1, -center.z}, {0, 0, 0, 1}}};
}

mat<4, 4> init_perspective(const double f)
{
    return {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

mat<4, 4> init_viewport(const int x, const int y, const int w, const int h)
{
    return {{{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

std::vector<double> init_zbuffer(const int width, const int height)
{
    return std::vector<double>(width * height, -1000.);
}

void tangent_render(const Model& model, TGAImage& framebuffer)
{
    TangentShader shader(model);
    for (int f = 0; f < model.nfaces(); f++)
    {
        Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2)};
        rasterize(clip, shader, framebuffer);
    }
}

void blank_render(const Model& model, TGAImage& framebuffer)
{
    BlankShader shader(model);
    for (int f = 0; f < model.nfaces(); f++)
    {
        Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2)};
        rasterize(clip, shader, framebuffer);
    }
}

void save_zbuffer(std::string filename, std::vector<double>& zbuffer, int width, int height)
{
    TGAImage zimg(width, height, TGAImage::GRAYSCALE, {0, 0, 0, 0});
    double minz = +1000;
    double maxz = -1000;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            double z = zbuffer[x + y * width];
            if (z < -100)
                continue;
            minz = std::min(z, minz);
            maxz = std::max(z, maxz);
        }
    }
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            double z = zbuffer[x + y * width];
            if (z < -100)
                continue;
            z = (z - minz) / (maxz - minz) * 255;
            zimg.set(x, y, {(std::uint8_t)z, 255, 255, 255});
        }
    }
    zimg.write_tga_file(filename);
}

void ShadowMap_render(const Model& model, TGAImage& framebuffer)
{
    ShadowMapShader shader(model);
    for (int f = 0; f < model.nfaces(); f++)
    {
        Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2)};
        rasterize(clip, shader, framebuffer);
    }
}

void Shadow_render(const Model& model, const std::vector<double>& shadowmap, const mat<4, 4>& N, TGAImage& framebuffer)
{
    ShadowShader shader{model, shadowmap, N};
    for (int f = 0; f < model.nfaces(); f++)
    {
        Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1), shader.vertex(f, 2)};
        rasterize(clip, shader, framebuffer);
    }
    return;
}

int main()
{

    constexpr int width = 800;
    constexpr int height = 800;

    constexpr int shadow_w = 800; // shadow map buffer size
    constexpr int shadow_h = 800;

    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{0, 0, 0};
    constexpr vec3 up{0, 1, 0};

    // constexpr vec3 light{-1, 1, 1};
    constexpr vec3 light{1, 1, 1};

    std::vector<bool> mask(width * height, false);

    Model diablo{"../obj/diablo3_pose/diablo3_pose.obj"};
    Model floor{"../obj/floor.obj"};

    // // step1 从光源视角渲染获得shadowmap
    // TGAImage mapbuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    // Perspective = init_perspective(norm(eye - center));
    // Viewport = init_viewport(shadow_w / 16, shadow_h / 16, shadow_w * 7 / 8, shadow_h * 7 / 8);
    // zbuffer = init_zbuffer(shadow_w, shadow_h);
    // ModelView = lookat(light, center, up);


    // ShadowMap_render(diablo, mapbuffer);
    // ShadowMap_render(floor, mapbuffer);
    // auto lig_zbuffer = zbuffer;
    // auto ligMatrix = Perspective * ModelView;

    // save_zbuffer("zbuffer_1.tga", lig_zbuffer, width, height);
    // system("open zbuffer_1.tga");

    // // step2 从相机视角出发渲染进行真正的渲染
    // ModelView = lookat(eye, center, up);
    // zbuffer = init_zbuffer(shadow_w, shadow_h);
    // TGAImage shadowbuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    // Shadow_render(diablo, lig_zbuffer, ligMatrix, shadowbuffer);
    // Shadow_render(floor, lig_zbuffer, ligMatrix, shadowbuffer);
    // shadowbuffer.write_tga_file("shadowbuffer.tga");
    // system("open shadowbuffer.tga");

    // save_zbuffer("zbuffer_2.tga", shadowmap, width, height);
    // system("open zbuffer_2.tga");

    // orign---20250925
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    ModelView = lookat(eye, center, up);
    Perspective = init_perspective(norm(eye - center));
    Viewport = init_viewport(shadow_w / 16, shadow_h / 16, shadow_w * 7 / 8, shadow_h * 7 / 8);
    zbuffer = init_zbuffer(shadow_w, shadow_h);
    tangent_render(diablo, framebuffer);
    tangent_render(floor, framebuffer);
    framebuffer.write_tga_file("framebuffer.tga");
    // system("open framebuffer.tga");

    mat<4, 4> M = (Viewport * Perspective * ModelView).invert();

    ModelView = lookat(light, center, up);
    Perspective = init_perspective(norm(eye - center));
    Viewport = init_viewport(shadow_w / 16, shadow_h / 16, shadow_w * 7 / 8, shadow_h * 7 / 8);
    auto zbuffer_copy = zbuffer;
    save_zbuffer("zbuffer_1.tga", zbuffer, width, height);
    // system("open zbuffer_1.tga");
    zbuffer = init_zbuffer(shadow_w, shadow_h);

    {
        TGAImage trash(shadow_w, shadow_h, TGAImage::RGB, {177, 195, 209, 255});
        blank_render(diablo, trash);
        blank_render(floor, trash);
        trash.write_tga_file("trash.tga");
        // system("open trash.tga");
    }
    save_zbuffer("zbuffer_2.tga", zbuffer, width, height);
    // system("open zbuffer_2.tga");

    mat<4, 4> N = Viewport * Perspective * ModelView; // 基于light视角的顶点着色矩阵

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            vec4 fragment = M * vec4{(double)x, (double)y, zbuffer_copy.at(x + y * width), 1.};
            vec4 q = N * fragment;
            vec3 p = q.xyz() / q.w;
            mask[x + y * width] = (
                fragment.z < -100 || (p.x < 0 || p.x >= shadow_w || p.y < 0 || p.y > shadow_h)
                || (p.z > zbuffer[int(p.x) + int(p.y) * shadow_w] - .03));
        }
    }

    TGAImage maskimg(width, height, TGAImage::GRAYSCALE);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (!mask[x + y * height])
            {
                continue;
            }
            maskimg.set(x, y, {255, 255, 255, 255});
        }
    }
    maskimg.write_tga_file("mask.tga");
    // system("open mask.tga");

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (mask[x + y * height])
            {
                continue;
            }
            TGAColor c = framebuffer.get(x, y);
            vec3 a = {(double)c[0], (double)c[1], (double)c[2]};
            if (norm(a) < 80)
            {
                continue;
            }
            a = normalized(a) * 80;
            framebuffer.set(x, y, {(std::uint8_t)a[0], (std::uint8_t)a[1], (std::uint8_t)a[2], 255});
        }
    }
    framebuffer.write_tga_file("shadow.tga");
    system("open shadow.tga");

    return 0;
}