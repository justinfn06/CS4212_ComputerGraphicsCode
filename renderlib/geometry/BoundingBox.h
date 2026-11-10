#pragma once

#include <algorithm>
#include <limits>

#include "../math/ray.h"

class BoundingBox
{
public:
    point3 llb;
    point3 urf;
    BoundingBox()
        : llb(
              std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity(),
              std::numeric_limits<double>::infinity()),
          urf(
              -std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity(),
              -std::numeric_limits<double>::infinity())
    {
    }

    BoundingBox(const point3& a, const point3& b)
    {
        // makes a bounding box that goes around both a and b
        for (int axis = 0; axis < 3; ++axis) {
            llb[axis] = std::min(a[axis], b[axis]);
            urf[axis] = std::max(a[axis], b[axis]);
        }

        constexpr double minimumWidth = 0.0001;

        // expand if too small
        for (int axis = 0; axis < 3; ++axis) {
            if (urf[axis] - llb[axis] < minimumWidth) {
                const double padding = minimumWidth / 2.0;
                llb[axis] -= padding;
                urf[axis] += padding;
            }
        }
    }

    // this makes a bounding box that goes around both a and b
    BoundingBox(const BoundingBox& a, const BoundingBox& b)
    {
        for (int axis = 0; axis < 3; ++axis) {
            llb[axis] = std::min(a.llb[axis], b.llb[axis]);
            urf[axis] = std::max(a.urf[axis], b.urf[axis]);
        }
    }

    // this function checks if ray intersects bounding box within the range
    bool hit(const ray& r, double tMin, double tMax) const
    {
        // check for intersection at each axis
        for (int axis = 0; axis < 3; ++axis) {
            const double origin = r.origin()[axis];
            const double direction = r.direction()[axis];

            // if parallel, check if origin is in bounds
            if (direction == 0.0) {
                if (origin < llb[axis] || origin > urf[axis]) {
                    return false;
                }

                continue;
            }

            // calculate intersection points in plane
            double t0 = (llb[axis] - origin) / direction;
            double t1 = (urf[axis] - origin) / direction;

            // ensure t0 is smaller
            if (t0 > t1) {
                std::swap(t0, t1);
            }

            tMin = std::max(tMin, t0);
            tMax = std::min(tMax, t1);

            // no intersection
            if (tMax <= tMin) {
                return false;
            }
        }

        return true;
    }
};