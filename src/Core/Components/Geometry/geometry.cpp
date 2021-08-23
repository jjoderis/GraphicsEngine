#include "geometry.h"

#include "../../../Util/fileHandling.h"
#include <algorithm>

namespace filesystem = std::filesystem;

Engine::GeometryComponent::GeometryComponent() : m_vertices{}, m_faces{} {}
Engine::GeometryComponent::GeometryComponent(std::initializer_list<Math::Vector3> vertices,
                                             std::initializer_list<unsigned int> faces)
    : m_vertices{vertices}, m_faces{faces}
{
    calculateBoundingBox();
}
Engine::GeometryComponent::GeometryComponent(std::vector<Math::Vector3> &&vertices,
                                             std::vector<Math::Vector3> &&normals,
                                             std::vector<unsigned int> &&faces)
    : m_vertices{std::move(vertices)}, m_normals{std::move(normals)}, m_faces{std::move(faces)}
{
    calculateBoundingBox();
}

std::vector<Engine::Math::Vector3> &Engine::GeometryComponent::getVertices() { return m_vertices; }
const std::vector<Engine::Math::Vector3> &Engine::GeometryComponent::getVertices() const { return m_vertices; }

std::vector<unsigned int> &Engine::GeometryComponent::getFaces() { return m_faces; }

std::vector<Engine::Math::Vector3> &Engine::GeometryComponent::getNormals() { return m_normals; }

std::vector<Engine::Math::Vector2> &Engine::GeometryComponent::getTexCoords() { return m_texCoords; };

Engine::AccelerationStructure &Engine::GeometryComponent::getAccStructure() { return m_bounding; }

void Engine::GeometryComponent::addVertex(Math::Vector3 &&newVertex)
{
    m_vertices.emplace_back(newVertex);
    // to prevent errors with expected normals atm
    m_normals.emplace_back(Math::Vector3{0.0, 0.0, -1.0});
}

void Engine::GeometryComponent::addFace(unsigned int a, unsigned int b, unsigned int c)
{
    m_faces.emplace_back(a);
    m_faces.emplace_back(b);
    m_faces.emplace_back(c);
}

void Engine::GeometryComponent::calculateNormals()
{
    m_normals.clear();
    m_normals.resize(m_vertices.size(), Math::Vector3{0.0, 0.0, 0.0});

    // the normal of a vertex is the normalized average of the normals of its adjacent faces (keep track how many faces
    // contributed to a vertex)
    // TODO: maybe improve by weighing
    std::vector<unsigned int> normalContributions{};
    normalContributions.resize(m_vertices.size(), 0);

    // we asume that we have faces when we try to calculate the normals (and that these faces are representing triangles
    // => num of indices is divisible by 3)
    // TODO: maybe make this a bit more foolproof
    for (int i{0}; i < m_faces.size(); i += 3)
    {
        Math::Vector3 a{m_vertices[m_faces[i + 1]] - m_vertices[m_faces[i]]};
        Math::Vector3 b{m_vertices[m_faces[i + 2]] - m_vertices[m_faces[i]]};
        Math::Vector3 faceNormal = normalize(MathLib::cross(a, b));

        // add the faceNormal to each of the faces vertices
        m_normals[m_faces[i]] += faceNormal;
        m_normals[m_faces[i + 1]] += faceNormal;
        m_normals[m_faces[i + 2]] += faceNormal;

        normalContributions[m_faces[i]]++;
        normalContributions[m_faces[i + 1]]++;
        normalContributions[m_faces[i + 2]]++;
    }

    for (int i{0}; i < m_vertices.size(); ++i)
    {
        if (normalContributions[i])
        {
            // this vertex has adjacent faces => calculate vertex normal
            m_normals[i] / normalContributions[i];
            normalize(m_normals[i]);
        }
        else
        {
            // this vertex has no adjacent faces => just give some unit vector
            m_normals[i] = Math::Vector3{0.0, 0.0, 1.0};
        }
    }
}

void Engine::GeometryComponent::calculateBoundingBox()
{
    m_bounding = AccelerationStructure{nullptr, 0, m_vertices.size(), this};
}

