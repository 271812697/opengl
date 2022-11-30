#pragma once
#include "GPUObject.h"
#include "GraphicsDefs.h"
#include<iostream>
namespace Opengl {
class IndexBuffer :public GPUObject
{
public:
	IndexBuffer() = default;
	~IndexBuffer() = default;
	bool SetData(const void* data,unsigned size);
	std::shared_ptr<unsigned char>data;
	unsigned int size;
	void Create();
	inline void setdynamic_(bool flag) { dynamic_ = flag; }
private:
	bool dynamic_;
};

}


