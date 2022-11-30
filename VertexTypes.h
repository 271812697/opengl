#pragma once
#include"Vector4.h"
namespace Opengl
{
    struct IpnutLayoutDesc {
        unsigned index;
        int size;
        unsigned type;
        unsigned char normalized;
        int stride;
        const void* pointer;
    };
    struct VertexPosition
    {
        VertexPosition() = default;

        VertexPosition(const VertexPosition&) = default;
        VertexPosition& operator=(const VertexPosition&) = default;

        VertexPosition(VertexPosition&&) = default;
        VertexPosition& operator=(VertexPosition&&) = default;

        VertexPosition(Vector3 const& iposition) noexcept
            : position(iposition)
        {
        }

 
        Vector3 position;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };


    // Vertex struct holding position and color information.
    struct VertexPositionColor
    {
        VertexPositionColor() = default;

        VertexPositionColor(const VertexPositionColor&) = default;
        VertexPositionColor& operator=(const VertexPositionColor&) = default;

        VertexPositionColor(VertexPositionColor&&) = default;
        VertexPositionColor& operator=(VertexPositionColor&&) = default;

        VertexPositionColor(Vector3 const& iposition, Vector4 const& icolor) noexcept
            : position(iposition),
            color(icolor)
        {
        }

    

        Vector3 position;
        Vector4 color;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };


    // Vertex struct holding position and texture mapping information.
    struct VertexPositionTexture
    {
        VertexPositionTexture() = default;

        VertexPositionTexture(const VertexPositionTexture&) = default;
        VertexPositionTexture& operator=(const VertexPositionTexture&) = default;

        VertexPositionTexture(VertexPositionTexture&&) = default;
        VertexPositionTexture& operator=(VertexPositionTexture&&) = default;

        VertexPositionTexture(Vector3 const& iposition, Vector2 const& itextureCoordinate) noexcept
            : position(iposition),
            textureCoordinate(itextureCoordinate)
        {
        }

     

        Vector3 position;
        Vector2 textureCoordinate;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };
    // Vertex struct holding position and normal vector.
    struct VertexPositionNormal
    {
        VertexPositionNormal() = default;

        VertexPositionNormal(const VertexPositionNormal&) = default;
        VertexPositionNormal& operator=(const VertexPositionNormal&) = default;

        VertexPositionNormal(VertexPositionNormal&&) = default;
        VertexPositionNormal& operator=(VertexPositionNormal&&) = default;

        VertexPositionNormal(Vector3 const& iposition, Vector3 const& inormal) noexcept
            : position(iposition),
            normal(inormal)
        {
        }
        Vector3 position;
        Vector3 normal;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };


    // Vertex struct holding position, color, and texture mapping information.
    struct VertexPositionColorTexture
    {
        VertexPositionColorTexture() = default;

        VertexPositionColorTexture(const VertexPositionColorTexture&) = default;
        VertexPositionColorTexture& operator=(const VertexPositionColorTexture&) = default;

        VertexPositionColorTexture(VertexPositionColorTexture&&) = default;
        VertexPositionColorTexture& operator=(VertexPositionColorTexture&&) = default;

        VertexPositionColorTexture(Vector3 const& iposition, Vector4 const& icolor, Vector2 const& itextureCoordinate) noexcept
            : position(iposition),
            color(icolor),
            textureCoordinate(itextureCoordinate)
        {
        }
        Vector3 position;
        Vector4 color;
        Vector2 textureCoordinate;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;

    };
    // Vertex struct holding position, normal vector, and color information.
    struct VertexPositionNormalColor
    {
        VertexPositionNormalColor() = default;

        VertexPositionNormalColor(const VertexPositionNormalColor&) = default;
        VertexPositionNormalColor& operator=(const VertexPositionNormalColor&) = default;

        VertexPositionNormalColor(VertexPositionNormalColor&&) = default;
        VertexPositionNormalColor& operator=(VertexPositionNormalColor&&) = default;

        VertexPositionNormalColor(Vector3 const& iposition, Vector3 const& inormal, Vector4 const& icolor) noexcept
            : position(iposition),
            normal(inormal),
            color(icolor)
        {
        }
        Vector3 position;
        Vector3 normal;
        Vector4 color;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };


    // Vertex struct holding position, normal vector, and texture mapping information.
    struct VertexPositionNormalTexture
    {
        VertexPositionNormalTexture() = default;

        VertexPositionNormalTexture(const VertexPositionNormalTexture&) = default;
        VertexPositionNormalTexture& operator=(const VertexPositionNormalTexture&) = default;

        VertexPositionNormalTexture(VertexPositionNormalTexture&&) = default;
        VertexPositionNormalTexture& operator=(VertexPositionNormalTexture&&) = default;

        VertexPositionNormalTexture(Vector3 const& iposition, Vector3 const& inormal, Vector2 const& itextureCoordinate) noexcept
            : position(iposition),
            normal(inormal),
            textureCoordinate(itextureCoordinate)
        {
        }
        Vector3 position;
        Vector3 normal;
        Vector2 textureCoordinate;
        static IpnutLayoutDesc inputelements[6];
        static unsigned Elementnum;
    };    
}
