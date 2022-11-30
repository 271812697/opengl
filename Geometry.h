#pragma once
#include<vector>
#include"VertexTypes.h"
namespace Opengl
{
    using VertexCollection = std::vector<VertexPositionNormalTexture>;
    using IndexCollection = std::vector<unsigned int>;
    float XM_PI = 3.141592654f;
    float XM_2PI = 6.283185307f;
    float XM_1DIVPI = 0.318309886f;
    float XM_1DIV2PI = 0.159154943f;
    float XM_PIDIV2 = 1.570796327f;
    float XM_PIDIV4 = 0.785398163f;
    inline void XMScalarSinCos
    (
        float* pSin,
        float* pCos,
        float  Value
    )
    {
        assert(pSin);
        assert(pCos);

        // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
        float quotient = XM_1DIV2PI * Value;
        if (Value >= 0.0f)
        {
            quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
        }
        else
        {
            quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
        }
        float y = Value - XM_2PI * quotient;

        // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
        float sign;
        if (y > XM_PIDIV2)
        {
            y = XM_PI - y;
            sign = -1.0f;
        }
        else if (y < -XM_PIDIV2)
        {
            y = -XM_PI - y;
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        float y2 = y * y;

        // 11-degree minimax approximation
        *pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

        // 10-degree minimax approximation
        float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *pCos = sign * p;
    }
    inline void ReverseWinding(IndexCollection& indices, VertexCollection& vertices)
    {
        assert((indices.size() % 3) == 0);
        for (auto it = indices.begin(); it != indices.end(); it += 3)
        {
            std::swap(*it, *(it + 2));
        }

        for (auto& it : vertices)
        {
            it.textureCoordinate.x_ = (1.f - it.textureCoordinate.x_);
        }
    }
    // Helper for inverting normals of geometric primitives for 'inside' vs. 'outside' viewing
    inline void InvertNormals(VertexCollection& vertices)
    {
        for (auto& it : vertices)
        {
            it.normal.x_ = -it.normal.x_;
            it.normal.y_ = -it.normal.y_;
            it.normal.z_ = -it.normal.z_;
        }
    }
    inline void index_push_back(IndexCollection& indices, size_t value)
    {
       
        indices.push_back((value));
    }
    inline void ComputeBox(VertexCollection& vertices, IndexCollection& indices, const Vector3& size, bool rhcoords=true, bool invertn=false){
    vertices.clear();
    indices.clear();
    constexpr int FaceCount = 6;
    static const Vector3 faceNormals[FaceCount] =
    {
        {  0,  0,  1 },
        {  0,  0, -1},
        {  1,  0,  0},
        { -1,  0,  0 },
        {  0,  1,  0},
        {  0, -1,  0 } 
    };
    static const Vector4 textureCoordinates[4] =
    {
        { 1, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 0, 0 } 
    };
    Vector3 tsize ={size.x_/2,size.y_/2,size.z_/2};
    // Create each face in turn.
    for (int i = 0; i < FaceCount; i++)
    {
        const Vector3 normal = faceNormals[i];

        // Get two vectors perpendicular both to the face normal and to each other.
        const Vector3 basis = (i >= 4)? Vector3(0.0f, 0.0f, 1.0f): Vector3(0.0f, 1.0f, 0.0f);
     
        const Vector3 side1 = normal.CrossProduct(basis);
        const Vector3 side2 = normal.CrossProduct(side1);

        // Six indices (two triangles) per face.
        const size_t vbase = vertices.size();
        indices.push_back(vbase + 0);
        indices.push_back(vbase + 2);
        indices.push_back(vbase + 1);

        indices.push_back(vbase + 0);
        indices.push_back(vbase + 3);
        indices.push_back(vbase + 2);
        

        // Four vertices per face.
        // (normal - side1 - side2) * tsize // normal // t0
        vertices.push_back(VertexPositionNormalTexture((normal - side1 - side2) * tsize, normal, {1,0}));

        // (normal - side1 + side2) * tsize // normal // t1
        vertices.push_back(VertexPositionNormalTexture((normal - side1 + side2) * tsize, normal, {1,1}));

        // (normal + side1 + side2) * tsize // normal // t2
        vertices.push_back(VertexPositionNormalTexture((normal + side1 + side2) * tsize, normal, {0,1}));

        // (normal + side1 - side2) * tsize // normal // t3
        vertices.push_back(VertexPositionNormalTexture((normal + side1 - side2) * tsize, normal, {0,0}));
    }
    }
    
    inline void ComputeSphere(VertexCollection& vertices, IndexCollection& indices, float diameter, size_t tessellation, bool rhcoords=true, bool invertn=false) {
        vertices.clear();
        indices.clear();

        if (tessellation < 3)
            throw std::invalid_argument("tesselation parameter must be at least 3");

        const size_t verticalSegments = tessellation;
        const size_t horizontalSegments = tessellation * 2;

        const float radius = diameter / 2;

        // Create rings of vertices at progressively higher latitudes.
        for (size_t i = 0; i <= verticalSegments; i++)
        {
            const float v = 1 - float(i) / float(verticalSegments);

            const float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
            float dy, dxz;

            XMScalarSinCos(&dy, &dxz, latitude);

            // Create a single ring of vertices at this latitude.
            for (size_t j = 0; j <= horizontalSegments; j++)
            {
                const float u = float(j) / float(horizontalSegments);

                const float longitude = float(j) * XM_2PI / float(horizontalSegments);
                float dx, dz;

                XMScalarSinCos(&dx, &dz, longitude);

                dx *= dxz;
                dz *= dxz;

                Vector3 normal = { dx, dy, dz };
                Vector2 textureCoordinate = { u, v };

                vertices.push_back(VertexPositionNormalTexture(normal*radius, normal, textureCoordinate));
            }
        }

        // Fill the index buffer with triangles joining each pair of latitude rings.
        const size_t stride = horizontalSegments + 1;

        for (size_t i = 0; i < verticalSegments; i++)
        {
            for (size_t j = 0; j <= horizontalSegments; j++)
            {
                const size_t nextI = i + 1;
                const size_t nextJ = (j + 1) % stride;

                index_push_back(indices, i * stride + j);
                index_push_back(indices, nextI * stride + j);
                index_push_back(indices, i * stride + nextJ);

                index_push_back(indices, i * stride + nextJ);
                index_push_back(indices, nextI * stride + j);
                index_push_back(indices, nextI * stride + nextJ);
            }
        }

        // Build RH above
        if (!rhcoords)
            ReverseWinding(indices, vertices);

        if (invertn)
            InvertNormals(vertices);
    }
   /*void ComputeGeoSphere(VertexCollection& vertices, IndexCollection& indices, float diameter, size_t tessellation, bool rhcoords);
    void ComputeCylinder(VertexCollection& vertices, IndexCollection& indices, float height, float diameter, size_t tessellation, bool rhcoords);
    void ComputeCone(VertexCollection& vertices, IndexCollection& indices, float diameter, float height, size_t tessellation, bool rhcoords);
    void ComputeTorus(VertexCollection& vertices, IndexCollection& indices, float diameter, float thickness, size_t tessellation, bool rhcoords);
    void ComputeTetrahedron(VertexCollection& vertices, IndexCollection& indices, float size, bool rhcoords);
    void ComputeOctahedron(VertexCollection& vertices, IndexCollection& indices, float size, bool rhcoords);
    void ComputeDodecahedron(VertexCollection& vertices, IndexCollection& indices, float size, bool rhcoords);
    void ComputeIcosahedron(VertexCollection& vertices, IndexCollection& indices, float size, bool rhcoords);
    void ComputeTeapot(VertexCollection& vertices, IndexCollection& indices, float size, size_t tessellation, bool rhcoords);
  */    
  
}