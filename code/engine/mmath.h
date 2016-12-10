#ifndef MMATH_H
#define MMATH_H

#include "types.h"
#include <math.h>

#define mmin(a,b) ((a) > (b) ? (b) : (a))
#define mmax(a,b) ((a) > (b) ? (a) : (b))

inline I32 maxI32(I32 a, I32 b) {
  return mmax(a,b);
}

inline I32 minI32(I32 a, I32 b) {
  return mmin(a,b);
}

inline F32 maxF32(F32 a, F32 b) {
  return mmax(a,b);
}

inline F32 minF32(F32 a, F32 b) {
  return mmin(a,b);
}

struct Vector3 {
    union {
        F32 data[3];
        struct {
            F32 x;
            F32 y;
            F32 z;
        };
    };

    // inline Vector3 add(const Vector3& other) {
    //     Vector3 result = {other.x + x,
    //                         other.y + y,
    //                         other.z + z};
    //     return result;
    // }

    // inline Vector3 subtract(const Vector3& other) {
    //     Vector3 result = {x - other.x,
    //                         y - other.y,
    //                         z - other.z};
    //     return result;
    // }

    // inline Vector3 multiply(F32 other) {
    //     Vector3 result = {other * x,
    //                         other * y,
    //                         other * z};
    //     return result;
    // }
    // inline Vector3 negate() {
    //   return {-x,-y,-z};
    // }

    inline Vector3 operator+(const Vector3& other) {
        Vector3 result = {other.x + x,
                            other.y + y,
                            other.z + z};
        return result;
    }

    inline Vector3 operator-(const Vector3& other) {
        Vector3 result = {x - other.x,
                            y - other.y,
                            z - other.z};
        return result;
    }

    inline Vector3 operator*(F32 other) {
        Vector3 result = {other * x,
                            other * y,
                            other * z};
        return result;
    }
    inline Vector3 operator-() {
      return {-x,-y,-z};
    }

    inline Vector3 multiplyVector(Vector3 other) {
        Vector3 result = {other.x * x,
                            other.y * y,
                            other.z * z};
        return result;
    }

    inline F32 dot(const Vector3& other) {
        F32 result = other.x * x+other.y * y+other.z * z;
        return result;
    }

    inline Vector3 cross(const Vector3& other) {
        Vector3 result = {y*other.z - z*other.y,
                          z*other.x - x*other.z,
                          x*other.y - y*other.x};
        return result;
    }

    inline F32 lengthSquared() {
      return x*x+y*y+z*z;
    }

    F32 length();

    inline Vector3 normalize() {
        return (*this)*(1.0/length());
    }
};

struct Matrix4
{
    F32 data[16];

    Matrix4 operator*(const Matrix4& other) {
        Matrix4 result = {data[0]*other.data[0] + data[4]*other.data[1] + data[8]*other.data[2] + data[12]*other.data[3],
                          data[1]*other.data[0] + data[5]*other.data[1] + data[9]*other.data[2] + data[13]*other.data[3],
                          data[2]*other.data[0] + data[6]*other.data[1] + data[10]*other.data[2] + data[14]*other.data[3],
                          data[3]*other.data[0] + data[7]*other.data[1] + data[11]*other.data[2] + data[15]*other.data[3],

                          data[0]*other.data[4] + data[4]*other.data[5] + data[8]*other.data[6] + data[12]*other.data[7],
                          data[1]*other.data[4] + data[5]*other.data[5] + data[9]*other.data[6] + data[13]*other.data[7],
                          data[2]*other.data[4] + data[6]*other.data[5] + data[10]*other.data[6] + data[14]*other.data[7],
                          data[3]*other.data[4] + data[7]*other.data[5] + data[11]*other.data[6] + data[15]*other.data[7],

                          data[0]*other.data[8] + data[4]*other.data[9] + data[8]*other.data[10] + data[12]*other.data[11],
                          data[1]*other.data[8] + data[5]*other.data[9] + data[9]*other.data[10] + data[13]*other.data[11],
                          data[2]*other.data[8] + data[6]*other.data[9] + data[10]*other.data[10] + data[14]*other.data[11],
                          data[3]*other.data[8] + data[7]*other.data[9] + data[11]*other.data[10] + data[15]*other.data[11],

                          data[0]*other.data[12] + data[4]*other.data[13] + data[8]*other.data[14] + data[12]*other.data[15],
                          data[1]*other.data[12] + data[5]*other.data[13] + data[9]*other.data[14] + data[13]*other.data[15],
                          data[2]*other.data[12] + data[6]*other.data[13] + data[10]*other.data[14] + data[14]*other.data[15],
                          data[3]*other.data[12] + data[7]*other.data[13] + data[11]*other.data[14] + data[15]*other.data[15]};
        return result;
    }

