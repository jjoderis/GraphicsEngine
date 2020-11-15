#ifndef CORE_COMPONENTS_GEOMETRY
#define CORE_COMPONENTS_GEOMETRY

#include <vector>
#include <initializer_list>
#include "../../Math/math.h"

namespace Engine {
    class GeometryComponent {
    private:
        std::vector<Math::Vector3> m_vertices;
        std::vector<unsigned int> m_faces;
    public:
        GeometryComponent();
        GeometryComponent(std::initializer_list<Math::Vector3> vertices, std::initializer_list<unsigned int> faces);
        std::vector<Math::Vector3>& getVertices();
        std::vector<unsigned int>& getFaces();

        // adds a single vertex
        void addVertex(Math::Vector3 && newVertex);

        // adds a face consisting of three indices
        void addFace(unsigned int a, unsigned int b, unsigned int c);
    };
}

#endif