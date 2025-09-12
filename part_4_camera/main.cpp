#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"
#include <cmath>

constexpr auto height = 800;
constexpr auto width = 800;

constexpr vec3 eye{-1, 0, 2};   // camera position
constexpr vec3 center{0, 0, 0}; // camera direction
constexpr vec3 up{0, 1, 0};     // camera up vector

mat<4, 4> Model_View_M;
mat<4, 4> Viewport_M;
mat<4, 4> Perspective_M;

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
    return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

void rasterize(const vec4 clip[3], std::vector<double>& zbuffer, TGAImage& framebuffer, const TGAColor color)
{
    vec4 ndc[3] = 
    {
        clip[0] / clip[0].w, 
        clip[1] / clip[1].w, 
        clip[2] / clip[2].w
    }; // normalized device coordinates
    vec2 screen[3] = 
    {
        (Viewport_M * ndc[0]).xy(), 
        (Viewport_M * ndc[1]).xy(),
        (Viewport_M * ndc[2]).xy()
    }; // screen coordinates

    mat<3, 3> ABC = {{{screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.}}};
    if (ABC.det() < 1)
    {
        return; // backface culling + discarding triangles that cover less than a pixel
    }

    auto [bbminx, bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy, bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
#pragma omp parallel for
    for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++)
    { // clip the bounding box by the screen
        for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++)
        {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
             // barycentric coordinates of {x,y} w.r.t the triangle
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
            {
                continue; // negative barycentric coordinate => the pixel is outside the triangle
            }
            double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z};
            if (z <= zbuffer[x + y * framebuffer.width()])
            {
                continue;
            }
            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

void lookat(const vec3& eye, const vec3& center, const vec3& up)
{
    vec3 n = normalized(eye - center); // 摄像机 -Z 方向
    vec3 l = normalized(cross(up, n)); // 右方向
    vec3 m = normalized(cross(n, l));  // 上方向
    Model_View_M = 
    mat<4, 4>
    {
        {
            {l.x, l.y, l.z, 0}, 
            {m.x, m.y, m.z, 0}, 
            {n.x, n.y, n.z, 0}, 
            {0, 0, 0, 1}
        }
    } 
    * mat<4, 4>
    {
        {
            {1, 0, 0, -center.x}, 
            {0, 1, 0, -center.y}, 
            {0, 0, 1, -center.z}, 
            {0, 0, 0, 1}
        }
    };
}

void perspective(const double f)
{
    Perspective_M = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void viewport(const int x, const int y, const int w, const int h)
{
    Viewport_M = {{{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

int main()
{
    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());
    Model model{"../obj/diablo3_pose/diablo3_pose.obj"};

    lookat(eye, center, up);
    perspective(norm(eye - center));
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);//上下左右预留十六分之一的距离

    for (int i=0; i<model.nfaces(); i++) 
    { // iterate through all triangles
        vec4 clip[3];
        for (int d : {0,1,2}) 
        {            // assemble the primitive
            vec4 v = model.vert(i, d);
            clip[d] = Perspective_M * Model_View_M * vec4{v.x, v.y, v.z, 1.};
        }
        TGAColor rnd;
        for (int c=0; c<3; c++) rnd[c] = std::rand() % 255;
        {
            rasterize(clip, zbuffer, framebuffer, rnd); // rasterize the primitive
        }
    }
    framebuffer.write_tga_file("frame_img.tga");
    system("open frame_img.tga");
    return 0;
}