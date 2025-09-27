#include "model.hpp"
#include "tgaimage.hpp"
#include "geometry.hpp"
#include <cstdlib>
#include <utility>
#include <vector>

using namespace std;
using Triangle4 = array<vec4, 3>;
using Triangle2 = array<vec2, 3>;
using Models = vector<Model>;

constexpr auto height = 800;
constexpr auto width = 800;
constexpr vec3 light{1, 1, 1};
constexpr vec3 eye{-1, 0, 2};
constexpr vec3 center{ 0, 0, 0};
constexpr vec3 up{ 0, 1, 0};
vector<double> depth(width * height, -1000.0);

using scene = vector<Model>;

struct Uniform
{
    mat<4, 4> ModelView;
    mat<4, 4> Perspective;
    mat<4, 4> Viewport;
} uniform;

struct Shader
{
    Model _m;
    vec4 _l{1,1,1,0};
    Triangle4 tri;
    Triangle2 varying_uv;
    Triangle4 varying_nrm;

    // 顶点着色
    vec4 vertex(const int face, const int vert)
    {
        auto pt = _m.vert(face, vert);
        varying_uv[vert] = _m.uv(face, vert);
        varying_nrm[vert] = uniform.ModelView.invert_transpose() * _m.normal(face, vert);
        vec4 gl_Position = uniform.ModelView * _m.vert(face, vert);
        tri[vert] = gl_Position;
        return uniform.Perspective * gl_Position;
    }

    // 纹理采样
    TGAColor sample2D(const TGAImage &img, const vec2 &uv) const
    {
        return img.get(uv.x * img.width(), uv.y * img.height());
    }

