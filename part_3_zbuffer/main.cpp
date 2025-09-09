#include "model.hpp"
#include "tgaimage.hpp"

constexpr auto height = 800;
constexpr auto width = 800;

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

/**
传入zbuffer用于记录每个点的最小深度
zbuffer每个点的初始深度都是255（最大深度值）
如果当前点的深度值小于buffer中的深度值、则绘制这个点、同时更新最小深度
这样保证最终只有深度最小的点被绘制
*/ 
void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& zbuffer,
              TGAImage& framebuffer, TGAColor color)
{
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 1)
        return; // backface culling + discarding triangles that cover less than a pixel

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
                continue; // negative barycentric coordinate => the pixel is outside the triangle
            }
            unsigned char z = // 重心坐标插值得到每个三角形的z值
                static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            if (z <= zbuffer.get(x, y)[0])
            {
                continue;
            }
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

// First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
// Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
// we want to shift the vector (x,y) and then scale it to span the entire screen.
void project(int& x, int& y, int& z, vec4 v)
{
    x = (v.x + 1.) * width / 2;
    y = (v.y + 1.) * height / 2;
    z = (v.z + 1.) * 255. / 2;//其实不是实际深度，但是能保证相对深度一致
}

int main()
{
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    Model model{"../obj/diablo3_pose/diablo3_pose.obj"};

#pragma omp parallel for
    for (int i = 0; i < model.nfaces(); i++)
    { // iterate through all triangles
        int ax, ay, az;
        project(ax, ay, az, model.vert(i, 0));
        int bx, by, bz;
        project(bx, by, bz, model.vert(i, 1));
        int cx, cy, cz;
        project(cx, cy, cz, model.vert(i, 2));
        TGAColor rnd;
        for (int c = 0; c < 3; c++)
        {
            rnd[c] = std::rand() % 255;
        }
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }
    framebuffer.write_tga_file("frame_img.tga");
    zbuffer.write_tga_file("z_buffer_img.tga");
    system("open frame_img.tga");
    system("open z_buffer_img.tga");
    return 0;
}