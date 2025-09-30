#pragma once

class Vector3D {
public:
    float x, y, z;

    Vector3D();
    Vector3D(float x, float y, float z);

    Vector3D operator+(const Vector3D& other) const;
    Vector3D operator-(const Vector3D& other) const;
    Vector3D operator*(float scalar) const;
    Vector3D operator/(float scalar) const;

    float magnitude() const;
    Vector3D normalize() const;
    float dot(const Vector3D& other) const;
    Vector3D cross(const Vector3D& other) const;
};