std::shared_ptr<Engine::GeometryComponent>
Engine::createSphereGeometry(float radius, int hIntersections, int vIntersections)
{
    std::shared_ptr<GeometryComponent> geometry = std::make_shared<GeometryComponent>();
    std::vector<Math::Vector3> &vertices{geometry->getVertices()};
    std::vector<Math::Vector3> &normals{geometry->getNormals()};
    std::vector<unsigned int> &faces{geometry->getFaces()};

    // we want at least 4 points around the equator and on between the poles
    hIntersections = std::max(hIntersections, 4);
    vIntersections = std::max(vIntersections, 1);

    // add north pole
    vertices.resize(hIntersections * vIntersections + 2, Engine::Math::Vector3{});
    normals.resize(hIntersections * vIntersections + 2, Engine::Math::Vector3{});

    // each faces has 3 indices
    // we have 8 faces between 2 adjacent points on the latitutes closest to the poles and the poles themselves
    // we have an additional 2 faces between 2 adjacent points and two points of the next latitude for each additional
    // latitude
    faces.resize(3 * 2 * hIntersections * vIntersections);

    vertices[0] = Math::Vector3{0.0f, radius, 0.0f};
    normals[0] = Math::Vector3{0.0f, 1.0f, 0.0f};
    int vertexIndex{1};
    int faceIndex{0};

    double horizontalStep{(2.0f * M_PI) / hIntersections};
    double verticalStep{M_PI / (vIntersections + 1)};
    // create the points for the first latitude and the faces between them and the north pole
    double y{radius * cos(verticalStep)};
    double sinTheta{sin(verticalStep)};
    for (int j{0}; j < hIntersections; ++j)
    {
        vertices[vertexIndex] =
            Math::Vector3{radius * sinTheta * sin(j * horizontalStep), y, radius * sinTheta * cos(j * horizontalStep)};
        normals[vertexIndex] = vertices[vertexIndex] / radius;
        faces[faceIndex++] = vertexIndex;
        faces[faceIndex++] = (vertexIndex % hIntersections) + 1;
        faces[faceIndex++] = 0;
        ++vertexIndex;
    }

    for (int i{1}; i < vIntersections; ++i)
    {
        y = radius * cos((i + 1) * verticalStep);
        sinTheta = sin((i + 1) * verticalStep);
        int latitudeStartIndex{vertexIndex};
        for (int j{0}; j < hIntersections; ++j)
        {
            vertices[vertexIndex] = Math::Vector3{
                radius * sinTheta * sin(j * horizontalStep), y, radius * sinTheta * cos(j * horizontalStep)};
            normals[vertexIndex] = vertices[vertexIndex] / radius;
            faces[faceIndex++] = vertexIndex;
            faces[faceIndex++] = latitudeStartIndex + ((vertexIndex % hIntersections));
            faces[faceIndex++] = (latitudeStartIndex + ((vertexIndex % hIntersections))) - hIntersections;
            faces[faceIndex++] = vertexIndex;
            faces[faceIndex++] = (latitudeStartIndex + ((vertexIndex % hIntersections))) - hIntersections;
            faces[faceIndex++] = vertexIndex - hIntersections;
            ++vertexIndex;
        }
    }

    int maxVertexIndex{hIntersections * vIntersections + 1};
    vertices[maxVertexIndex] = Math::Vector3{0.0f, -radius, 0.0f};
    normals[maxVertexIndex] = Math::Vector3{0.0f, -1.0f, 0.0f};
    // go back to the first vertex in the last latitude
    vertexIndex -= hIntersections;
    int latitudeStartIndex{vertexIndex};
    // create the faces between the last latitude and the south pole
    for (int j{0}; j < hIntersections; ++j)
    {
        faces[faceIndex++] = maxVertexIndex;
        faces[faceIndex++] = (latitudeStartIndex + ((vertexIndex % hIntersections)));
        faces[faceIndex++] = vertexIndex;
        ++vertexIndex;
    }

    geometry->calculateBoundingBox();

    return geometry;
}

