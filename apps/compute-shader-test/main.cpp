// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "./openGL/openGL.h"
#include "./window/window.h"

#include "./helpers/shaderHelper.h"

#include <iostream>
#include <random>

#include "./helpers/textureHelper.h"
#include <OpenGL/Util/textureLoader.h>
#include <stb_image_write.h>

#include "./imageFilter.h"

#include <Core/Math/math.h>

#include <chrono>
#include <cstring>
#include <random>
#include <vector>

#include <string>

#include <chrono>
#include <thread>

#include <algorithm>

struct Material
{
    int type;
    Engine::Vector3 albedo;
    float fuzz;
    float ir;

    Material(int t, const Engine::Vector3 &a, float f, float i)
    {
        type = t;
        albedo = a;
        fuzz = f;
        ir = i;
    }
};

constexpr size_t materialSize{sizeof(int) + 11 * sizeof(float)};

void addMaterial(char *data, const Material &material)
{
    std::memcpy(data, &material.type, sizeof(int));
    std::memcpy(data + 4 * sizeof(float), material.albedo.data(), 3 * sizeof(float));
    std::memcpy(data + 7 * sizeof(float), &material.fuzz, sizeof(float));
    std::memcpy(data + 8 * sizeof(float), &material.ir, sizeof(float));
}

struct Sphere
{
    Engine::Point3 center;
    float radius;
    int mat;

    Sphere(const Engine::Point3 &c, float r, int m)
    {
        center = c;
        radius = r;
        mat = m;
    }
};

constexpr size_t sphereSize{7 * sizeof(float) + sizeof(int)};

void addSphere(char *data, const Sphere &sphere)
{
    std::memcpy(data, sphere.center.data(), 3 * sizeof(float));
    std::memcpy(data + 3 * sizeof(float), &sphere.radius, sizeof(float));
    std::memcpy(data + 4 * sizeof(float), &sphere.mat, sizeof(int));
}

// (int) start, (int) end
// (int) leftType, (int) leftIndex, (int) rightType, (int) rightIndex
// (float) minX, (float) maxX
// (float) minY, (float) maxY
// (float) minZ, (float) maxZ
constexpr size_t boxSize{6 * sizeof(int) + 6 * sizeof(float)};

