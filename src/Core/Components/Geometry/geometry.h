#ifndef CORE_COMPONENTS_GEOMETRY
#define CORE_COMPONENTS_GEOMETRY

#include <vector>
#include <initializer_list>
#include "../../Math/math.h"

namespace Engine {
    class GeometryComponent {
    private:
        std::vector<Math::Vector3> m_vertices;
    public:
        GeometryComponent();
        GeometryComponent(std::initializer_list<Math::Vector3> vertices);
        std::vector<Math::Vector3>& getVertices();
    };
}

#endif