std::shared_ptr<Engine::GeometryComponent> Engine::loadOffFile(const filesystem::path &filePath)
{
    std::istringstream stream{Util::readTextFile(filePath.c_str())};

    std::string line;
    std::getline(stream, line);

    // check if the vertex information contains
    bool hasNormals{false};
    if (line.at(0) == 'N')
    {
        hasNormals = true;
    }

    std::getline(stream, line);

    std::istringstream iss(line);
    int numVertices, numFaces;
    iss >> numVertices >> numFaces;
    std::vector<Math::Vector3> vertices;
    vertices.reserve(numVertices);

    std::vector<Math::Vector3> normals;
    if (hasNormals)
    {
        normals.reserve(numVertices);

        for (int i{0}; i < numVertices; ++i)
        {
            float x, y, z, nx, ny, nz;

            std::getline(stream, line);
            std::istringstream iss(line);

            iss >> x >> y >> z >> nx >> ny >> nz;
            vertices.emplace_back(x, y, z);
            normals.emplace_back(nx, ny, nz);
        }
    }
    else
    {
        normals.resize(numVertices, Math::Vector3{0.0, 0.0, -1.0});

        for (int i{0}; i < numVertices; ++i)
        {
            float x, y, z;

            std::getline(stream, line);
            std::istringstream iss(line);

            iss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }
    }

    std::vector<unsigned int> faces;
    faces.reserve(numFaces * 3);

    for (int i{0}; i < numFaces; ++i)
    {
        int tmp, a, b, c;

        std::getline(stream, line);
        std::istringstream iss(line);

        iss >> tmp >> a >> b >> c;

        faces.emplace_back(a);
        faces.emplace_back(b);
        faces.emplace_back(c);
    }

    return std::make_shared<GeometryComponent>(std::move(vertices), std::move(normals), std::move(faces));
}

Engine::AccelerationStructure::AccelerationStructure() {}

Engine::AccelerationStructure::AccelerationStructure(AccelerationStructure *parent,
                                                     int start,
                                                     int end,
                                                     GeometryComponent *geometry)
    : m_parent{parent}, m_startIndex{start}, m_endIndex{end}, m_geometry{geometry}
{
    m_min = Math::Vector3{std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity()};
    m_max = Math::Vector3{-std::numeric_limits<float>::infinity(),
                          -std::numeric_limits<float>::infinity(),
                          -std::numeric_limits<float>::infinity()};

    auto &vertices{geometry->getVertices()};

    for (auto &vertex : vertices)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (vertex.at(i) < m_min.at(i))
            {
                m_min.at(i) = vertex.at(i);
            }
            if (vertex.at(i) > m_max.at(i))
            {
                m_max.at(i) = vertex.at(i);
            }
        }
    }

    m_min -= 0.01;
    m_max += 0.01;

    std::vector<int> vertexRef(m_endIndex);
    for (int i = 0; i < m_endIndex; ++i)
    {
        vertexRef[i] = i;
    }

    subdivide(vertexRef);

    // // TODO: reorganize the vertices in the geometry based on the vertexRef (sort normals and uv coordinates as well,
    // change faces to reference correct new indices)
    std::vector<Engine::Math::Vector3> tmp3(m_endIndex);

    for (int i = 0; i < m_endIndex; ++i)
    {
        tmp3[i] = vertices[vertexRef[i]];
    }

    vertices.swap(tmp3);

    auto &normals{geometry->getNormals()};
    if (normals.size() == m_endIndex)
    {
        for (int i = 0; i < m_endIndex; ++i)
        {
            tmp3[i] = normals[vertexRef[i]];
        }

        normals.swap(tmp3);
    }

    auto &uv{geometry->getTexCoords()};
    if (uv.size() == m_endIndex)
    {
        std::vector<Engine::Math::Vector2> tmp2(m_endIndex);
        for (int i = 0; i < m_endIndex; ++i)
        {
            tmp2[i] = uv[vertexRef[i]];
        }
        uv.swap(tmp2);
    }

    std::vector<int> reverseRef(m_endIndex);

    auto &faces{geometry->getFaces()};

    if (faces.size())
    {
        // create a reference to use to update the faces
        for (int i = 0; i < m_endIndex; ++i)
        {
            reverseRef[vertexRef[i]] = i;
        }

        // update faces
        for (auto &index : faces)
        {
            index = reverseRef[index];
        }

        // assign triangles to bounding boxes
        if (faces.size() % 3 == 0)
        {
            for (int i = 0; i < faces.size(); i += 3)
            {
                // calculate triangles bounding box
                Engine::Math::Vector3 min{std::numeric_limits<float>::infinity(),
                                          std::numeric_limits<float>::infinity(),
                                          std::numeric_limits<float>::infinity()};
                Engine::Math::Vector3 max{-std::numeric_limits<float>::infinity(),
                                          -std::numeric_limits<float>::infinity(),
                                          -std::numeric_limits<float>::infinity()};

                for (int corner = 0; corner < 3; ++corner)
                {
                    Engine::Math::Vector3 &vertex{vertices[faces[i + corner]]};

                    for (int axis = 0; axis < 3; ++axis)
                    {
                        min.at(axis) = (vertex.at(axis) < min.at(axis)) ? vertex.at(axis) : min.at(axis);
                        max.at(axis) = (vertex.at(axis) > max.at(axis)) ? vertex.at(axis) : max.at(axis);
                    }
                }

                assignTriangle(i, min, max);
            }
        }
    }
}