    Vector3 transformPoint(const Vector3& other) {
        Vector3 result = {data[0]*other.data[0] + data[4]*other.data[1] + data[8]*other.data[2] + data[12],
                            data[1]*other.data[0] + data[5]*other.data[1] + data[9]*other.data[2] + data[13],
                            data[2]*other.data[0] + data[6]*other.data[1] + data[10]*other.data[2] + data[14]};
        return result;
    }

    Vector3 transformVector(const Vector3& other) {
        Vector3 result = {data[0]*other.data[0] + data[4]*other.data[1] + data[8]*other.data[2],
                            data[1]*other.data[0] + data[5]*other.data[1] + data[9]*other.data[2],
                            data[2]*other.data[0] + data[6]*other.data[1] + data[10]*other.data[2]};
        return result;
    }

    Vector3 transformPerspective(const Vector3& other) {
      Vector3 result = transformPoint(other);
      F32 w = data[3]*other.data[0] + data[7]*other.data[1] + data[11]*other.data[2] + data[15];
      result = result * (1.0/w);
      return result;
    }
};

struct AABB {
  Vector3 min;
  Vector3 max;
  AABB transformByMatrix4(Matrix4* matrix) {
    Vector3 newmin = matrix->transformPoint(min);
	Vector3 newmax = matrix->transformPoint(max);
	F32 minx = mmin(newmin.x, newmax.x); F32 maxx = mmax(newmin.x, newmax.x);
    newmin.x = minx; newmax.x = maxx;
	F32 miny = mmin(newmin.y, newmax.y); F32 maxy = mmax(newmin.y, newmax.y);
    newmin.y = miny; newmax.y = maxy;
	F32 minz = mmin(newmin.z, newmax.z); F32 maxz = mmax(newmin.z, newmax.z);
    newmin.z = minz; newmax.z = maxz;
	AABB result = {newmin, newmax};
    return result;
    // TODO: ensure max and min are max and min
  }
  inline bool intersect(AABB other) {
    return (other.max.x >= min.x && other.max.y >= min.y && other.max.z >= min.z  && other.min.x <= max.x && other.min.y <= max.y && other.min.z <= max.z);

  }
};

struct Quat {
    union {
        F32 data[4];
        struct {
            F32 a;
            F32 b;
            F32 c;
            F32 d;
        };
    };
    inline Quat multiply(Quat other) {
        Quat result = {a * other.a - b * other.b - c * other.c - d * other.d,
                       a * other.b + b * other.a + c * other.d - d * other.c,
                       a * other.c - b * other.d + c * other.a + d * other.b,
                       a * other.d + b * other.c - c * other.b + d * other.a};
        return result;
    }

    inline Vector3 transformVector(Vector3 v) {
      Quat vectorQuat = {0, v.x, v.y, v.z};
      Quat transformQuat = multiply(vectorQuat).multiply(inverse());
      Vector3 result = {transformQuat.b,transformQuat.c,transformQuat.d};
      return result;
    }

    inline Quat inverse() {
      Quat result = {a, -b, -c, -d};
      return result;
    }

