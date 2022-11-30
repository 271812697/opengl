#include "IndexBuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
bool Opengl::IndexBuffer::SetData(const void* data, unsigned size)
{
	if (!data)
		return false;
	if (_name) {
		this->data = std::shared_ptr<unsigned char>(new unsigned char[size]);
		memcpy(this->data.get(),data,size);
		this->size = size;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _name);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, dynamic_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}
	return true;
}
void Opengl::IndexBuffer::Create()
{
	if (!_name) {
		glGenBuffers(1, &_name);
	}
}
