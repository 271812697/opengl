#pragma once
#include"Opengl/asset/texture.h"
#include<memory>
#include <vector>
class Framebuffer
{
public:

	Framebuffer(uint16_t p_width = 0, uint16_t p_height = 0);


	~Framebuffer();

	void Bind();
	void Clear() const;

	void Unbind();


	void Resize(uint16_t p_width, uint16_t p_height);


	void AddColorTexture(uint16_t count);

	void SetDrawBuffers() const;

	uint32_t GetID()const;

	void Clear(GLint index) const;

	uint32_t GetTextureID(uint16_t index = 0);

    uint32_t width = 0, height = 0;
private:
	
	std::vector<asset::Texture>color_attachments;
	uint32_t m_bufferID = 0;
};