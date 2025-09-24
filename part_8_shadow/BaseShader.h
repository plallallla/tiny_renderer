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
     * @param bc 当前着色片元的重心坐标 用于片元着色差值
     * @return std::pair<bool, TGAColor> 
     */
    virtual std::pair<bool, TGAColor> fragment(const vec3 bc) const = 0;
};