#include "./quaternion.h"

Engine::Quaternion::Quaternion() {}

Engine::Quaternion::Quaternion(const MathLib::Vector<float, 3> &imaginary, float real)
{
    m_qv = imaginary;
    m_qw = real;
}

Engine::Quaternion::Quaternion(float x, float y, float z, float w) : Quaternion{}
{
    m_qv(0) = x;
    m_qv(1) = y;
    m_qv(2) = z;
    m_qw = w;
}

Engine::Quaternion::Quaternion(const Quaternion &other) : Quaternion{}
{
    m_qv = other.m_qv;
    m_qw = other.m_qw;
}

Engine::Quaternion &Engine::Quaternion::operator=(const Quaternion &other)
{

    if (&other == this)
    {
        return *this;
    }

    m_qv = other.m_qv;
    m_qw = other.m_qw;

    return *this;
}

float &Engine::Quaternion::qx() { return m_qv(0); }

float Engine::Quaternion::qx() const { return m_qv(0); }

float &Engine::Quaternion::qy() { return m_qv(1); }

float Engine::Quaternion::qy() const { return m_qv(1); }

float &Engine::Quaternion::qz() { return m_qv(2); }

float Engine::Quaternion::qz() const { return m_qv(2); }

float &Engine::Quaternion::qw() { return m_qw; }

float Engine::Quaternion::qw() const { return m_qw; }

MathLib::Vector<float, 3> &Engine::Quaternion::qv() { return m_qv; }

const MathLib::Vector<float, 3> &Engine::Quaternion::qv() const { return m_qv; }

Engine::Quaternion Engine::Quaternion::getConjugate() const { return Quaternion{-m_qv, m_qw}; }

float Engine::Quaternion::norm() const
{
    return sqrt(pow(m_qv(0), 2.0) + pow(m_qv(1), 2.0) + pow(m_qv(2), 2.0) + pow(m_qw, 2.0));
}

Engine::Quaternion &Engine::Quaternion::setIdentity()
{
    m_qv = MathLib::Vector<float, 3>{0.0, 0.0, 0.0};
    m_qw = 1;

    return *this;
}

Engine::Quaternion Engine::Quaternion::getInverse() const
{
    return (1 / pow(this->norm(), 2.0)) * this->getConjugate();
}

Engine::Quaternion &Engine::Quaternion::setUnit()
{
    *this = (1 / this->norm()) * (*this);

    return *this;
}

Engine::Quaternion Engine::Quaternion::getUnit()
{
    Quaternion unit{*this};
    unit.setUnit();
    return unit;
}

Engine::Quaternion &Engine::Quaternion::setRotation(const MathLib::Vector<float, 3> &axis, float angle)
{
    MathLib::Vector<float, 3> normalizedAxis{axis};
    normalize(normalizedAxis);
    m_qv = normalizedAxis * sin(angle / 2);
    m_qw = cos(angle / 2);
    return *this;
}

Engine::Quaternion &Engine::Quaternion::operator*=(const Quaternion &other)
{
    *this = *this * other;
    return *this;
}

Engine::Quaternion Engine::operator*(const Quaternion &q, const Quaternion &r)
{
    Quaternion product{};

    product.qv() = (cross(q.qv(), r.qv()) + r.qw() * q.qv() + q.qw() * r.qv());
    product.qw() = q.qw() * r.qw() - dot(q.qv(), r.qv());

    return product;
}

Engine::Quaternion Engine::operator*(float scalar, const Quaternion &q)
{
    Quaternion product{};

    product.qv() = q.qv() * scalar;
    product.qw() = q.qw() * scalar;

    return product;
}

Engine::Quaternion Engine::operator*(const Quaternion &q, float scalar) { return scalar * q; }

Engine::Quaternion Engine::operator+(const Quaternion &q, const Quaternion &r)
{
    Quaternion sum{};

    sum.qv() = q.qv() + r.qv();
    sum.qw() = q.qw() + r.qw();

    return sum;
}

Engine::Quaternion Engine::slerp(const Quaternion &q, const Quaternion &r, float stepSize)
{
    float angle{acos(dot(q.qv(), r.qv()) + q.qw() * r.qw())};

    return (sin(angle * (1 - stepSize)) / sin(angle)) * q + (sin(angle * stepSize) / sin(angle)) * r;
}

bool Engine::operator==(const Quaternion &q1, const Quaternion &q2)
{
    return allClose(q1.qv(), q2.qv()) && MathLib::Util::isClose(q1.qw(), q2.qw());
}