    Matrix4 toRotationMatrix() {
      Matrix4 result = {1 - 2*c*c - 2*d*d, 2*b*c + 2*d*a, 2*b*d - 2*a*c, 0,
                        2*b*c - 2*d*a, 1 - 2*b*b - 2*d*d, 2*c*d + 2*b*a, 0,
                        2*b*d + 2*a*c, 2*c*d - 2*b*a, 1 - 2*b*b - 2*c*c, 0,
                        0, 0, 0,                                         1};
      return result;
    }
};

struct VectorI {
    I32 x;
    I32 y;
    I32 z;
};

struct Vector2 {
    union {
        F32 data[2];
        struct {
            F32 x;
            F32 y;
        };
    };

    inline Vector2 operator+(const Vector2& other) {
        Vector2 result = {other.x + x,
                            other.y + y};
        return result;
    }

    inline Vector2 operator-(const Vector2& other) {
        Vector2 result = {-other.x + x,
                            -other.y + y};
        return result;
    }

    inline Vector2 operator*(F32 other) {
        Vector2 result = {other * x,
                            other * y};
        return result;
    }
    inline Vector2 multiplyVector(const Vector2& other) {
        Vector2 result = {other.x * x,
                            other.y * y};
        return result;
    }

    inline F32 dot(const Vector2& other) {
        F32 result = other.x * x+other.y * y;
        return result;
    }

    F32 length();

    inline Vector2 normalize() {
        return (*this)*(1.0/length());
    }
};

struct Rect {
  Vector2 min, max;
  bool containsPoint(Vector2 point);
  bool intersectRect(Rect other);
};

struct Quad {
  Vector2 bottomleft;
  Vector2 topright;
  Vector2 bottomright;
  Vector2 topleft;
};

struct Matrix3 {
  F32 data[9];
};

struct TransformMatrix2d {
  F32 data[6];
  TransformMatrix2d multiply(const TransformMatrix2d& other) {
      TransformMatrix2d result = {data[0]*other.data[0] + data[2]*other.data[1],
                                  data[1]*other.data[0] + data[3]*other.data[1],

                                  data[0]*other.data[2] + data[2]*other.data[3],
                                  data[1]*other.data[2] + data[3]*other.data[3],

                                  data[0]*other.data[4] + data[2]*other.data[5] + data[4],
                                  data[1]*other.data[4] + data[3]*other.data[5] + data[5]};
      return result;
  }

  Matrix3 toMatrix3() {
    Matrix3 result = {data[0], data[1], 0.0f,
                      data[2], data[3], 0.0f,
                      data[4], data[5], 1.0f};
    return result;
  }

  Vector2 transformPoint(const Vector2& other) {
      Vector2 result = {data[0]*other.data[0] + data[2]*other.data[1] + data[4],
                        data[1]*other.data[0] + data[3]*other.data[1] + data[5]};
      return result;
  }

  Vector2 transformVector(const Vector2& other) {
      Vector2 result = {data[0]*other.data[0] + data[2]*other.data[1],
                        data[1]*other.data[0] + data[3]*other.data[1]};
      return result;
  }

  Quad transformQuad(const Quad& other) {
      Quad result = {transformPoint(other.bottomleft),transformPoint(other.topright),
                      transformPoint(other.bottomright),transformPoint(other.topleft)};
      return result;
  }
};

struct Vector4 {
  union {
    F32 data[4];
    struct {
      Vector3 xyz;
      F32 _w;
    };
    struct {
      F32 x;
      F32 y;
      F32 z;
      F32 w;
    };
  };
};


Quat math_getAngleAxisQuat(F32 angle, Vector3 axis);
Quat math_getIdentityQuat();

Matrix4 math_getIdentityMatrix();
Matrix4 math_getPerspectiveMatrix(F32 znear, F32 zfar, F32 aspect, F32 fov);
Matrix4 math_getViewMatrix(Vector3 p_pos, Vector3 p_dir, Vector3 p_up);
Matrix4 math_getTranslationMatrix(Vector3 translation);
Matrix4 math_getScaleMatrix(F32 scale);

#define PI 3.14159265

#endif
