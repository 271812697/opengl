#pragma once
#include"VaoObject.h"
#include"Node.h"
#include"Shader.h"
#include"asset/shader.h"
#include"asset/vao.h"
namespace Opengl{
class GameObject
{
public:
	Node* TransForm ;
	VaoObject* drawData ;
	GameObject() = default;
	GameObject(Node* t, VaoObject*p) {
		TransForm = t;
		drawData = p;
	}
	void Draw(const Shader& s) {
		auto it = TransForm->GetWorldTransform().ToMatrix4().Transpose();
		s.setMat4("world",it);
		drawData->DrawElements();
	}
    void Draw(const ::asset::Shader& shader) {
        auto it = TransForm->GetWorldTransform().ToMatrix4().Transpose();
        shader.SetUniform(3, it);
       // drawData->DrawElements();
        
      
        
    }


};
}
