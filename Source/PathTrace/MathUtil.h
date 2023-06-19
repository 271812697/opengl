#pragma once
#include <cmath>
#include <algorithm>
#define PI 3.14159265358979323846f
namespace PathTrace {
    struct Math
    {
    public:

        static inline float Degrees(float radians) { return radians * (180.f / PI); };
        static inline float Radians(float degrees) { return degrees * (PI / 180.f); };
        static inline float Clamp(float x, float lower, float upper) { return std::min(upper, std::max(x, lower)); };
    };
    struct iVec2
    {
    public:
        iVec2() { x = 0, y = 0; };
        iVec2(int x, int y) { this->x = x; this->y = y; };

        int x, y;
    };

    struct Vec2
    {
    public:
        Vec2() { x = 0, y = 0; };
        Vec2(float x, float y) { this->x = x; this->y = y; };

        float x, y;
    };
    struct Vec4
    {
    public:
        Vec4();
        Vec4(float x, float y, float z, float w);

        float operator[](int i) const;

        float x, y, z, w;
    };

    inline Vec4::Vec4()
    {
        x = y = z = w = 0;
    };

    inline Vec4::Vec4(float x, float y, float z, float w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    };

    inline float Vec4::operator[](int i) const
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else if (i == 2)
            return z;
        else
            return w;
    };
    struct Vec3
    {
    public:
        Vec3();
        Vec3(float x, float y, float z);
        Vec3(const Vec4& b);

        Vec3 operator*(const Vec3& b) const;
        Vec3 operator+(const Vec3& b) const;
        Vec3 operator-(const Vec3& b) const;
        Vec3 operator*(float b) const;

        float operator[](int i) const;
        float& operator[](int i);

        static Vec3 Min(const Vec3& a, const Vec3& b);
        static Vec3 Max(const Vec3& a, const Vec3& b);
        static Vec3 Cross(const Vec3& a, const Vec3& b);
        static Vec3 Pow(const Vec3& a, float exp);
        static float Length(const Vec3& a);
        static float Distance(const Vec3& a, const Vec3& b);
        static float Dot(const Vec3& a, const Vec3& b);
        static Vec3 Clamp(const Vec3& a, const Vec3& min, const Vec3& max);
        static Vec3 Normalize(const Vec3& a);

        float x, y, z;
    };

    inline Vec3::Vec3()
    {
        x = y = z = 0;
    };

    inline Vec3::Vec3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    };

    inline Vec3::Vec3(const Vec4& b)
    {
        x = b.x;
        y = b.y;
        z = b.z;
    };

    inline Vec3 Vec3::operator*(const Vec3& b) const
    {
        return Vec3(x * b.x, y * b.y, z * b.z);
    };

    inline Vec3 Vec3::operator+(const Vec3& b) const
    {
        return Vec3(x + b.x, y + b.y, z + b.z);
    };

    inline Vec3 Vec3::operator-(const Vec3& b) const
    {
        return Vec3(x - b.x, y - b.y, z - b.z);
    };

    inline Vec3 Vec3::operator*(float b) const
    {
        return Vec3(x * b, y * b, z * b);
    };

    inline float Vec3::operator[](int i) const
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else
            return z;
    };

    inline float& Vec3::operator[](int i)
    {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else
            return z;
    };

    inline Vec3 Vec3::Min(const Vec3& a, const Vec3& b)
    {
        Vec3 out;
        out.x = std::min(a.x, b.x);
        out.y = std::min(a.y, b.y);
        out.z = std::min(a.z, b.z);
        return out;
    };

    inline Vec3 Vec3::Max(const Vec3& a, const Vec3& b)
    {
        Vec3 out;
        out.x = std::max(a.x, b.x);
        out.y = std::max(a.y, b.y);
        out.z = std::max(a.z, b.z);
        return out;
    };

    inline Vec3 Vec3::Cross(const Vec3& a, const Vec3& b)
    {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    };

    inline Vec3 Vec3::Pow(const Vec3& a, float exp)
    {
        return Vec3(
            powf(a.x, exp),
            powf(a.y, exp),
            powf(a.z, exp)
        );
    };

    inline float Vec3::Length(const Vec3& a)
    {
        return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
    };

    inline float Vec3::Dot(const Vec3& a, const Vec3& b)
    {
        return (a.x * b.x + a.y * b.y + a.z * b.z);
    };

    inline float Vec3::Distance(const Vec3& a, const Vec3& b)
    {
        Vec3 t = a;
        return Length(t - b);
    }

    inline Vec3 Vec3::Clamp(const Vec3& a, const Vec3& min, const Vec3& max)
    {
        return Vec3(
            Math::Clamp(a.x, min.x, max.x),
            Math::Clamp(a.y, min.y, max.y),
            Math::Clamp(a.z, min.z, max.z)
        );
    }

    inline Vec3 Vec3::Normalize(const Vec3& a)
    {
        float l = Length(a);
        return Vec3(a.x / l, a.y / l, a.z / l);
    };
    struct Mat4
    {

    public:
        Mat4();

        float(&operator [](int i))[4] { return data[i]; };
        Mat4 operator*(const Mat4& b) const;

        static Mat4 Translate(const Vec3& a);
        static Mat4 Scale(const Vec3& a);
        static Mat4 QuatToMatrix(float x, float y, float z, float w);

        float data[4][4];
    };

    inline Mat4::Mat4()
    {
        data[0][0] = 1; data[0][1] = 0;  data[0][2] = 0;  data[0][3] = 0;
        data[1][0] = 0; data[1][1] = 1;  data[1][2] = 0;  data[1][3] = 0;
        data[2][0] = 0; data[2][1] = 0;  data[2][2] = 1;  data[2][3] = 0;
        data[3][0] = 0; data[3][1] = 0;  data[3][2] = 0;  data[3][3] = 1;
    };

    inline Mat4 Mat4::Translate(const Vec3& a)
    {
        Mat4 out;
        out[3][0] = a.x;
        out[3][1] = a.y;
        out[3][2] = a.z;
        return out;
    }

    inline Mat4 Mat4::Scale(const Vec3& a)
    {
        Mat4 out;
        out[0][0] = a.x;
        out[1][1] = a.y;
        out[2][2] = a.z;
        return out;
    }

    inline Mat4 Mat4::operator*(const Mat4& b) const
    {
        Mat4 out;

        out[0][0] = data[0][0] * b.data[0][0] + data[0][1] * b.data[1][0] + data[0][2] * b.data[2][0] + data[0][3] * b.data[3][0];
        out[0][1] = data[0][0] * b.data[0][1] + data[0][1] * b.data[1][1] + data[0][2] * b.data[2][1] + data[0][3] * b.data[3][1];
        out[0][2] = data[0][0] * b.data[0][2] + data[0][1] * b.data[1][2] + data[0][2] * b.data[2][2] + data[0][3] * b.data[3][2];
        out[0][3] = data[0][0] * b.data[0][3] + data[0][1] * b.data[1][3] + data[0][2] * b.data[2][3] + data[0][3] * b.data[3][3];

        out[1][0] = data[1][0] * b.data[0][0] + data[1][1] * b.data[1][0] + data[1][2] * b.data[2][0] + data[1][3] * b.data[3][0];
        out[1][1] = data[1][0] * b.data[0][1] + data[1][1] * b.data[1][1] + data[1][2] * b.data[2][1] + data[1][3] * b.data[3][1];
        out[1][2] = data[1][0] * b.data[0][2] + data[1][1] * b.data[1][2] + data[1][2] * b.data[2][2] + data[1][3] * b.data[3][2];
        out[1][3] = data[1][0] * b.data[0][3] + data[1][1] * b.data[1][3] + data[1][2] * b.data[2][3] + data[1][3] * b.data[3][3];

        out[2][0] = data[2][0] * b.data[0][0] + data[2][1] * b.data[1][0] + data[2][2] * b.data[2][0] + data[2][3] * b.data[3][0];
        out[2][1] = data[2][0] * b.data[0][1] + data[2][1] * b.data[1][1] + data[2][2] * b.data[2][1] + data[2][3] * b.data[3][1];
        out[2][2] = data[2][0] * b.data[0][2] + data[2][1] * b.data[1][2] + data[2][2] * b.data[2][2] + data[2][3] * b.data[3][2];
        out[2][3] = data[2][0] * b.data[0][3] + data[2][1] * b.data[1][3] + data[2][2] * b.data[2][3] + data[2][3] * b.data[3][3];

        out[3][0] = data[3][0] * b.data[0][0] + data[3][1] * b.data[1][0] + data[3][2] * b.data[2][0] + data[3][3] * b.data[3][0];
        out[3][1] = data[3][0] * b.data[0][1] + data[3][1] * b.data[1][1] + data[3][2] * b.data[2][1] + data[3][3] * b.data[3][1];
        out[3][2] = data[3][0] * b.data[0][2] + data[3][1] * b.data[1][2] + data[3][2] * b.data[2][2] + data[3][3] * b.data[3][2];
        out[3][3] = data[3][0] * b.data[0][3] + data[3][1] * b.data[1][3] + data[3][2] * b.data[2][3] + data[3][3] * b.data[3][3];

        return out;
    }

    inline Mat4 Mat4::QuatToMatrix(float x, float y, float z, float w)
    {
        Mat4 out;

        const float x2 = x + x;
        const float y2 = y + y;
        const float z2 = z + z;

        const float xx = x * x2;
        const float xy = x * y2;
        const float xz = x * z2;

        const float yy = y * y2;
        const float yz = y * z2;
        const float zz = z * z2;

        const float wx = w * x2;
        const float wy = w * y2;
        const float wz = w * z2;

        out.data[0][0] = 1.0f - (yy + zz);
        out.data[0][1] = xy + wz;
        out.data[0][2] = xz - wy;
        out.data[0][3] = 0.0f;

        out.data[1][0] = xy - wz;
        out.data[1][1] = 1.0f - (xx + zz);
        out.data[1][2] = yz + wx;
        out.data[1][3] = 0.0f;

        out.data[2][0] = xz + wy;
        out.data[2][1] = yz - wx;
        out.data[2][2] = 1.0f - (xx + yy);
        out.data[2][3] = 0.0f;

        out.data[3][0] = 0;
        out.data[3][1] = 0;
        out.data[3][2] = 0;
        out.data[3][3] = 1.0f;

        return out;
    }

}