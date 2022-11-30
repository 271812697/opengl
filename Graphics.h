#pragma once
#include<iostream>
#include"GraphicsDefs.h"
#include"Color.h"
namespace Opengl
{
    class Vector3;
    class Vector4;
    struct ScratchBuffer
    {
        ScratchBuffer() :
            size_(0),
            reserved_(false)
        {
        }
        std::shared_ptr<unsigned char> data_;
        unsigned size_;
        bool reserved_;
    };

    /// %Graphics subsystem. Manages the application window, rendering state and GPU resources.
    class  Graphics
    {
    public:
        Graphics() = default;
        ~Graphics() = default;
        void Clear(ClearTarget flags, const Color& color = Color(0.0f, 0.0f, 0.0f, 0.0f), float depth = 1.0f, unsigned stencil = 0);

        /// Draw non-indexed geometry.
        void Draw(PrimitiveType type, unsigned vertexStart, unsigned vertexCount);
        /// Draw indexed geometry.
        void Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount);
        /// Draw indexed geometry with vertex index offset.
        void Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex, unsigned vertexCount);
        /// Draw indexed, instanced geometry. An instancing vertex buffer must be set.
        void DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount,
            unsigned instanceCount);
        /// Draw indexed, instanced geometry with vertex index offset.
        void DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex,
            unsigned vertexCount, unsigned instanceCount);



    private:



    };
}

