#include "VertexBuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
bool Opengl::VertexBuffer::SetData(const void* data,unsigned size)
{
    if (!data )
        return false;
 
    if (_name) {
        glBindBuffer(GL_ARRAY_BUFFER,_name);
        glBufferData(GL_ARRAY_BUFFER, size, data, dynamic_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }
    return true;
}
void Opengl::VertexBuffer::Create()
{
    if (!_name) {
        glGenBuffers(1,&_name);
    }
}
