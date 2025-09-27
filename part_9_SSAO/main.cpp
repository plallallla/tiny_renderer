#include "BaseShader.h"
#include "BlankShader.hpp"
#include "Rasterize.hpp"
#include "TangentShader.hpp"
#include <random>

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

int main()
{

    constexpr int width = 800;
    constexpr int height = 800;

    constexpr int shadow_w = 800; // shadow map buffer size
    constexpr int shadow_h = 800;

    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{0, 0, 0};
    constexpr vec3 up{0, 1, 0};

    constexpr vec3 light{1, 1, 1};

    std::vector<bool> mask(width * height, false);

    Model diablo{"../obj/diablo3_pose/diablo3_pose.obj"};
    Model floor{"../obj/floor.obj"};

    ModelView = lookat(light, center, up);
    Perspective = init_perspective(norm(eye - center));
    Viewport = init_viewport(shadow_w / 16, shadow_h / 16, shadow_w * 7 / 8, shadow_h * 7 / 8);
    zbuffer = init_zbuffer(shadow_w, shadow_h);
    TGAImage trash(shadow_w, shadow_h, TGAImage::RGB, {177, 195, 209, 255});
    blank_render(diablo, trash);
    blank_render(floor, trash);

    mat<4, 4> N = Viewport * Perspective * ModelView;
    auto shadow_depth = zbuffer;

    ModelView = lookat(eye, center, up);
    Perspective = init_perspective(norm(eye - center));
    Viewport = init_viewport(shadow_w / 16, shadow_h / 16, shadow_w * 7 / 8, shadow_h * 7 / 8);
    mat<4, 4> M = (Viewport * Perspective * ModelView).invert();
    zbuffer = init_zbuffer(shadow_w, shadow_h);

    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    tangent_render(diablo, framebuffer);
    tangent_render(floor, framebuffer);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            vec4 fragment = M * vec4{(double)x, (double)y, zbuffer.at(x + y * width), 1.};
            vec4 q = N * fragment;
            vec3 p = q.xyz() / q.w;
            mask[x + y * width] = (
                fragment.z < -100 || (p.x < 0 || p.x >= shadow_w || p.y < 0 || p.y > shadow_h)
                || (p.z > shadow_depth[int(p.x) + int(p.y) * shadow_w] - .03));
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


    constexpr double ao_radius = .1;  // ssao ball radius in normalized device coordinates
    constexpr int nsamples = 128;     // number of samples in the ball
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-ao_radius, ao_radius);
    auto smoothstep = [](double edge0, double edge1, double x) 
    {         // smoothstep returns 0 if the input is less than the left edge,
        double t = std::max(0.0, std::min(1.0, (x - edge0)/(edge1 - edge0)));  // 1 if the input is greater than the right edge,
        return t*t*(3 - 2*t);                                        // Hermite interpolation inbetween. The derivative of the smoothstep function is zero at both edges.
    };

#pragma omp parallel for
    for (int x=0; x<width; x++) 
    {
        for (int y=0; y<height; y++) 
        {
            double z = zbuffer[x+y*width];
            if (z < -100)
            {
                continue;
            }
            vec4 fragment = Viewport.invert() * vec4{(double)x, (double)y, z, 1.};
            double vote = 0;
            double voters = 0;
            for(int i = 0; i < nsamples; i++) 
            {
                vec4 p = Viewport * (fragment + vec4{dist(gen), dist(gen), dist(gen), 0.});
                if (p.x<0 || p.x>=width || p.y<0 || p.y>=height)
                {
                    continue;
                }
                double d = zbuffer[int(p.x) + int(p.y)*width];
                if (z + 5 * ao_radius < d)// range check to remove the dark halo
                {
                    continue;
                }
                voters++;
                vote += d > p.z;
            }
            double ssao = smoothstep(0, 1, 1 - (vote / voters) * .4);
            TGAColor c = framebuffer.get(x, y);
            framebuffer.set(x, y, { (std::uint8_t)(c[0]*ssao), (std::uint8_t)(c[1]*ssao), (std::uint8_t)(c[2]*ssao), c[3] });
        }
    }

    framebuffer.write_tga_file("ssao.tga");
    system("open ssao.tga");

    return 0;
}