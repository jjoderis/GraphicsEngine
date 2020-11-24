#ifndef CORE_COMPONENTS_GEOMETRY
#define CORE_COMPONENTS_GEOMETRY

#include <vector>
#include <memory>
#include <initializer_list>
#include "../../Math/math.h"

namespace Engine {
    class GeometryComponent {
    private:
        std::vector<Math::Vector3> m_vertices;
        std::vector<Math::Vector3> m_normals;
        std::vector<unsigned int> m_faces;
    public:
        GeometryComponent();
        GeometryComponent(std::initializer_list<Math::Vector3> vertices, std::initializer_list<unsigned int> faces);
        std::vector<Math::Vector3>& getVertices();
        std::vector<Math::Vector3>& getNormals();
        std::vector<unsigned int>& getFaces();

        // adds a single vertex
        void addVertex(Math::Vector3 && newVertex);

        // adds a face consisting of three indices
        void addFace(unsigned int a, unsigned int b, unsigned int c);

        void calculateNormals();
    };

    /**
     *  creates a component representing a sphere
     * 
     *  param radius: the distance of all points from the center of the sphere (0,0,0)
     *  param hIntersections: the number of points on a latitude
     *  param vIntersections: the number of points when going from the north to the south pole (excluding the poles)
     **/
    std::shared_ptr<GeometryComponent> createSphereGeometry(float radius, int hIntersections, int vIntersections);
}

#endif