int addBox(
    char *data, char *sphereData, std::vector<int> &sphereIndices, int startSphere, int numSpheres, int numPrevBoxes)
{
    Engine::Point3 min = Engine::Point3{std::numeric_limits<float>::infinity(),
                                        std::numeric_limits<float>::infinity(),
                                        std::numeric_limits<float>::infinity()};
    Engine::Point3 max = Engine::Point3{-std::numeric_limits<float>::infinity(),
                                        -std::numeric_limits<float>::infinity(),
                                        -std::numeric_limits<float>::infinity()};

    for (int i{startSphere}; i < startSphere + numSpheres; ++i)
    {
        int sphereIndex{sphereIndices[i]};
        char *sphere{sphereData + sphereIndex * sphereSize};
        Engine::Point3 vertex{
            *((float *)sphere), *((float *)(sphere + sizeof(float))), *((float *)(sphere + 2 * sizeof(float)))};

        float radius{*((float *)(sphere + 3 * sizeof(float)))};

        for (int i = 0; i < 3; ++i)
        {
            if ((vertex.at(i) - radius) < min.at(i))
            {
                min.at(i) = vertex.at(i) - radius;
            }
            if ((vertex.at(i) + radius) > max.at(i))
            {
                max.at(i) = vertex.at(i) + radius;
            }
        }
    }

    min -= 0.01;
    max += 0.01;

    float *boundaries{(float *)(data + 6 * sizeof(int))};

    boundaries[0] = min(0);
    boundaries[1] = max(0);
    boundaries[2] = min(1);
    boundaries[3] = max(1);
    boundaries[4] = min(2);
    boundaries[5] = max(2);

    int maxIndex{-1};
    float maxLength{0};
    for (int i = 0; i < 3; ++i)
    {
        float length{abs(max.at(i) - min.at(i))};
        if (length > maxLength)
        {
            maxIndex = i;
            maxLength = length;
        }
    }

    std::sort(sphereIndices.begin() + startSphere,
              sphereIndices.begin() + startSphere + numSpheres,
              [maxIndex, sphereData](int a, int b)
              {
                  Engine::Point3 vertexA{*((float *)(sphereData + a * sphereSize)),
                                         *((float *)(sphereData + a * sphereSize + sizeof(float))),
                                         *((float *)(sphereData + a * sphereSize + 2 * sizeof(float)))};
                  Engine::Point3 vertexB{*((float *)(sphereData + b * sphereSize)),
                                         *((float *)(sphereData + b * sphereSize + sizeof(float))),
                                         *((float *)(sphereData + b * sphereSize + 2 * sizeof(float)))};
                  return vertexA.at(maxIndex) < vertexB.at(maxIndex);
              });

    int *children{(int *)(data + 2 * sizeof(int))};
    int *start{(int *)data};

    *start = numPrevBoxes;

    int *end{(int *)(data + sizeof(int))};

    if (numSpheres == 1)
    {
        children[0] = 0;
        children[1] = sphereIndices[startSphere];
        children[2] = -1;

        *end = numPrevBoxes + 1;

        return 1;
    }
    else if (numSpheres == 2)
    {
        children[0] = 0;
        children[1] = sphereIndices[startSphere];
        children[2] = 0;
        children[3] = sphereIndices[startSphere + 1];

        *end = numPrevBoxes + 1;

        return 1;
    }
    else
    {
        children[0] = 1;

        int numLeftBoxes =
            addBox(data + boxSize, sphereData, sphereIndices, startSphere, numSpheres / 2, numPrevBoxes + 1);

        children[2] = 1;
        children[3] = numPrevBoxes + numLeftBoxes + 1;

        int numRigthBoxes = addBox(data + boxSize * (numLeftBoxes + 1),
                                   sphereData,
                                   sphereIndices,
                                   startSphere + (numSpheres / 2),
                                   (numSpheres / 2) + (numSpheres % 2),
                                   numPrevBoxes + numLeftBoxes + 1);

        *end = numPrevBoxes + numLeftBoxes + numRigthBoxes + 1;

        return numLeftBoxes + numRigthBoxes + 1;
    }
}

