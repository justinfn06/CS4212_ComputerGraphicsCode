#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "shape.h"

class BoundingVolumeNode : public Shape
{
public:
    BoundingVolumeNode(
        std::vector<std::shared_ptr<Shape>> shapes,
        int axis = 0
    )
        : Shape()
    {
        const std::size_t count = shapes.size();

        if (count == 1) {
            leftChild = shapes[0];
            rightChild = nullptr;

            box = leftChild->boundingBox();
            return;
        }

        if (count == 2) {
            leftChild = shapes[0];
            rightChild = shapes[1];
        } else {
            std::sort(
                shapes.begin(),
                shapes.end(),
                [axis](
                    const std::shared_ptr<Shape>& a,
                    const std::shared_ptr<Shape>& b
                ) {
                    return a->centroid()[axis]
                         < b->centroid()[axis];
                }
            );

            const auto middle = shapes.begin() + count / 2;

            std::vector<std::shared_ptr<Shape>> leftShapes(
                shapes.begin(), middle
            );

            std::vector<std::shared_ptr<Shape>> rightShapes(
                middle, shapes.end()
            );

            const int nextAxis = (axis + 1) % 3;

            leftChild = std::make_shared<BoundingVolumeNode>(
                leftShapes, nextAxis
            );

            rightChild = std::make_shared<BoundingVolumeNode>(
                rightShapes, nextAxis
            );
        }

        box = BoundingBox(
            leftChild->boundingBox(),
            rightChild->boundingBox()
        );
    }

    bool intersect(
        const ray& r,
        float t_min,
        float& t_max,
        HitStruct& hit
    ) const override
    {
        if (!box.hit(r, t_min, t_max)) {
            return false;
        }

        const bool hitLeft =
            leftChild->intersect(r, t_min, t_max, hit);

        bool hitRight = false;

        if (rightChild) {
            hitRight =
                rightChild->intersect(r, t_min, t_max, hit);
        }

        return hitLeft || hitRight;
    }

    BoundingBox boundingBox() const override
    {
        return box;
    }

    point3 centroid() const override
    {
        return (box.llb + box.urf) / 2.0;
    }

    vec3 getColor() const override
    {
        return vec3(0, 0, 0);
    }

private:
    std::shared_ptr<Shape> leftChild;
    std::shared_ptr<Shape> rightChild;

    BoundingBox box;
};