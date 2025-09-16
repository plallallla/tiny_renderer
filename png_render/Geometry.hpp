#pragma once
#include <glm/glm.hpp>
#include <array>

/**
包围盒类
*/
class BoundingBox
{
    BoundingBox() = default;
    BoundingBox(const glm::vec3& a, const glm::vec3& b) : min(a), max(b)
    {
    }

    /* 获取顶点 */
    void getCorners(std::array<glm::vec3, 8>& dst) const
    {
        dst[0] = glm::vec3(min.x, max.y, max.z);
        dst[1] = glm::vec3(min.x, min.y, max.z);
        dst[2] = glm::vec3(max.x, min.y, max.z);
        dst[3] = glm::vec3(max.x, max.y, max.z);

        dst[4] = glm::vec3(max.x, max.y, min.z);
        dst[5] = glm::vec3(max.x, min.y, min.z);
        dst[6] = glm::vec3(min.x, min.y, min.z);
        dst[7] = glm::vec3(min.x, max.y, min.z);
    }

    /* 变换为一个新包围盒 */
    BoundingBox transform(const glm::mat4& matrix) const
    {
        std::array<glm::vec3, 8> corners;
        getCorners(corners);

        corners[0] = matrix * glm::vec4(corners[0], 1.f);
        glm::vec3 newMin = corners[0];
        glm::vec3 newMax = corners[0];
        for (int i = 1; i < 8; i++)
        {
            corners[i] = matrix * glm::vec4(corners[i], 1.f);
            updateMinMax(&corners[i], &newMin, &newMax);
        }
        return {newMin, newMax};
    }

    /* 与另一个包围盒是否相交 */
    bool intersects(const BoundingBox& box) const
    {
        return ((min.x >= box.min.x && min.x <= box.max.x) || (box.min.x >= min.x && box.min.x <= max.x)) &&
               ((min.y >= box.min.y && min.y <= box.max.y) || (box.min.y >= min.y && box.min.y <= max.y)) &&
               ((min.z >= box.min.z && min.z <= box.max.z) || (box.min.z >= min.z && box.min.z <= max.z));
    }

    /* 与另一个包围盒进行融合 */
    void merge(const BoundingBox& box)
    {
        min.x = std::fmin(min.x, box.min.x);
        min.y = std::fmin(min.y, box.min.y);
        min.z = std::fmin(min.z, box.min.z);

        max.x = std::fmax(max.x, box.max.x);
        max.y = std::fmax(max.y, box.max.y);
        max.z = std::fmax(max.z, box.max.z);
    }

  protected:
    /* 更新包围盒 */
    static void updateMinMax(glm::vec3* point, glm::vec3* min, glm::vec3* max)
    {
        min->x = std::fmin(min->x, point->x);
        min->y = std::fmin(min->y, point->y);
        max->x = std::fmax(max->x, point->x);
        max->y = std::fmax(max->y, point->y);
    }

  public:
    glm::vec3 min{0.f, 0.f, 0.f};
    glm::vec3 max{0.f, 0.f, 0.f};
};

/**
平面类
*/
class Plane
{
  public:
    enum PlaneIntersects
    {
        Intersects_Cross = 0,   // 穿过
        Intersects_Tangent = 1, // 相切
        Intersects_Front = 2,   // 正面
        Intersects_Back = 3     // 背面
    };

    void set(const glm::vec3& n, const glm::vec3& pt)
    {
        _normal = glm::normalize(n);
        _d = -(glm::dot(_normal, pt));
    }

    /* pt到平面距离 */
    float distance(const glm::vec3& pt) const
    {
        return glm::dot(_normal, pt) + _d;
    }

    /* 获取平面法向量 */
    inline const glm::vec3& getNormal() const
    {
        return _normal;
    }

    /* 平面与包围盒关系 */
    Plane::PlaneIntersects intersects(const BoundingBox& box) const
    {
        glm::vec3 center = (box.min + box.max) * 0.5f;
        glm::vec3 extent = (box.max - box.min) * 0.5f;
        float d = distance(center);
        // 计算包围盒在平面法线方向上的投影半长
        float r = fabsf(extent.x * _normal.x) + fabsf(extent.y * _normal.y) + fabsf(extent.z * _normal.z);
        if (d == r)
        {
            return Plane::Intersects_Tangent;
        }
        else if (std::abs(d) < r)
        {
            return Plane::Intersects_Cross;
        }
        return (d > 0.0f) ? Plane::Intersects_Front : Plane::Intersects_Back;
    }

    /* 平面与点关系 */
    Plane::PlaneIntersects intersects(const glm::vec3& p0) const
    {
        float d = this->distance(p0);
        if (d == 0)
        {
            return Plane::Intersects_Tangent;
        }
        return (d > 0.0f) ? Plane::Intersects_Front : Plane::Intersects_Back;
    }

    /* 平面与直线关系 */
    Plane::PlaneIntersects intersects(const glm::vec3& p0, const glm::vec3& p1) const
    {
        auto state0 = intersects(p0);
        auto state1 = intersects(p1);
        if (state0 == state1)
        {
            return state0;
        }
        else if (state0 == Plane::Intersects_Tangent || state1 == Plane::Intersects_Tangent)
        {
            return Plane::Intersects_Tangent;
        }
        return Plane::Intersects_Cross;
    }

    /* 平面与三角形关系 */
    Plane::PlaneIntersects intersects(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2) const
    {
        Plane::PlaneIntersects state0 = intersects(p0, p1);
        Plane::PlaneIntersects state1 = intersects(p0, p2);
        Plane::PlaneIntersects state2 = intersects(p1, p2);
        if (state0 == state1 && state0 == state2)
        {
            return state0;
        }
        else if (state0 == Plane::Intersects_Cross || state1 == Plane::Intersects_Cross ||
                 state2 == Plane::Intersects_Cross)
        {
            return Plane::Intersects_Cross;
        }
        return Plane::Intersects_Tangent;
    }

  private:
    glm::vec3 _normal;
    float _d = 0;
};