int main()
{
    initWindow();
    initOpenGL();

    initVisualizer();

    auto window = getWindow();

    auto program{createComputeProgram(
        "#version 440 core\n"
        "layout (local_size_x = 1000) in;"
        "layout (rgba32f, binding = 0) uniform image2D outputTexture;"

        "const float infinity = 1.0 / 0.0;"
        "const float pi = 3.1415926535897932385;"

        "uniform int startX;"
        "uniform int startY;"
        "uniform int width;"
        "uniform int height;"

        "// RNG from GPU Gems 3: "
        "https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/"
        "chapter-37-efficient-random-number-generation-and-application \n"

        "uint z1, z2, z3, z4;"

        "uint TausStep(inout uint z, int S1, int S2, int S3, uint M) {"
        "   uint b=(((z << S1) ^ z) >> S2);"
        "   return z = (((z & M) << S3) ^ b);"
        "}"

        "uint LCGStep(inout uint z, uint A, uint C) {"
        "   return z=(A*z+C);"
        "}"

        "float HybridTaus() {"
        "   uint a1 = TausStep(z1, 13, 19, 12, 4294967294);"
        "   uint a2 = TausStep(z2, 2, 25, 4, 4294967288);"
        "   uint a3 = TausStep(z3, 3, 11, 17, 4294967280);"
        "   uint a4 = LCGStep(z4, 1664525, 1013904223);"
        "   return 2.3283064365387e-10 * float(a1 ^ a2 ^ a3 ^ a4);"
        "}"

        "float randomNumber() {"
        "   return HybridTaus();"
        "}"

        "float randomNumber(float min, float max) {"
        "   return min + (max-min)*randomNumber();"
        "}"

        "vec3 randomVec3() {"
        "   return vec3(randomNumber(), randomNumber(), randomNumber());"
        "}"

        "vec3 randomVec3(float min, float max) {"
        "   return vec3(randomNumber(min, max), randomNumber(min, max), randomNumber(min, max));"
        "}"

        "vec3 randomInUnitSphere() {"
        "   while (true) {"
        "       vec3 p = randomVec3(-1,1);"
        "       if (dot(p, p) >= 1) continue;"
        "       return p;"
        "   }"
        "}"

        "vec3 randomInHemisphere(vec3 normal) {"
        "   vec3 inUnitSphere = randomInUnitSphere();"
        "   if (dot(inUnitSphere, normal) > 0.0) {"
        "       return inUnitSphere;"
        "   } else {"
        "       return -inUnitSphere;"
        "   }"
        "}"

        "vec3 randomUnitVector() {"
        "   return normalize(randomInUnitSphere());"
        "}"

        "vec3 randomInUnitDisk() {"
        "   while (true) {"
        "       vec3 p = vec3(randomNumber(-1,1), randomNumber(-1,1), 0);"
        "       if (dot(p, p) >= 1) continue;"
        "       return p;"
        "   }"
        "}"

        "bool nearZero(vec3 v) {"
        "   float s = 1e-8;"
        "   return (abs(v.x) < s && abs(v.y) < s && abs(v.z) < s);"
        "}"

        "float degreesToRadians(float degrees) {"
        "   return degrees * pi / 180.0;"
        "}"

        "struct Camera{"
        "   vec3 origin;"
        "   vec3 lowerLeftCorner;"
        "   vec3 horizontal;"
        "   vec3 vertical;"
        "   vec3 u, v, w;"
        "   float lensRadius;"
        "};"

        "layout(std430, binding = 0) buffer SceneCamera {"
        "   Camera sceneCamera;"
        "};"

        "struct Ray {"
        "   vec3 origin;"
        "   vec3 direction;"
        "};"

        "float reflectance(float cosine, float refIdx) {"
        "   float r0 = (1-refIdx) / (1+refIdx);"
        "   r0 = r0*r0;"
        "   return r0 + (1-r0)*pow((1 - cosine),5);"
        "}"

        "Ray getRay(Camera c, float s, float t) {"
        "   vec3 rd = c.lensRadius * randomInUnitDisk();"
        "   vec3 offset = c.u * rd.x + c.v * rd.y;"

        "   return Ray("
        "   c.origin + offset,"
        "   c.lowerLeftCorner + s*c.horizontal + t*c.vertical - c.origin - offset"
        "   );"
        "}"

        "vec3 at(Ray r, float t) {"
        "   return r.origin + t * r.direction;"
        "}"

        "struct Material {"
        "   int type;"
        "   vec3 albedo;"
        "   float fuzz;"
        "   float ir;"
        "};"

        "layout(std430, binding = 1) buffer SceneMaterials {"
        "   Material sceneMaterials[];"
        "};"

        "struct HitRecord {"
        "   vec3 p;"
        "   vec3 normal;"
        "   Material mat;"
        "   float t;"
        "   bool frontFace;"
        "};"

        "void setFaceNormal(inout HitRecord rec, Ray r, vec3 outwardNormal) {"
        "   rec.frontFace = dot(r.direction, outwardNormal) < 0;"
        "   rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;"
        "}"

        "struct Sphere {"
        "   vec3 center;"
        "   float radius;"
        "   int mat;"
        "};"

        "layout(std430, binding = 2) buffer SceneSpheres {"
        "   Sphere spheres[];"
        "};"

        "struct Hittable {"
        "   int type;"
        "   int index;"
        "};"

        "layout(std430, binding = 3) buffer Hittables {"
        "   Hittable hittables[];"
        "};"

        "struct BoundingBox {"
        "   int start;"
        "   int end;"
        "   int leftType;"
        "   int leftIndex;"
        "   int rightType;"
        "   int rightIndex;"
        "   float minX;"
        "   float maxX;"
        "   float minY;"
        "   float maxY;"
        "   float minZ;"
        "   float maxZ;"
        "};"

        "layout(std430, binding = 4) buffer BoundingBoxes {"
        "   BoundingBox boundingBoxes[];"
        "};"

        "bool scatter(Material mat, Ray rIn, HitRecord rec, out vec3 attenuation, out Ray scattered) {"
        // lambertian
        "   if (mat.type == 0) {"
        "       vec3 scatterDirection = vec3(rec.normal + randomUnitVector());"

        "       if (nearZero(scatterDirection)) {"
        "           scatterDirection = rec.normal;"
        "       }"

        "       scattered = Ray(rec.p, scatterDirection);"
        "       attenuation = mat.albedo;"
        "       return true;"
        "       } else if (mat.type == 1) {"
        // metal
        "       vec3 reflected = reflect(normalize(rIn.direction), rec.normal);"
        "       scattered = Ray(rec.p, reflected + mat.fuzz * randomInUnitSphere());"
        "       attenuation = mat.albedo;"
        "       return (dot(scattered.direction, rec.normal) > 0);"
        "   } else if (mat.type == 2) {"
        // dielectric
        "       attenuation = vec3(1.0, 1.0, 1.0);"
        "       float refractionRatio = rec.frontFace ? (1.0/mat.ir) : mat.ir;"

        "       vec3 unitDirection = normalize(rIn.direction);"
        "       float cosTheta = min(dot(-unitDirection, rec.normal), 1.0);"
        "       float sinTheta = sqrt(1.0 - cosTheta*cosTheta);"

        "       bool cannotRefract = refractionRatio * sinTheta > 1.0;"
        "       vec3 direction = vec3(0);"

        "       if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomNumber()) {"
        "           direction = reflect(unitDirection, rec.normal);"
        "       } else {"
        "           direction = refract(unitDirection, rec.normal, refractionRatio);"
        "       }"

        "       scattered = Ray(rec.p, direction);"
        "       return true;"
        "   } else {"
        "       return false;"
        "   }"
        "}"

        "vec3 emit(Material mat) {"
        "   if (mat.type == 3) {"
        "       return mat.albedo;"
        "   }"

        "   return vec3(0,0,0);"
        "}"

        "bool hit(Sphere s, Ray r, float t_min, float t_max, inout HitRecord rec) {"
        "   vec3 oc = r.origin - s.center;"
        "   float a = dot(r.direction, r.direction);"
        "   float half_b = dot(oc, r.direction);"
        "   float c = dot(oc, oc) - s.radius*s.radius;"
        "   float discriminant = half_b*half_b - a*c;"
        "   if (discriminant < 0.0f) return false;"
        "   float sqrtd = sqrt(discriminant);"
        "   float root = (-half_b - sqrtd) / a;"
        "   if (root < t_min || t_max < root) {"
        "       root = (-half_b + sqrtd) / a;"
        "       if (root < t_min || t_max < root) return false;"
        "   }"
        "   rec.t = root;"
        "   rec.p = at(r, root);"
        "   vec3 outwardNormal = (rec.p - s.center) / s.radius;"
        "   setFaceNormal(rec, r, outwardNormal);"
        "   rec.mat = sceneMaterials[s.mat];\n"
        "   return true;"
        "}"

        "bool hit(BoundingBox box, Ray r, float t_min, float t_max) {"
        "   vec3 min = vec3(box.minX, box.minY, box.minZ);"
        "   vec3 max = vec3(box.maxX, box.maxY, box.maxZ);"
        "   for (int a = 0; a < 3; ++a) {"
        "       float invD = 1.0 / r.direction[a];"
        "       float t0 = (min[a] - r.origin[a]) * invD;"
        "       float t1 = (max[a] - r.origin[a]) * invD;"
        "       if (invD < 0.0) {"
        "           float tmp = t0;"
        "           t0 = t1;"
        "           t1 = tmp;"
        "       }"
        "       t_min = (t0 > t_min) ? t0 : t_min;"
        "       t_max = (t1 < t_max) ? t1 : t_max;"
        "       if (t_max <= t_min) return false;"
        "   }"
        "   return true;"
        "}"

        "bool hit(BoundingBox box, Ray r, float t_min, float t_max, inout HitRecord rec) {"
        "   int end = box.end;"

        "   HitRecord tempRec = HitRecord(vec3(0,0,0), vec3(0,0,0), Material(0, vec3(0,0,0), 0.0, 0.0), 0, false);"
        "   bool hitAnything = false;"
        "   float closestSoFar = t_max;"

        "   for (int i = box.start; i < end; ++ i) {"
        "       BoundingBox curr = boundingBoxes[i];"

        "       if(!hit(curr, r, t_min, t_max)) {"
        "           i = curr.end - 1;"
        "           continue;"
        "       }"

        "       if (curr.leftType == 0) {"
        "           if (hit(spheres[curr.leftIndex], r, t_min, closestSoFar, tempRec)) {"
        "               hitAnything = true;"
        "               closestSoFar = tempRec.t;"
        "               rec = tempRec;"
        "           }"
        "       }"
        "       if (curr.rightType == 0) {"
        "           if (hit(spheres[curr.rightIndex], r, t_min, closestSoFar, tempRec)) {"
        "               hitAnything = true;"
        "               closestSoFar = tempRec.t;"
        "               rec = tempRec;"
        "           }"
        "       }"
        "   }"

        "   return hitAnything;"
        "}"

        "bool hit(Hittable object, Ray r, float t_min, float t_max, inout HitRecord rec) {"
        "   if (object.type == 0) {"
        "       return hit(spheres[object.index], r, t_min, t_max, rec);"
        "   } else if (object.type == 1) {"
        "       return hit(boundingBoxes[object.index], r, t_min, t_max, rec);"
        "   }"
        "   return false;"
        "}"

        "bool hitWorld(Ray r, float t_min, float t_max, inout HitRecord rec) {"
        "   HitRecord tempRec = HitRecord(vec3(0,0,0), vec3(0,0,0), Material(0, vec3(0,0,0), 0.0, 0.0), 0, false);"
        "   bool hitAnything = false;"
        "   float closestSoFar = t_max;"
        "   for (int i = 0; i < hittables.length(); ++i) {"
        "       if (hit(hittables[i], r, t_min, closestSoFar, tempRec)) {"
        "           hitAnything = true;"
        "           closestSoFar = tempRec.t;"
        "           rec = tempRec;"
        "       }"
        "   }"
        "   return hitAnything;"
        "}"

        "vec3 rayColor(Ray r) {"
        "   HitRecord rec = HitRecord(vec3(0,0,0), vec3(0,0,0), Material(0, vec3(0,0,0), 0.0, 0.0), 0, false);"

        "   vec3 currentColor = vec3(1, 1, 1);"
        "   Ray nextRay = Ray(vec3(0,0,0), vec3(0,0,0));"

        "   for(int i = 0; i < 50; ++i) {"
        "       if (hitWorld(r, 0.001, infinity, rec)) {"
        "           vec3 attenuation = vec3(0,0,0);"
        "           vec3 emitted = emit(rec.mat);"
        "           if (scatter(rec.mat, r, rec, attenuation, nextRay)) {"
        "               currentColor = currentColor * attenuation + emitted;"
        "               r = nextRay;"
        "           } else {"
        "               return emitted * currentColor;"
        "           }"
        "       } else {"
        "           return vec3(0,0,0);"
        "           vec3 unitDirection = normalize(r.direction);"
        "           float t = 0.5*(unitDirection.y + 1.0);"
        "           return currentColor * ((1.0-t)*vec3(0.1, 0.1, 0.1) + t*vec3(0.05, 0.07, .1));"
        "       }"
        "   }"

        "   return vec3(0,0,0);"
        "}"

        "shared uvec3 accColor;"

        "void main(void){"
        "   if (uvec3(gl_LocalInvocationID).x == 0) {"
        "       accColor = uvec3(0,0,0);"
        "   }"
        "   barrier();"

        "   uint test = 100;"
        "   uvec2 global = uvec2(gl_GlobalInvocationID) + uvec2(startX, startY);"
        "   z1 = LCGStep(test, global.x, global.y);\n"
        "   z2 = LCGStep(test, global.x, global.y);\n"
        "   z3 = LCGStep(test, global.x, global.y);\n"
        "   z4 = LCGStep(test, global.x, global.y);\n"

        "   ivec2 pixel = ivec2(gl_WorkGroupID.xy) + ivec2(startX, startY);"
        "   float u = float(pixel.x + randomNumber()) / (width - 1);"
        "   float v = float(pixel.y + randomNumber()) / (height - 1);"
        "   Ray r = getRay(sceneCamera, u, v);"
        "   vec3 color = rayColor(r);"
        "   uvec3 uColor = uvec3(256 * color);"
        "   atomicAdd(accColor.x, uColor.x);"
        "   atomicAdd(accColor.y, uColor.y);"
        "   atomicAdd(accColor.z, uColor.z);"
        "   barrier();"
        "   if (uvec3(gl_LocalInvocationID).z == 0) {"
        "       color = vec3(accColor) / (256 * uvec3(gl_WorkGroupSize).x);"
        "       imageStore(outputTexture, pixel, vec4(sqrt(color.x), sqrt(color.y), sqrt(color.z), 1.0));"
        "   }"
        "}")};

    // Image
    const float aspectRatio{16.0 / 9.0};
    const int width{1920};
    const int height{static_cast<int>(width / aspectRatio)};

    auto outTexture{createOutputTexture(width, height)};

    glUseProgram(program);

    struct Camera
    {
        Engine::Point3 origin;
        Engine::Point3 lowerLeftCorner;
        Engine::Vector3 horizontal;
        Engine::Vector3 vertical;
        Engine::Vector3 u;
        Engine::Vector3 v;
        Engine::Vector3 w;
        float lensRadius;

        Camera(const Engine::Point3 &lookfrom,
               const Engine::Point3 &lookat,
               const Engine::Vector3 &vup,
               float vFov,
               float aspectRatio,
               float aperture,
               float focusDist)
        {
            auto theta{MathLib::Util::degToRad(vFov)};
            auto h{tan(theta / 2)};
            auto viewportHeight{2.0 * h};
            auto viewportWidth{aspectRatio * viewportHeight};

            w = normalize(lookfrom - lookat);
            u = normalize(cross(vup, w));
            v = cross(w, u);

            origin = lookfrom;
            horizontal = focusDist * viewportWidth * u;
            vertical = focusDist * viewportHeight * v;
            lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - focusDist * w;

            lensRadius = aperture / 2;
        }
    };

    unsigned int cameraBuffer;
    glGenBuffers(1, &cameraBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cameraBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 29 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cameraBuffer);

    char *materialData{(char *)malloc(600 * materialSize)};
    addMaterial(materialData, {0, {0.5, 0.5, 0.5}, 0.0, 0.0});
    addMaterial(materialData + materialSize, {2, {0, 0, 0}, 0.0, 1.5});
    addMaterial(materialData + 2 * materialSize, {3, {1.5, 1.5, 1.5}, 0.0, 0.0});
    addMaterial(materialData + 3 * materialSize, {2, {0, 0, 0}, 0.0, 1.5});
    addMaterial(materialData + 4 * materialSize, {3, {0.9, 0.6, 0.2}, 0.0, 0.0});
    addMaterial(materialData + 5 * materialSize, {2, {0, 0, 0}, 0.0, 1.5});

    char *sphereData{(char *)malloc(600 * sphereSize)};
    addSphere(sphereData, {{0, -1000, 0}, 1000, 0});
    addSphere(sphereData + sphereSize, {{0, 1, 0}, 1, 1});
    addSphere(sphereData + 2 * sphereSize, {{-4, 1, 0}, 0.8, 2});
    addSphere(sphereData + 3 * sphereSize, {{-4, 1, 0}, 1, 3});
    addSphere(sphereData + 4 * sphereSize, {{4, 1, 0}, 0.8, 4});
    addSphere(sphereData + 5 * sphereSize, {{4, 1, 0}, 1, 5});

    int count{6};

    for (int a{-10}; a < 10; ++a)
    {
        for (int b{-10}; b < 10; ++b)
        {
            float chooseMat{MathLib::Util::random_number<float>()};
            Engine::Point3 center{
                a + 0.9 * MathLib::Util::random_number<float>(), 0.2, b + 0.9 * MathLib::Util::random_number<float>()};

            if ((center - Engine::Point3{4, 0.2, 0}).norm() > 0.9)
            {
                if (chooseMat < 0.8)
                {
                    addMaterial(materialData + count * materialSize,
                                {0, Engine::Vector3::random() * Engine::Vector3::random(), 0.0, 0.0});
                    addSphere(sphereData + count * sphereSize, {center, 0.2, count});
                }
                else if (chooseMat < 0.90)
                {
                    addMaterial(materialData + count * materialSize,
                                {1, Engine::Vector3::random(0.5, 1), MathLib::Util::random_number<float>(0, 0.5), 0.0});
                    addSphere(sphereData + count * sphereSize, {center, 0.2, count});
                }
                else
                {
                    addMaterial(materialData + count * materialSize,
                                {3, Engine::Vector3::random() * Engine::Vector3::random(), 0, 0});
                    addSphere(sphereData + count * sphereSize, {center, 0.18, count});
                    ++count;
                    addMaterial(materialData + count * materialSize, {2, {0, 0, 0}, 0.0, 1.5});
                    addSphere(sphereData + count * sphereSize, {center, 0.2, count});
                }

                ++count;
            }
        }
    }

    char *bbData{(char *)malloc(boxSize * (count + 100))};

    std::vector<int> sphereIndices(count - 6);

    for (int i{0}; i < (count - 6); ++i)
    {
        sphereIndices[i] = i + 6;
    }

    int numBoxes{addBox(bbData, sphereData, sphereIndices, 0, (count - 6), 0)};

    unsigned int boxBuffer;
    glGenBuffers(1, &boxBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, boxBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numBoxes * boxSize, bbData, GL_STATIC_DRAW);
    free(bbData);

    unsigned int materialBuffer;
    glGenBuffers(1, &materialBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, count * materialSize, materialData, GL_STATIC_DRAW);
    free(materialData);

    unsigned int sphereBuffer;
    glGenBuffers(1, &sphereBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, count * sphereSize, sphereData, GL_STATIC_DRAW);
    free(sphereData);

    int hittables[]{0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 1, 0};
    unsigned int hittableBuffer;
    glGenBuffers(1, &hittableBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, hittableBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 14 * sizeof(int), hittables, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, hittableBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, boxBuffer);

    glBindImageTexture(0, outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    std::cout << "Started Rendering\n";

    glUniform1i(glGetUniformLocation(program, "width"), width);
    glUniform1i(glGetUniformLocation(program, "height"), height);

    auto startXLocation{glGetUniformLocation(program, "startX")};
    auto startYLocation{glGetUniformLocation(program, "startY")};

    auto xSubdivisions{7};
    auto ySubdivisions{7};

    auto subX{width / xSubdivisions};
    auto subY{height / ySubdivisions};

    std::vector<unsigned char> pixels;
    pixels.resize(width * height * 3);
    stbi_flip_vertically_on_write(true);

    // subdividing the workload to prevent the compute shader from running to long and being killed by the OS
    // see: https://stackoverflow.com/a/57216257

    int numPics{1};

    for (int i{0}; i < numPics; ++i)
    {
        auto rotation{Engine::getRotation({0, MathLib::Util::degToRad(i * (360.0 / numPics)), 0})};

        Engine::Point3 lookfrom{rotation * Engine::Point3{13, 2, 3}};
        Engine::Point3 lookat{0, 0, 0};
        Camera c{lookfrom, lookat, {0, 1, 0}, 20.0, aspectRatio, 0.1, 10.0};

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cameraBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 3 * sizeof(float), c.origin.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(float), 3 * sizeof(float), c.lowerLeftCorner.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 8 * sizeof(float), 3 * sizeof(float), c.horizontal.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 12 * sizeof(float), 3 * sizeof(float), c.vertical.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16 * sizeof(float), 3 * sizeof(float), c.u.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 20 * sizeof(float), 3 * sizeof(float), c.v.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 24 * sizeof(float), 3 * sizeof(float), c.w.data());
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 28 * sizeof(float), sizeof(float), &c.lensRadius);

        for (int y{0}; y < ySubdivisions; ++y)
        {
            for (int x{0}; x < ySubdivisions; ++x)
            {
                glUniform1i(startXLocation, x * subX);
                glUniform1i(startYLocation, y * subY);
                glDispatchCompute(subX, subY, 1);
                glFinish();
            }
        }

        // glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindTexture(GL_TEXTURE_2D, outTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        std::string fileName{"test" + std::to_string(i) + ".jpg"};

        stbi_write_jpg(fileName.c_str(), width, height, 3, pixels.data(), 100);
    }

    glDeleteBuffers(1, &cameraBuffer);
    glDeleteBuffers(1, &materialBuffer);
    glDeleteBuffers(1, &sphereBuffer);

    glDeleteTextures(1, &outTexture);

    glfwTerminate();
}
