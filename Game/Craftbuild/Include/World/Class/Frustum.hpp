#pragma once

#include <Core/core.hpp>

namespace Craftbuild {
    class Frustum {
    private:
        enum Plane {
            LEFT = 0,
            RIGHT,
            BOTTOM,
            TOP,
            NEAR,
            FAR,
            COUNT
        };

        glm::vec4 planes[COUNT];

    public:
        void update(const glm::mat4& view_proj) {
            glm::mat4 mat = glm::transpose(view_proj);

            planes[LEFT] = mat[3] + mat[0];
            planes[RIGHT] = mat[3] - mat[0];
            planes[BOTTOM] = mat[3] + mat[1];
            planes[TOP] = mat[3] - mat[1];
            planes[NEAR] = mat[3] + mat[2];
            planes[FAR] = mat[3] - mat[2];

            for (int i = 0; i < COUNT; i++) {
                float length = glm::length(glm::vec3(planes[i]));
                planes[i] /= length;
            }
        }

        bool is_box_visible(const glm::vec3& min, const glm::vec3& max) const {
            for (int i = 0; i < COUNT; i++) {
                glm::vec3 positive = min;
                if (planes[i].x > 0) positive.x = max.x;
                if (planes[i].y > 0) positive.y = max.y;
                if (planes[i].z > 0) positive.z = max.z;

                float distance = planes[i].x * positive.x +
                    planes[i].y * positive.y +
                    planes[i].z * positive.z +
                    planes[i].w;

                if (distance < 0) {
                    return false;
                }
            }
            return true;
        }
    };
}