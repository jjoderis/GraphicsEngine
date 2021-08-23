#ifndef CORE_COMPONENTS_GEOMETRY
#define CORE_COMPONENTS_GEOMETRY

#include "../../Math/math.h"
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <vector>

namespace Engine
{
class GeometryComponent;

class AccelerationStructure
{

public:
    AccelerationStructure();
    AccelerationStructure(AccelerationStructure *parent, int start, int end, GeometryComponent *geometry);
    AccelerationStructure &operator=(const AccelerationStructure &other);

    Engine::Point3 &getMin();
    Engine::Point3 &getMax();
    std::vector<int> &getTriangles();
    std::vector<AccelerationStructure> &getChildren();

private:
    AccelerationStructure *m_parent{nullptr};
    Engine::Point3 m_min{};
    Engine::Point3 m_max{};
    int m_startIndex{0};
    int m_endIndex{0};
    std::vector<int> m_triangles{};
    std::vector<AccelerationStructure> m_children{};
    GeometryComponent *m_geometry;
    int m_maxVertices{50};

    void subdivide(std::vector<int> &vertexRef);
    bool assignTriangle(int index, const Engine::Point3 &min, const Engine::Point3 &max);
};

class GeometryComponent
{
private:
    std::vector<Point3> m_vertices;
    std::vector<Vector3> m_normals;
    std::vector<Vector2> m_texCoords;
    std::vector<unsigned int> m_faces;

    AccelerationStructure m_bounding{};

public:
    GeometryComponent();
    GeometryComponent(std::initializer_list<Point3> vertices, std::initializer_list<unsigned int> faces);
    GeometryComponent(std::vector<Point3> &&vertices,
                      std::vector<Vector3> &&normals,
                      std::vector<unsigned int> &&faces);
    std::vector<Point3> &getVertices();
    const std::vector<Point3> &getVertices() const;
    std::vector<Vector3> &getNormals();
    std::vector<Vector2> &getTexCoords();
    std::vector<unsigned int> &getFaces();

    AccelerationStructure &getAccStructure();

    // adds a single vertex
    void addVertex(Point3 &&newVertex);

    // adds a face consisting of three indices
    void addFace(unsigned int a, unsigned int b, unsigned int c);

    void calculateNormals();
    void calculateBoundingBox();
};

/**
 *  creates a component representing a sphere
 *
 *  param radius: the distance of all points from the center of the sphere (0,0,0)
 *  param hIntersections: the number of points on a latitude
 *  param vIntersections: the number of points when going from the north to the south pole (excluding the poles)
 **/
std::shared_ptr<GeometryComponent> createSphereGeometry(float radius, int hIntersections, int vIntersections);

/**
 * Creates a geometry component with the information contained in a OFF file
 **/
std::shared_ptr<GeometryComponent> loadOffFile(const std::filesystem::path &filePath);
} // namespace Engine

#endif