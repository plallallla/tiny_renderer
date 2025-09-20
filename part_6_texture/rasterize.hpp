#pragma once
#include "BaseShader.h"

using Triangle = std::array<vec4, 3>;

// 变换矩阵
mat<4, 4> ModelView, Perspective, Viewport;
// 深度缓存
std::vector<double> zbuffer;

/**
 * @brief 光栅化函数 模拟渲染管线流程
 * 
 * @param clip 裁剪空间坐标
 * @param shader 着色器对象
 * @param framebuffer 渲染上下文
 */
inline void rasterize(const Triangle& clip, const BaseShader& shader, TGAImage& framebuffer)
{
    vec4 ndc[3] = 
    {
        clip[0] / clip[0].w, 
        clip[1] / clip[1].w, 
        clip[2] / clip[2].w
    };
    vec2 screen[3] = 
    {
        (Viewport * ndc[0]).xy(), 
        (Viewport * ndc[1]).xy(), 
        (Viewport * ndc[2]).xy()
    };

    mat<3, 3> ABC = 
    {
        {
            {screen[0].x, screen[0].y, 1.}, 
            {screen[1].x, screen[1].y, 1.}, 
            {screen[2].x, screen[2].y, 1.}
        }
    };
    if (ABC.det() < 1)
    {
        return;
    }
    auto bbx = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto bby = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
#pragma omp parallel for
    for (int x = std::max<int>(bbx.first, 0); x <= std::min<int>(bbx.second, framebuffer.width() - 1); x++)
    { // clip the bounding box by the screen
        for (int y = std::max<int>(bby.first, 0); y <= std::min<int>(bby.second, framebuffer.height() - 1); y++)
        {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // barycentric coordinates of {x,y} w.r.t the triangle
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
            {
                continue;
            }
            double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z}; // linear interpolation of the depth
            if (z <= zbuffer[x + y * framebuffer.width()])
            {
                continue; // discard fragments that are too deep w.r.t the z-buffer
            }
            auto frag = shader.fragment(bc);
            if (frag.first)
            {
                continue;                             // fragment shader can discard current fragment
            }
            zbuffer[x + y * framebuffer.width()] = z; // update the z-buffer
            framebuffer.set(x, y, frag.second);       // update the framebuffer
        }
    }
}