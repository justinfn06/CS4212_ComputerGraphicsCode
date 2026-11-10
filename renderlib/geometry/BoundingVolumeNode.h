#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "shape.h"

class BoundingVolumeNode : public Shape
{
public:
    BoundingVolumeNode(std::vector<std::shared_ptr<Shape>> shapes, int axis = 0)
        : Shape()
    {
        const std::size_t count = shapes.size();

        // if only one shape, node becomes leaf node (one left node, no right node)
        if (count == 1) {
            leftChild = shapes[0];
            rightChild = nullptr;

            box = leftChild->boundingBox();
            return;
        }

        // if two shapes, node becomes leaf node (one left node, one right node)
        if (count == 2) {
            leftChild = shapes[0];
            rightChild = shapes[1];
        } else { // if more than two shapes, split into two child nodes
            std::sort( // sort shapes so that its easier to split into two child nodes
                shapes.begin(),
                shapes.end(),
                [axis](
                    const std::shared_ptr<Shape>& a,
                    const std::shared_ptr<Shape>& b
                ) {
                    return a->centroid()[axis] < b->centroid()[axis];
                }
            );

            const auto middle = shapes.begin() + count / 2; // find middle

            // split shapes into two
            std::vector<std::shared_ptr<Shape>> leftShapes(
                shapes.begin(), middle
            );

            // split shapes into two
            std::vector<std::shared_ptr<Shape>> rightShapes(
                middle, shapes.end()
            );

            // this math alternates the axis to split on for each child node
            const int nextAxis = (axis + 1) % 3;

            // make child node
            leftChild = std::make_shared<BoundingVolumeNode>(
                leftShapes, nextAxis
            );

            // make child node
            rightChild = std::make_shared<BoundingVolumeNode>(
                rightShapes, nextAxis
            );
        }

        // make bounding box
        box = BoundingBox(
            leftChild->boundingBox(),
            rightChild->boundingBox()
        );
    }

    // check if ray intersects node. if so, check if it intersects child nodes. if so, return true
    bool intersect(
        const ray& r,
        float t_min,
        float& t_max,
        HitStruct& hit
    ) const override
    {
        // if ray doesn't intersect box, return false
        if (!box.hit(r, t_min, t_max)) {
            return false;
        }


        // check if ray intersects child
        const bool hitLeft = leftChild->intersect(r, t_min, t_max, hit);

        bool hitRight = false;

        // if right child exists, check if ray intersects it.
        if (rightChild) {
            hitRight = rightChild->intersect(r, t_min, t_max, hit);
        }

        return hitLeft || hitRight; // return true if ray intersects either child
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