    // 片元着色
    pair<bool, TGAColor> fragment(vec3 bc)
    {
        mat<2,4> E{ tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U{ varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        mat<4,4> D
        {
            normalized(T[0]), 
            normalized(T[1]),
            normalized(varying_nrm[0]*bc[0] + varying_nrm[1]*bc[1] + varying_nrm[2]*bc[2]),
            {0,0,0,1}
        };
        vec2 uv = varying_uv[0] * bc[0] + varying_uv[1] * bc[1] + varying_uv[2] * bc[2];
        vec4 n = normalized(D.transpose() * _m.tangent_normal(uv));
        vec4 r = normalized(n * (n * _l)*2 - _l);
        double ambient  = .4;
        double diffuse  = 1.*std::max(0., n * _l);
        double specular = (3.*sample2D(_m.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);
        TGAColor gl_FragColor = sample2D(_m.diffuse(), uv);
        for (int channel : {0,1,2})
        {
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        }
        return {true, gl_FragColor};   
    }
};

// 设置变换矩阵 lookat函数
mat<4, 4> get_modelview(const vec3& _eye, const vec3& _up, const vec3& _center)
{
    mat<4, 4> model =
    {
        1.,0.,0.,0.,
        0.,1.,0.,0.,
        0.,0.,1.,0.,
        0.,0.,0.,1.,
    };
    vec3 n = normalized(_eye - _center);
    vec3 l = normalized(cross(_up, n));
    vec3 m = normalized(cross(n, l));
    mat<4, 4> view =
    {
        l.x,l.y,l.z,-center.x,
        m.x,m.y,m.z,-center.y,
        n.x,n.y,n.z,-center.z,
        0.,0.,0.,1.,
    };
    return view * model;
}

// 透视矩阵
mat<4, 4> get_perspective(const vec3& _eye)
{
    auto f = norm(_eye - center);
    return
    {
        1.,0.,0.,0.,
        0.,1.,0.,0.,
        0.,0.,1.,0.,
        0.,0.,-1./f,1.,
    };
}

// 视窗矩阵
mat<4, 4> get_viewport()
{
    auto x = width / 16.;
    auto y = height / 16.;
    auto w = width * 7. / 8;
    auto h = height * 7. / 8;
    return 
    {
        w/2.,0.,0.,x+w/2.,
        0.,h/2.,0.,y+h/2.,
        0.,0.,1.,0.,
        0.,0.,0.,1.,
    };
}

// 片元组装
Triangle4 primitive_assembly(const vec4& _1, const vec4& _2, const vec4& _3)
{
    return {_1, _2, _3};
}

// 包围盒计算 左右上下
void get_aabb(const Triangle2& t, int& left, int& right, int& top, int& bottom)
{
    auto aabb_x = std::minmax({t[0].x, t[1].x, t[2].x});
    left = max<int>(aabb_x.first, 0);
    right = min<int>(aabb_x.second, width - 1);
    auto aabb_y = minmax({t[0].y, t[1].y, t[2].y});
    top = max<int>(aabb_y.first, 0);
    bottom = min<int>(aabb_y.second, height - 1);
}

// 光栅化
void rasterize(const Triangle4& t, Shader& s, TGAImage& b)
{
    // 归一化坐标
    Triangle4 ndc{t[0] / t[0].w, t[1] / t[1].w, t[2] / t[2].w};
    // 转为屏幕坐标
    Triangle2 screen
    {
        (uniform.Viewport * ndc[0]).xy(), 
        (uniform.Viewport * ndc[1]).xy(), 
        (uniform.Viewport * ndc[2]).xy(), 
    };
    // 计算三角形矩阵
    mat<3, 3> ABC = 
    {
        screen[0].x, screen[0].y, 1.,
        screen[1].x, screen[1].y, 1.,
        screen[2].x, screen[2].y, 1.
    };
    // 剔除背面三角形与面积过小三角形
    if (ABC.det() < 1)
    {
        return;
    }
    // 获取包围盒
    int left = 0, right = 0, top = 0, bottom = 0;
    get_aabb(screen, left, right, top, bottom);
    // 遍历包围盒
    for (int x = left; x <= right; x++)
    {
        for (int y = top; y <= bottom; y++)
        {
            // 计算重心坐标
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            // 剔除不在三角形中的点
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
            {
                continue;
            }
            // 插值得到深度z值，进行深度测试，缓存z值
            double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z};
            if (z <= depth[x + y * width])
            {
                continue;
            }
            depth[x + y * width] = z;
            //绘制
            auto frag_result = s.fragment(bc);
            if (frag_result.first)
            {
                b.set(x, y, frag_result.second);
            }
        }
    }
}

// 只记录深度的渲染 用于shadowmap的生成
void rasterize(const Triangle4& t, Shader& s, vector<double>& shadow_depth)
{
    // 归一化坐标
    Triangle4 ndc{t[0] / t[0].w, t[1] / t[1].w, t[2] / t[2].w};
    // 转为屏幕坐标
    Triangle2 screen
    {
        (uniform.Viewport * ndc[0]).xy(), 
        (uniform.Viewport * ndc[1]).xy(), 
        (uniform.Viewport * ndc[2]).xy(), 
    };
    // 计算三角形矩阵
    mat<3, 3> ABC = 
    {
        screen[0].x, screen[0].y, 1.,
        screen[1].x, screen[1].y, 1.,
        screen[2].x, screen[2].y, 1.
    };
    // 剔除背面三角形与面积过小三角形
    if (ABC.det() < 1)
    {
        return;
    }
    // 获取包围盒
    int left = 0, right = 0, top = 0, bottom = 0;
    get_aabb(screen, left, right, top, bottom);
    // 遍历包围盒
    for (int x = left; x <= right; x++)
    {
        for (int y = top; y <= bottom; y++)
        {
            // 计算重心坐标
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            // 剔除不在三角形中的点
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
            {
                continue;
            }
            // 插值得到深度z值，进行深度测试，缓存z值
            double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z};
            if (z <= shadow_depth[x + y * height])
            {
                continue;
            }
            shadow_depth[x + y * height] = z;
        }
    }
}

// 从光源视角渲染一次
void get_shadowmap(const Models& models, vector<double>& shadow_depth)
{
    uniform.ModelView = get_modelview(light, up, center);
    uniform.Perspective = get_perspective(eye);
    uniform.Viewport = get_viewport();
    for (auto& model : models)
    {
        Shader shader{ model };
        for (int i = 0; i < model.nfaces(); i++)
        {
            Triangle4 primitive = primitive_assembly(shader.vertex(i, 0), shader.vertex(i, 1), shader.vertex(i, 2));
            rasterize(primitive, shader, shadow_depth);
        }
    }
}

// 以蒙版形式应用阴影
void apply_shadowmap(const vector<double>& shadowmap, TGAImage& framebuffer)
{
    mat<4, 4> M = (uniform.Viewport * uniform.Perspective * uniform.ModelView).invert();
    mat<4, 4> N = uniform.Viewport * get_perspective(light) * get_modelview(light, up, center);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            // 计算 N * M ^ {-1} * pt, 将渲染坐标转到光源下
            vec4 fragment = M * vec4{(double)x, (double)y, depth.at(x + y * width), 1.};
            vec4 q = N * fragment;
            vec3 p = q.xyz() / q.w;
            if (fragment.z < -100 || (p.x < 0 || p.x >= width || p.y < 0 || p.y > height)
                || (p.z > shadowmap[int(p.x) + int(p.y) * width] - .03))// .03作为bias偏移
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
}

// 渲染
void render(const Models& models, TGAImage& buffer)
{
    uniform.ModelView = get_modelview(eye, up, center);
    uniform.Perspective = get_perspective(eye);
    uniform.Viewport = get_viewport();
    for (auto& model : models)
    {
        Shader shader{ model };
        for (int i = 0; i < model.nfaces(); i++)
        {
            Triangle4 primitive = primitive_assembly(shader.vertex(i, 0), shader.vertex(i, 1), shader.vertex(i, 2));
            rasterize(primitive, shader, buffer);
        }
    }
}

int main()
{
    //读取本地模型
    Models m;
    m.emplace_back("../obj/diablo3_pose/diablo3_pose.obj");
    m.emplace_back("../obj/floor.obj");
    //预处理shadow
    vector<double> shadowmap(height*width, -1000.);
    get_shadowmap(m, shadowmap);
    //渲染
    TGAImage buffer{height, width, TGAImage::RGB, TGAColor{177, 195, 209, 255}};
    render(m, buffer);
    //应用shadow蒙版
    apply_shadowmap(shadowmap, buffer);
    //保存渲染结果
    buffer.write_tga_file("tiny_renderer_buffer.tga");
    system("open tiny_renderer_buffer.tga");
    return 0;
}