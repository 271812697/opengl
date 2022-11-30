#pragma once
#include "GPUObject.h"
#include "GraphicsDefs.h"
#include<iostream>
namespace Opengl {
    class  VertexBuffer : public GPUObject
    {
    public:
        VertexBuffer()=default;
        ~VertexBuffer()=default;
        bool SetData(const void* data,unsigned size);
        unsigned GetVertexCount() const { return vertexCount_; }
        unsigned GetVertexSize() const { return vertexSize_; }
        void Create();
        inline void setdynamic_(bool flag) { dynamic_ = flag; }
    private:
        bool dynamic_;
        unsigned vertexCount_{};
        unsigned vertexSize_{};
    };
}

