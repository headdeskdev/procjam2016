#include "mmath.h"

Quat math_getAngleAxisQuat(F32 angle, Vector3 axis) {
  axis = axis.normalize();
  F32 s = sinf(angle/2.0);
  Quat result = {cosf(angle/2.0), s*axis.x, s*axis.y, s*axis.z};
  return result;
}

Quat math_getIdentityQuat() {
  Quat result = {1.0, 0.0, 0.0, 0.0};
  return result;
}

Matrix4 math_getIdentityMatrix()
{
    Matrix4 result = { 1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       0.0,0.0,0.0,1.0};
    return result;
}

Matrix4 math_getPerspectiveMatrix(F32 znear, F32 zfar, F32 aspect, F32 fov)
{
  F32 f = tan(3.141592 * 0.5 - 0.5 * fov);
  F32 rangeInv = 1.0 / (znear - zfar);

    Matrix4 result = {f / aspect, 0.0, 0.0, 0.0,
      0.0, f, 0.0, 0.0,
      0.0, 0.0, (znear + zfar) * rangeInv, -1.0,
      0.0, 0.0, znear * zfar * rangeInv * 2.0f, 0.0};
    return result;
}

Matrix4 math_getViewMatrix(Vector3 p_pos, Vector3 p_dir, Vector3 p_up)
{
    Vector3 position = p_pos;
    Vector3 zaxis = p_dir.normalize() * -1.0;
    Vector3 xaxis = p_up.cross(zaxis).normalize();
    Vector3 yaxis = zaxis.cross(xaxis).normalize();

    Vector3 up = p_up.normalize();
    Matrix4 T = {1.0,0.0,0.0,0.0,
                 0.0,1.0f,0.0,0.0,
                 0.0,0.0,1.0,0.0,
                 -position.x,-position.y,-position.z,1.0};
    Matrix4 M = {xaxis.x,yaxis.x,zaxis.x,0.0,
                 xaxis.y,yaxis.y,zaxis.y,0.0,
                 xaxis.z,yaxis.z,zaxis.z,0.0,
                0.0,0.0, 0.0,1.0};
    return M * T;

}

Matrix4 math_getTranslationMatrix(Vector3 translation) {
    Matrix4 T = {1.0,0.0,0.0,0.0,
                 0.0,1.0f,0.0,0.0,
                 0.0,0.0,1.0,0.0,
                 translation.x,translation.y,translation.z,1.0};
    return T;
}

Matrix4 math_getScaleMatrix(F32 scale)
{
    Matrix4 result = { scale,0.0,0.0,0.0,
                       0.0,scale,0.0,0.0,
                       0.0,0.0,scale,0.0,
                       0.0,0.0,0.0,1.0};
    return result;
}

inline F32 Vector3::length() {
    return sqrt(x*x+y*y+z*z);
}

inline F32 Vector2::length() {
    return sqrt(x*x+y*y);
}

inline bool Rect::containsPoint(Vector2 point) {
    return (point.x >= min.x && point.y >= min.y && point.x < max.x && point.y < max.y);
}

inline bool Rect::intersectRect(Rect other) {
  return (other.max.x >= min.x && other.max.y >= min.y && other.min.x <= max.x && other.min.y <= max.y);
}

static TransformMatrix2d math_get2dRotationMatrix(F32 rotation) {
  TransformMatrix2d matrix = {cosf(rotation),sinf(rotation),-sinf(rotation),cosf(rotation),0.0,0.0};
  return matrix;
}

static TransformMatrix2d math_get2dRigidTransformMatrix(Vector2 translation, F32 rotation, F32 scale) {
  TransformMatrix2d matrix = {scale*cosf(rotation),scale*sinf(rotation),-scale*sinf(rotation),scale*cosf(rotation),translation.x,translation.y};
  return matrix;
}

inline static Quad math_getAxisAlignedQuad(Rect r) {
  Quad quad = {r.min,r.max,{r.max.x, r.min.y},{r.min.x, r.max.y}};
  return quad;
}

Vector3 math_getBarycentricCoordinates(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 v0 = b - a;
    Vector3 v1 = c - a;
    Vector3 v2 = p - a;
    F32 d00 = v0.dot(v0);
    F32 d01 = v0.dot(v1);
    F32 d11 = v1.dot(v1);
    F32 d20 = v2.dot(v0);
    F32 d21 = v2.dot(v1);
    F32 denom = d00 * d11 - d01 * d01;
    F32 v = (d11 * d20 - d01 * d21) / denom;
    F32 w = (d00 * d21 - d01 * d20) / denom;
    F32 u = 1.0f - v - w;
    return {u,v,w};
}
