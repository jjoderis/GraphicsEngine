#ifndef CORE_COMPONENTS_BOUNDING_BOX
#define CORE_COMPONENTS_BOUNDING_BOX

#include "../../Math/math.h"
#include <memory>

namespace Engine
{
class GeometryComponent;

class BoundingBoxComponent
{
public:
    BoundingBoxComponent() = delete;
    BoundingBoxComponent(const GeometryComponent &geometry);

    void calculateBoundingBox(const GeometryComponent &geometry);

    const Math::Vector3 &getMin() const;
    const Math::Vector3 &getMax() const;

private:
    Math::Vector3 m_min;
    Math::Vector3 m_max;
};

} // namespace Engine

#endif