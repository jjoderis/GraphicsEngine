#include "geometry.h"

Engine::GeometryComponent::GeometryComponent() : m_vertices{}, m_faces{} {}
Engine::GeometryComponent::GeometryComponent(std::initializer_list<Math::Vector3> vertices, std::initializer_list<unsigned int> faces)
    : m_vertices{ vertices }, m_faces{ faces } {}

std::vector<Engine::Math::Vector3>& Engine::GeometryComponent::getVertices() {
    return m_vertices;
}

std::vector<unsigned int>& Engine::GeometryComponent::getFaces() {
    return m_faces;
}

std::vector<Engine::Math::Vector3>& Engine::GeometryComponent::getNormals() {
    return m_normals;
}

void Engine::GeometryComponent::addVertex(Math::Vector3 && newVertex) {
    m_vertices.emplace_back(newVertex);
}

void Engine::GeometryComponent::addFace(unsigned int a, unsigned int b, unsigned int c) {
    m_faces.emplace_back(a);
    m_faces.emplace_back(b);
    m_faces.emplace_back(c);
}

void Engine::GeometryComponent::calculateNormals() {
    m_normals.resize(m_vertices.size(), Math::Vector3{ 0.0, 0.0, 0.0 });

    // the normal of a vertex is the normalized average of the normals of its adjacent faces (keep track how many faces contributed to a vertex)
    // TODO: maybe improve by weighing 
    std::vector<unsigned int> normalContributions{};
    normalContributions.resize(m_vertices.size(), 0);
    
    // we asume that we have faces when we try to calculate the normals (and that these faces are representing triangles => num of indices is divisible by 3)
    // TODO: maybe make this a bit more foolproof
    for (int i{0}; i < m_faces.size(); i += 3) {
        Math::Vector3 a{ m_vertices[m_faces[i+1]] - m_vertices[m_faces[i]] };
        Math::Vector3 b{ m_vertices[m_faces[i+2]] - m_vertices[m_faces[i]] };
        Math::Vector3 faceNormal = MathLib::cross(a, b).normalize();

        // add the faceNormal to each of the faces vertices
        m_normals[m_faces[i]] += faceNormal;
        m_normals[m_faces[i+1]] += faceNormal;
        m_normals[m_faces[i+2]] += faceNormal;

        normalContributions[m_faces[i]]++;
        normalContributions[m_faces[i+1]]++;
        normalContributions[m_faces[i+2]]++;
    }

    for (int i{0}; i < m_vertices.size(); ++i) {
        if (normalContributions[i]) {
            // this vertex has adjacent faces => calculate vertex normal
            m_normals[i] / normalContributions[i];
            m_normals[i].normalize();
        } else {
            // this vertex has no adjacent faces => just give some unit vector
            m_normals[i] = Math::Vector3{0.0, 0.0, 1.0};
        }
    }
}

Engine::GeometryComponent* Engine::createSphereGeometry(float radius, int hIntersections, int vIntersections) {
    GeometryComponent* geometry = new GeometryComponent{};
    std::vector<Math::Vector3>& vertices{geometry->getVertices()};
    std::vector<Math::Vector3>& normals{geometry->getNormals()};
    std::vector<unsigned int>& faces{geometry->getFaces()};

    // we want at least 4 points around the equator and on between the poles 
    hIntersections = std::max(hIntersections, 4);
    vIntersections = std::max(vIntersections, 1);

    // add north pole
    vertices.resize(hIntersections * vIntersections + 2, Engine::Math::Vector3{});
    normals.resize(hIntersections * vIntersections + 2, Engine::Math::Vector3{});

    // each faces has 3 indices
    // we have 8 faces between 2 adjacent points on the latitutes closest to the poles and the poles themselves
    // we have an additional 2 faces between 2 adjacent points and two points of the next latitude for each additional latitude
    faces.resize( 3 * 2 * hIntersections * vIntersections );

    vertices[0] = Math::Vector3{ 0.0f, radius, 0.0f };
    normals[0] = Math::Vector3{ 0.0f, 1.0f, 0.0f };
    int vertexIndex{1};
    int faceIndex{0};

    double horizontalStep{ (2.0f * M_PI) / hIntersections };
    double verticalStep{M_PI / (vIntersections + 1)};
    // create the points for the first latitude and the faces between them and the north pole
    double y{radius * cos(verticalStep)};
    double sinTheta{sin(verticalStep)};
    for(int j{0}; j < hIntersections; ++j) {
        vertices[vertexIndex] = Math::Vector3{radius * sinTheta * sin(j * horizontalStep), y, radius * sinTheta * cos(j * horizontalStep)};
        normals[vertexIndex] = vertices[vertexIndex] / radius;
        faces[faceIndex++] = vertexIndex;
        faces[faceIndex++] = (vertexIndex % hIntersections) + 1;
        faces[faceIndex++] = 0;
        ++vertexIndex;
    }

    for (int i{ 1 }; i < vIntersections; ++i) {
        y = radius * cos((i + 1) * verticalStep);
        sinTheta = sin((i + 1) * verticalStep);
        int latitudeStartIndex{ vertexIndex };
        for (int j{0}; j < hIntersections; ++j) {
            vertices[vertexIndex] = Math::Vector3{radius * sinTheta * sin(j * horizontalStep), y, radius * sinTheta * cos(j * horizontalStep)};
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
    vertices[maxVertexIndex] = Math::Vector3{ 0.0f, -radius, 0.0f };
    normals[maxVertexIndex] = Math::Vector3{ 0.0f, -1.0f, 0.0f };
    // go back to the first vertex in the last latitude
    vertexIndex -= hIntersections;
    int latitudeStartIndex{vertexIndex};
    // create the faces between the last latitude and the south pole
    for(int j{0}; j < hIntersections; ++j) {
        faces[faceIndex++] = maxVertexIndex;
        faces[faceIndex++] = (latitudeStartIndex + ((vertexIndex % hIntersections)));
        faces[faceIndex++] = vertexIndex;
        ++vertexIndex;
    }


    return geometry;
}