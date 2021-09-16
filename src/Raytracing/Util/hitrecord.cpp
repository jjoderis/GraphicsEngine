#include "hitrecord.h"

void Engine::HitRecord::setFaceNormal(const Engine::Ray &r, const Engine::Vector3 &outwardNormal)
{
    frontFace = dot(r.getDirection(), outwardNormal) < 0;
    normal = frontFace ? outwardNormal : -outwardNormal;
}