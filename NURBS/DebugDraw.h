#pragma once
#include<vector>
#include<cmath>
/*
* 2D空间中顶点的定义
*
*/
struct Vertex {

    Vertex() = default;
    Vertex(float xIn, float yIn) :x(xIn), y(yIn) {}

    float Size() {
        return sqrtf(x * x + y * y);
    }
    float SizeSquared()const {
        return x * x + y * y;
    }
    void operator += (const Vertex& v)
    {
        x += v.x; y += v.y;
    }
    void operator -= (const Vertex& v)
    {
        x -= v.x; y -= v.y;
    }
    void operator *= (float a)
    {
        x *= a; y *= a;
    }



    float x;
    float y;
};
using Vec2 = Vertex;
/*
*  顶点的运算重载操作符
*/
inline Vertex operator + (const Vertex& a, const Vertex& b)
{
    return Vertex(a.x + b.x, a.y + b.y);
}

inline Vertex operator - (const Vertex& a, const Vertex& b)
{
    return Vertex(a.x - b.x, a.y - b.y);
}

inline Vertex operator * (float s, const Vertex& a)
{
    return Vertex(s * a.x, s * a.y);
}
inline Vertex operator * (const Vertex& a,float s)
{
    return Vertex(s * a.x, s * a.y);
}

inline Vertex operator / (const Vertex& a,float s)
{
    return Vertex(a.x/s,  a.y/s);
}
inline bool operator == (const Vertex& a, const Vertex& b)
{
    return a.x == b.x && a.y == b.y;
}

inline bool operator != (const Vertex& a, const Vertex& b)
{
    return a.x != b.x || a.y != b.y;
}
template <typename T>
inline T Min(T a, T b)
{
    return a < b ? a : b;
}

inline Vec2 Min(const Vec2& a, const Vec2& b)
{
    return Vec2(Min(a.x, b.x), Min(a.y, b.y));
}

template <typename T>
inline T Max(T a, T b)
{
    return a > b ? a : b;
}

inline Vec2 Max(const Vec2& a, const Vec2& b)
{
    return Vec2(Max(a.x, b.x), Max(a.y, b.y));
}

/*
* 轴对齐包围盒
*
*/
struct AABB
{
    Vec2 lowerBound;
    Vec2 upperBound;
    bool IsValid() const {
        Vec2 d = upperBound - lowerBound;
        bool valid = d.x >= 0.0f && d.y >= 0.0f;
        return valid;
    }


    Vec2 GetCenter() const
    {
        return 0.5f * (lowerBound + upperBound);
    }

    Vec2 GetExtents() const
    {
        return 0.5f * (upperBound - lowerBound);
    }


    float GetPerimeter() const
    {
        float wx = upperBound.x - lowerBound.x;
        float wy = upperBound.y - lowerBound.y;
        return 2.0f * (wx + wy);
    }


    void Combine(const AABB& aabb)
    {
        lowerBound = Min(lowerBound, aabb.lowerBound);
        upperBound = Max(upperBound, aabb.upperBound);
    }


    void Combine(const AABB& aabb1, const AABB& aabb2)
    {
        lowerBound = Min(aabb1.lowerBound, aabb2.lowerBound);
        upperBound = Max(aabb1.upperBound, aabb2.upperBound);
    }

    /// Does this aabb contain the provided AABB.
    bool Contains(const AABB& aabb) const
    {
        bool result = true;
        result = result && lowerBound.x <= aabb.lowerBound.x;
        result = result && lowerBound.y <= aabb.lowerBound.y;
        result = result && aabb.upperBound.x <= upperBound.x;
        result = result && aabb.upperBound.y <= upperBound.y;
        return result;
    }
};
struct GLRenderPoints;
struct GLRenderLines;
struct GLRenderTriangles;

struct Color
{
	Color() {}
	Color(float rIn, float gIn, float bIn, float aIn = 1.0f)
	{
		r = rIn; g = gIn; b = bIn; a = aIn;
	}

	void Set(float rIn, float gIn, float bIn, float aIn = 1.0f)
	{
		r = rIn; g = gIn; b = bIn; a = aIn;
	}

	float r, g, b, a;
};
struct Camera
{
	Camera();

	void ResetView();
	Vec2 ConvertScreenToWorld(const Vec2& screenPoint);
	Vec2 ConvertWorldToScreen(const Vec2& worldPoint);
	void BuildProjectionMatrix(float* m, float zBias);

	Vec2 m_center;
	float m_zoom;
	int m_width;
	int m_height;
};

class DebugDraw 
{
public:
	
	DebugDraw();
	~DebugDraw();

	void Create();
	void Destroy();

	void DrawPolygon(const Vec2* vertices, int vertexCount, const Color& color) ;
    void DrawMulLine(const Vec2* vertices, int vertexCount, const Color& color);
	void DrawSolidPolygon(const Vec2* vertices, int vertexCount, const Color& color) ;

	void DrawCircle(const Vec2& center, float radius, const Color& color) ;
	
	
	

	void DrawSolidCircle(const Vec2& center, float radius, const Vec2& axis, const Color& color) ;

	void DrawSegment(const Vec2& p1, const Vec2& p2, const Color& color) ;



	void DrawAABB(AABB* aabb, const Color& color);

	void Flush();
private:
	 GLRenderPoints* m_points;
	 GLRenderLines* m_lines;
	 GLRenderTriangles* m_triangles;

};

extern DebugDraw maindraw;
extern Camera g_camera;