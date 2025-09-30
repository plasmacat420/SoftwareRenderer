#pragma once
#include "Vector3D.h"

class Matrix4x4 {
public:
    float m[4][4];

    Matrix4x4();
    
    static Matrix4x4 identity();
    static Matrix4x4 rotationX(float angle);
    static Matrix4x4 rotationY(float angle);
    static Matrix4x4 rotationZ(float angle);
    static Matrix4x4 translation(float x, float y, float z);

    Matrix4x4 operator*(const Matrix4x4& other) const;
    Vector3D transform(const Vector3D& vec) const;
};