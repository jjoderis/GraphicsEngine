#include "./quaternion.h"

Engine::Math::Quaternion::Quaternion() {}

Engine::Math::Quaternion::Quaternion(const TypeVector3<float> &imaginary, float real)
{
    m_qv = imaginary;
    m_qw = real;
}

Engine::Math::Quaternion::Quaternion(float x, float y, float z, float w) : Quaternion{}
{
    m_qv(0) = x;
    m_qv(1) = y;
    m_qv(2) = z;
    m_qw = w;
}

Engine::Math::Quaternion::Quaternion(const Quaternion &other) : Quaternion{}
{
    m_qv = other.m_qv;
    m_qw = other.m_qw;
}

Engine::Math::Quaternion &Engine::Math::Quaternion::operator=(const Quaternion &other)
{

    if (&other == this)
    {
        return *this;
    }

    m_qv = other.m_qv;
    m_qw = other.m_qw;

    return *this;
}

float &Engine::Math::Quaternion::qx() { return m_qv(0); }

float Engine::Math::Quaternion::qx() const { return m_qv(0); }

float &Engine::Math::Quaternion::qy() { return m_qv(1); }

float Engine::Math::Quaternion::qy() const { return m_qv(1); }

float &Engine::Math::Quaternion::qz() { return m_qv(2); }

float Engine::Math::Quaternion::qz() const { return m_qv(2); }

float &Engine::Math::Quaternion::qw() { return m_qw; }

float Engine::Math::Quaternion::qw() const { return m_qw; }

Engine::TypeVector3<float> &Engine::Math::Quaternion::qv() { return m_qv; }

const Engine::TypeVector3<float> &Engine::Math::Quaternion::qv() const { return m_qv; }

Engine::Math::Quaternion Engine::Math::Quaternion::getConjugate() const { return Quaternion{-m_qv, m_qw}; }

float Engine::Math::Quaternion::norm() const
{
    return sqrt(pow(m_qv(0), 2.0) + pow(m_qv(1), 2.0) + pow(m_qv(2), 2.0) + pow(m_qw, 2.0));
}

Engine::Math::Quaternion &Engine::Math::Quaternion::setIdentity()
{
    m_qv = TypeVector3<float>{0.0, 0.0, 0.0};
    m_qw = 1;

    return *this;
}

Engine::Math::Quaternion Engine::Math::Quaternion::getInverse() const
{
    return (1 / pow(this->norm(), 2.0)) * this->getConjugate();
}

Engine::Math::Quaternion &Engine::Math::Quaternion::setUnit()
{
    *this = (1 / this->norm()) * (*this);

    return *this;
}

Engine::Math::Quaternion Engine::Math::Quaternion::getUnit()
{
    Quaternion unit{*this};
    unit.setUnit();
    return unit;
}

Engine::Math::Quaternion &Engine::Math::Quaternion::setRotation(const TypeVector3<float> &axis, float angle)
{
    TypeVector3<float> normalizedAxis{axis};
    normalize(normalizedAxis);
    m_qv = normalizedAxis * sin(angle / 2);
    m_qw = cos(angle / 2);
    return *this;
}

Engine::Math::Quaternion &Engine::Math::Quaternion::operator*=(const Quaternion &other)
{
    *this = *this * other;
    return *this;
}

Engine::Math::Quaternion Engine::Math::operator*(const Quaternion &q, const Quaternion &r)
{
    Quaternion product{};

    product.qv() = (cross(q.qv(), r.qv()) + r.qw() * q.qv() + q.qw() * r.qv());
    product.qw() = q.qw() * r.qw() - dot(q.qv(), r.qv());

    return product;
}

Engine::Math::Quaternion Engine::Math::operator*(float scalar, const Quaternion &q)
{
    Quaternion product{};

    product.qv() = q.qv() * scalar;
    product.qw() = q.qw() * scalar;

    return product;
}

Engine::Math::Quaternion Engine::Math::operator*(const Quaternion &q, float scalar) { return scalar * q; }

Engine::Math::Quaternion Engine::Math::operator+(const Quaternion &q, const Quaternion &r)
{
    Quaternion sum{};

    sum.qv() = q.qv() + r.qv();
    sum.qw() = q.qw() + r.qw();

    return sum;
}

Engine::Math::Quaternion Engine::Math::slerp(const Quaternion &q, const Quaternion &r, float stepSize)
{
    float angle{acos(dot(q.qv(), r.qv()) + q.qw() * r.qw())};

    return (sin(angle * (1 - stepSize)) / sin(angle)) * q + (sin(angle * stepSize) / sin(angle)) * r;
}

bool Engine::Math::operator==(const Quaternion &q1, const Quaternion &q2)
{
    return allClose(q1.qv(), q2.qv()) && MathLib::Util::isClose(q1.qw(), q2.qw());
}