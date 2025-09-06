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

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
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

int main(int argc, char** argv)
{
    int width{250};
    int height{250};
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle_scanline(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangle_scanline(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangle_scanline(115, 83, 80, 90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    system("open framebuffer.tga");

    return 0;
}