bool Engine::AccelerationStructure::assignTriangle(int index,
                                                   const Engine::Math::Vector3 &min,
                                                   const Engine::Math::Vector3 &max)
{
    // early exit if the bounding box of the triangle doesn't fit into this hierarchy element
    for (int i = 0; i < 3; ++i)
    {
        if (min.at(i) < m_min.at(i) || max.at(i) > m_max.at(i))
        {
            return false;
        }
    }

    // see if the triangle fits into one of the children
    for (auto &child : m_children)
    {
        if (child.assignTriangle(index, min, max))
        {
            return true;
        }
    }

    // place into this element if no child can take the triangle
    m_triangles.emplace_back(index);

    return true;
}

Engine::AccelerationStructure &Engine::AccelerationStructure::operator=(const AccelerationStructure &other)
{
    if (&other == this)
    {
        return *this;
    }

    m_parent = other.m_parent;
    m_min = other.m_min;
    m_max = other.m_max;
    m_startIndex = other.m_startIndex;
    m_endIndex = other.m_endIndex;
    m_triangles = other.m_triangles;
    m_children = other.m_children;
    m_geometry = other.m_geometry;
}

void Engine::AccelerationStructure::subdivide(std::vector<int> &vertexRef)
{
    int numVertices{m_endIndex - m_startIndex};
    // don't subdivide if there are no more than the maximum allowed vertices inside this object
    if (numVertices <= m_maxVertices)
    {
        return;
    }

    // get the side with the biggest length
    int maxIndex{-1};
    float maxLength{0};
    for (int i = 0; i < 3; ++i)
    {
        float length{abs(m_max.at(i) - m_min.at(i))};
        if (length > maxLength)
        {
            maxIndex = i;
            maxLength = length;
        }
    }

    auto &vertices{m_geometry->getVertices()};

    // sort along the longest side
    std::sort(vertexRef.begin() + m_startIndex,
              vertexRef.begin() + m_endIndex,
              [&vertices, maxIndex](int a, int b) { return vertices[a].at(maxIndex) < vertices[b].at(maxIndex); });

    // subdivide into two new boxes along the longest side
    int middle = m_startIndex + (numVertices / 2);
    Engine::Math::Vector3 &middleVertex{vertices[vertexRef[middle]]};

    Engine::Math::Vector3 middleMin{m_min};
    middleMin.at(maxIndex) = middleVertex.at(maxIndex);
    Engine::Math::Vector3 middleMax{m_max};
    middleMax.at(maxIndex) = middleVertex.at(maxIndex);

    // TODO: create child Structures with default constructor set their class members and call their subdivide from here
    AccelerationStructure a{};
    a.m_parent = this;
    a.m_min = m_min;
    a.m_max = middleMax;
    a.m_startIndex = m_startIndex;
    a.m_endIndex = middle;
    a.m_geometry = m_geometry;
    a.subdivide(vertexRef);

    AccelerationStructure b{};
    b.m_parent = this;
    b.m_min = middleMin;
    b.m_max = m_max;
    b.m_startIndex = middle;
    b.m_endIndex = m_endIndex;
    b.m_geometry = m_geometry;
    b.subdivide(vertexRef);

    m_children.emplace_back(a);
    m_children.emplace_back(b);
}

Engine::Math::Vector3 &Engine::AccelerationStructure::getMin() { return m_min; }
Engine::Math::Vector3 &Engine::AccelerationStructure::getMax() { return m_max; }
std::vector<int> &Engine::AccelerationStructure::getTriangles() { return m_triangles; }
std::vector<Engine::AccelerationStructure> &Engine::AccelerationStructure::getChildren() { return m_children; }
