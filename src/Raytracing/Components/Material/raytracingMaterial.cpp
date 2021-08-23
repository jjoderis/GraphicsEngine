#include "raytracingMaterial.h"

Engine::RaytracingMaterial::RaytracingMaterial() {}

void Engine::RaytracingMaterial::setColor(const Vector4 &color) { m_color = color; }
void Engine::RaytracingMaterial::setColor(float r, float g, float b, float a) { m_color = Engine::Vector4{r, g, b, a}; }
Engine::Vector4 &Engine::RaytracingMaterial::getColor() { return m_color; }

void Engine::RaytracingMaterial::makeReflective() { m_reflective = true; }
void Engine::RaytracingMaterial::makeUnreflective() { m_reflective = false; }
bool Engine::RaytracingMaterial::isReflective() { return m_reflective; }