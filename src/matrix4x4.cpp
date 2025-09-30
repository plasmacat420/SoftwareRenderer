#include "Matrix4x4.h"
#include <cmath>
#include <cstring>

Matrix4x4::Matrix4x4() {
    std::memset(m, 0, sizeof(m));
}

Matrix4x4 Matrix4x4::identity() {
    Matrix4x4 result;
    for (int i = 0; i < 4; i++) {
        result.m[i][i] = 1.0f;
    }
    return result;
}

Matrix4x4 Matrix4x4::rotationX(float angle) {
    Matrix4x4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    return result;
}

Matrix4x4 Matrix4x4::rotationY(float angle) {
    Matrix4x4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    return result;
}

Matrix4x4 Matrix4x4::rotationZ(float angle) {
    Matrix4x4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    return result;
}

Matrix4x4 Matrix4x4::translation(float x, float y, float z) {
    Matrix4x4 result = identity();
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

Vector3D Matrix4x4::transform(const Vector3D& vec) const {
    float w = m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3];
    if (w == 0.0f) w = 1.0f;
    
    return Vector3D(
        (m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3]) / w,
        (m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3]) / w,
        (m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3]) / w
    );
}