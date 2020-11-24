#include <gtest/gtest.h>
#include <Core/Components/Geometry/geometry.h>

using namespace Engine;

TEST(GEOMETRY_TEST, createSphereGeometry_single_latitude) {
  GeometryComponent* geometry = createSphereGeometry(1.0f, 4, 1);  

  std::vector<Math::Vector3>& vertices{geometry->getVertices()};
  std::vector<unsigned int>& faces{geometry->getFaces()};

  EXPECT_EQ(vertices.size(), 6);
  EXPECT_EQ(faces.size(), 24);

  std::vector<Math::Vector3> expectedVertices{
    Math::Vector3{ 0.0f, 1.0f, 0.0f },
    Math::Vector3{ 0.0f, 0.0f, 1.0f },
    Math::Vector3{ 1.0f, 0.0f, 0.0f },
    Math::Vector3{ 0.0f, 0.0f,-1.0f },
    Math::Vector3{-1.0f, 0.0f, 0.0f },
    Math::Vector3{ 0.0f,-1.0f, 0.0f } 
  };

  std::vector<unsigned int> expectedFaces{
    1, 2, 0,
    2, 3, 0,
    3, 4, 0,
    4, 1, 0,

    5, 2, 1,
    5, 3, 2,
    5, 4, 3,
    5, 1, 4
  };

  EXPECT_EQ(faces, expectedFaces);

  for (int i = 0; i < 6; ++i) {
    MathLib::allClose(vertices[i], expectedVertices[i]);
  }

  delete geometry;
}

TEST(GEOMETRY_TEST, createSphereGeometry_multi_latitude) {
  GeometryComponent* geometry = createSphereGeometry(1.0f, 4, 2);  

  std::vector<Math::Vector3>& vertices{geometry->getVertices()};
  std::vector<unsigned int>& faces{geometry->getFaces()};

  EXPECT_EQ(vertices.size(), 10);
  EXPECT_EQ(faces.size(), 48);

  std::vector<Math::Vector3> expectedVertices{
    Math::Vector3{ 0.0f,              1.0f, 0.0f },
    Math::Vector3{ sin((1.0/3.0) * M_PI) * sin((0.0/4.0) * 2.0 * M_PI) , cos((1.0/3.0) * M_PI), sin((1.0/3.0) * M_PI) * cos((0.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((1.0/3.0) * M_PI) * sin((1.0/4.0) * 2.0 * M_PI) , cos((1.0/3.0) * M_PI), sin((1.0/3.0) * M_PI) * cos((1.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((1.0/3.0) * M_PI) * sin((2.0/4.0) * 2.0 * M_PI) , cos((1.0/3.0) * M_PI), sin((1.0/3.0) * M_PI) * cos((2.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((1.0/3.0) * M_PI) * sin((3.0/4.0) * 2.0 * M_PI) , cos((1.0/3.0) * M_PI), sin((1.0/3.0) * M_PI) * cos((3.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((2.0/3.0) * M_PI) * sin((0.0/4.0) * 2.0 * M_PI) , cos((2.0/3.0) * M_PI), sin((2.0/3.0) * M_PI) * cos((0.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((2.0/3.0) * M_PI) * sin((1.0/4.0) * 2.0 * M_PI) , cos((2.0/3.0) * M_PI), sin((2.0/3.0) * M_PI) * cos((1.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((2.0/3.0) * M_PI) * sin((2.0/4.0) * 2.0 * M_PI) , cos((2.0/3.0) * M_PI), sin((2.0/3.0) * M_PI) * cos((2.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ sin((2.0/3.0) * M_PI) * sin((3.0/4.0) * 2.0 * M_PI) , cos((2.0/3.0) * M_PI), sin((2.0/3.0) * M_PI) * cos((3.0/4.0) * 2.0 * M_PI)},
    Math::Vector3{ 0.0f,-1.0f, 0.0f } 
  };

  std::vector<unsigned int> expectedFaces{
    1, 2, 0,
    2, 3, 0,
    3, 4, 0,
    4, 1, 0,

    5, 6, 2,
    5, 2, 1,
    6, 7, 3,
    6, 3, 2,
    7, 8, 4,
    7, 4, 3,
    8, 5, 1,
    8, 1, 4,

    9, 6, 5,
    9, 7, 6,
    9, 8, 7,
    9, 5, 8
  };

  EXPECT_EQ(faces, expectedFaces);

  for (int i = 0; i < 10; ++i) {
    MathLib::allClose(vertices[i], expectedVertices[i]);
  }

  delete geometry;
}

TEST(GEOMETRY_TEST, calculate_normals_triangle) {
  GeometryComponent geometry{
    {
      Math::Vector3{-1.0, 0.0, 0.0 },
      Math::Vector3{ 1.0, 0.0, 0.0 },
      Math::Vector3{ 0.0, 1.0, 0.0 }
    },
    {
      0, 1, 2
    }
  };

  geometry.calculateNormals();

  std::vector<Math::Vector3>& normals{geometry.getNormals()};

  EXPECT_EQ(normals.size(), 3);

  std::vector<Math::Vector3> expectedNormals{
    Math::Vector3{0.0, 0.0, 1.0 },
    Math::Vector3{0.0, 0.0, 1.0 },
    Math::Vector3{0.0, 0.0, 1.0 }
  };

  for ( int i{ 0 }; i < 3; ++i) {
    EXPECT_TRUE(allClose(normals[i], expectedNormals[i]));
  }
}