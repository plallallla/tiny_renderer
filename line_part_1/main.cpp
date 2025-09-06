#include "tgaimage.hpp"
#include <cmath>

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

void line_0(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    for (float t = 0.; t < 1.; t += .02)
    {
        int x = std::round(ax + (bx - ax) * t);
        int y = std::round(ay + (by - ay) * t);
        framebuffer.set(x, y, color);
    }
}

// still insufficient sampling
void line_1(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    if (ax > bx)
    { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x = ax; x <= bx; x++)
    {
        float t = (x - ax) / static_cast<float>(bx - ax);
        int y = std::round(ay + (by - ay) * t);
        framebuffer.set(x, y, color);
    }
}

// DDA算法绘制直线 考虑微分增量
// y = k * x + b
// 判断k的大小
// 如果k大于1，则说明y变化更快，应该基于y的递增进行采样
void line_2(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    for (int x = ax; x <= bx; x++)
    {
        float t = (x - ax) / static_cast<float>(bx - ax);
        int y = std::round(ay + (by - ay) * t);
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
    }
}

void line_3(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    float error = 0;
    for (int x = ax; x <= bx; x++)
    {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        error += std::abs(by - ay) / static_cast<float>(bx - ax);
        if (error > .5)
        {
            y += by > ay ? 1 : -1;
            error -= 1.;
        }
    }
}

// 优化后避免浮点运算，中点Brensenham
void line_4(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    int ierror = 0;
    for (int x = ax; x <= bx; x++)
    {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by - ay);
        if (ierror > bx - ax)
        {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx - ax);
        }
    }
}

// 最终优化
void line_5(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    int ierror = 0;
    for (int x = ax; x <= bx; x++)
    {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by - ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx - ax) * (ierror > bx - ax);
    }
}

// 以下函数推导，详见README

void line_mid(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    int d = 1 - 2 * abs(by - ay); // D0 = 1 - 2*dy
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
        d -= 2 * abs(by - ay); // D0 > 0时,D1 = D0 - 2dy, y方向不变
        if (d < 0)             // D0 < 0时,D1 = D0 + 2dx - 2dy,y方向上进一
        {
            y += (by > ay ? 1 : -1);
            d += 2 * bx - ax;
        }
    }
}

void line_d(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    int d = 1 - 2 * abs(by - ay); // D0 = 1 - 2*dy
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
        d -= 2 * abs(by - ay); // D0 > 0时,D1 = D0 - 2dy, y方向不变
        if (d < 0)             // D0 < 0时,D1 = D0 + 2dx - 2dy,y方向上进一
        {
            y += (by > ay ? 1 : -1);
            d += 2 * bx - ax;
        }
    }
}

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

#include "geometry.hpp"
void project(int& x, int& y, vec4 v)
{ // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    x = (v.x + 1.) * 500;
    y = (v.y + 1.) * 500;
}

#include "model.hpp"
void homework()
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
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }
    for (int i = 0; i < model.nverts(); i++)
    { // iterate through all vertices
        int x, y;
        project(x, y, model.vert(i)); // project it to the screen
        framebuffer.set(x, y, white);
    }
    framebuffer.write_tga_file("framebuffer.tga");
    system("open framebuffer.tga");
}

int main(int argc, char** argv)
{
    constexpr int width = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, framebuffer, blue);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, yellow);
    line(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    // framebuffer.write_tga_file("framebuffer.tga");
    // system("open framebuffer.tga");

    homework();

    return 0;
}