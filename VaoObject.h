#pragma once
#include"IndexBuffer.h"
#include"VertexBuffer.h"
#include"VertexTypes.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
namespace Opengl {

class VaoObject
{
public:
	VaoObject() = default;
	~VaoObject() = default;
	void Create() {
		if (!_name) {
			glGenVertexArrays(1, &_name);
		}
		vertexbuffer.Create();
		indexbuffer.Create();
	}
	IndexBuffer indexbuffer;
	VertexBuffer vertexbuffer;
	int m_vcount = 0;
	int m_icount = 0;
    unsigned _name=0;
	template<class T = VertexPositionNormalTexture>
	bool SetDataV(const void* data, unsigned size) {
		m_vcount = size/sizeof(T);
		return vertexbuffer.SetData(data,size);
	}
	bool SetDataI(const void* data, unsigned size) {
		m_icount = size/4;
		return indexbuffer.SetData(data,size);
	}
	void SetDynamic(bool flag) {
		indexbuffer.setdynamic_(flag);
		vertexbuffer.setdynamic_(flag);
	}	
	template<class T = VertexPositionNormalTexture>
	void SetVertexPointer() {
		glBindVertexArray(_name);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer._name);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,indexbuffer._name);
		for (unsigned i = 0; i < T::Elementnum; i++) {
		IpnutLayoutDesc t=	T::inputelements[i];
        glVertexAttribPointer(t.index,t.size,t.type,t.normalized,t.stride,t.pointer);
        glEnableVertexAttribArray(i);
		}
	}
	void Bind() {
		glBindVertexArray(_name);
	}
	void DrawElements() {
		Bind();
		glDrawElements(GL_TRIANGLES, m_icount, GL_UNSIGNED_INT, 0);
	}
	void Draw() {
		Bind();
		glDrawArrays(GL_TRIANGLES, 0,m_vcount);
	}
};
}

