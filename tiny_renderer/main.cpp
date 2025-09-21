#include "model.hpp"
#include "tgaimage.hpp"
#include "geometry.hpp"
#include <vector>

using namespace std;
using Triangle = array<vec4, 3>;

constexpr auto height = 800;
constexpr auto width = 800;
constexpr vec3 eye = {};
constexpr vec3 center = {};
constexpr vec3 up = {};

struct Uniform
{
    mat<4, 4> ModelView;
    mat<4, 4> Perspective;
    mat<4, 4> Viewport;
} uniform;

struct Shader
{
    Model _m;

    vec4 vertex(const int face, const int vert)
    {
        return {};
    }

    void fragment()
    {

    }
};

void set_transform(mat<4, 4>& ModelView, mat<4, 4>& Perspective, mat<4, 4>& Viewport)
{
    // model矩阵
    mat<4, 4> model =
    {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };
    vec3 n = normalized(eye - center);
    vec3 l = normalized(cross(up, n));
    vec3 m = normalized(cross(n, l));
    mat<4, 4> view =
    {
        
    };
    ModelView = view * model;
    // Perspective矩阵
    
}

Triangle primitive_assembly(const vec4& _1, const vec4& _2, const vec4& _3)
{
    return {_1, _2, _3};
}

void rasterize(const Triangle& t, const Shader& s, TGAImage& b)
{

}

void render(const Model& model, TGAImage& buffer)
{
    set_transform(uniform.ModelView, uniform.Perspective, uniform.Viewport);
    vector<double> depth(width * height, -1000.0);
    Shader shader{ model };
    for (int i = 0; i < model.nfaces(); i++)
    {
        Triangle primitive = primitive_assembly(shader.vertex(i, 0), shader.vertex(i, 1), shader.vertex(i, 2));
        rasterize(primitive, shader, buffer);
    }
}

int main()
{
    //读取本地模型
    Model m{""};
    //渲染
    TGAImage buffer{height, width, TGAImage::RGB};
    render(m, buffer);
    //保存渲染结果
    buffer.write_tga_file("tiny_renderer_buffer.tga");
    return 0;
}