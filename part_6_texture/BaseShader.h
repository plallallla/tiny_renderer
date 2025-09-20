#pragma once
#include "geometry.hpp"
#include "tgaimage.hpp"

/**
 * @brief 着色器基类
 * 
 */
struct BaseShader
{
    /**
     * @brief 顶点着色器
     * 
     * @param face 
     * @param vert 
     * @return vec4 
     */
    virtual vec4 vertex(const int face, const int vert) = 0;

    /**
     * @brief 片元着色器
     * 
     * @param bar
     * @return std::pair<bool, TGAColor> 
     */
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};