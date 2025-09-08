#include "tgaimage.hpp"
#include <utility>

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if (steep)
    { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx)
    { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int e = -1 * (bx - ax);
    for (int x = ax; x <= bx; x++)
    {
        if (steep) // 绘制
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        e += 2 * abs(by - ay);
        if (e > 0)
        {
            y += (by > ay ? 1 : -1);
            e -= 2 * (bx - ax);
        }
    }
}

void triangle_line(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
{
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(cx, cy, ax, ay, framebuffer, color);
}

void triangle_scanline(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
{
    if (ay < by)
    {
        std::swap(ay, by);
        std::swap(ax, bx);
    }
    if (ay < cy)
    {
        std::swap(ay, cy);
        std::swap(ax, cx);
    }
    if (by < cy)
    {
        std::swap(by, cy);
        std::swap(bx, cx);
    }
    line(ax, ay, bx, by, framebuffer, green);
    line(bx, by, cx, cy, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, red);
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
{//利用重心坐标判断点是否在三角形上
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);

    if (total_area < 1) //backface culling + discarding triangles that cover less than a pixel 剔除退化三角形
    {
        return;
    }

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++)
    {
        for (int y = bbminy; y <= bbmaxy; y++)
        {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            framebuffer.set(x, y, color);
        }
    }
}

#include "geometry.hpp"
void project(int& x, int& y, vec4 v)
{ // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    x = (v.x + 1.) * 500;
    y = (v.y + 1.) * 500;
}

#include "model.hpp"
void Straightforward_rasterization_all_triangles()
{
    Model model("../obj/african_head/african_head.obj");
    TGAImage framebuffer(1000, 1000, TGAImage::RGB);

    for (int i = 0; i < model.nfaces(); i++)
    { // iterate through all triangles
        int ax, ay;
        project(ax, ay, model.vert(i, 0));
        int bx, by;
        project(bx, by, model.vert(i, 1));
        int cx, cy;
        project(cx, cy, model.vert(i, 2));
        TGAColor rnd;
        for (int c = 0; c < 3; c++)
        {
            rnd[c] = std::rand() % 255;
        }
        triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
    }
    framebuffer.write_tga_file("african_head_framebuffer.tga");
    system("open african_head_framebuffer.tga");
}

int main(int argc, char** argv)
{
    int width{250};
    int height{250};
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle_scanline(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangle_scanline(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangle_scanline(115, 83, 80, 90, 85, 120, framebuffer, green);
    // framebuffer.write_tga_file("framebuffer.tga");
    // system("open framebuffer.tga");

    Straightforward_rasterization_all_triangles();

    return 0;
}