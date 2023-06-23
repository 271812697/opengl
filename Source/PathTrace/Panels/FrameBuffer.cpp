#include<glad/glad.h>
#include "FrameBuffer.h"

Framebuffer::Framebuffer(uint16_t p_width, uint16_t p_height)
    :width(p_width), height(p_height)
{
    glCreateFramebuffers(1, &m_bufferID);
}

Framebuffer::~Framebuffer()
{
    /* Destroy OpenGL objects */
    glDeleteFramebuffers(1, &m_bufferID);


}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);
}
void Framebuffer::Clear() const
{
    for (int i = 0; i < color_attachments.size(); i++) {
        Clear(i);
    }


}
void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint16_t p_width, uint16_t p_height)
{
 
    this->width = p_width;
    this->height = p_height;
    int cnt = color_attachments.size();
    color_attachments.clear();
    AddColorTexture(cnt);

    


}
void Framebuffer::AddColorTexture(uint16_t count) {
   
    size_t n_color_buffs = color_attachments.size();
    color_attachments.reserve(n_color_buffs + count);
    for (GLuint i = 0; i < count; i++) {
        GLenum target =GL_TEXTURE_2D;
        auto& texture = color_attachments.emplace_back(target, width, height, 1, GL_RGBA16F, 1);
        GLuint tid = texture.ID();

        static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        // we cannot set any of the sampler states for multisampled textures
        if (true) {
            glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTextureParameterfv(tid, GL_TEXTURE_BORDER_COLOR, border);
        }

        glNamedFramebufferTexture(m_bufferID, GL_COLOR_ATTACHMENT0 + n_color_buffs + i, tid, 0);

    }
    SetDrawBuffers();
}


void Framebuffer::Clear(GLint index)const {
    const GLfloat clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat clear_depth = 1.0f;
    const GLint clear_stencil = 0;

    // clear one of the color attachments
    if (index >= 0) {
        glClearNamedFramebufferfv(m_bufferID, GL_COLOR, index, clear_color);
    }
}
void Framebuffer::SetDrawBuffers() const
{
    if (size_t n = color_attachments.size(); n > 0) {
        GLenum* attachments = new GLenum[n];
        for (GLenum i = 0; i < n; i++) {
            *(attachments + i) = GL_COLOR_ATTACHMENT0 + i;
        }
        glNamedFramebufferDrawBuffers(m_bufferID, n, attachments);
        delete[] attachments;
    }
}

uint32_t Framebuffer::GetID() const
{
    return m_bufferID;
}


uint32_t Framebuffer::GetTextureID(uint16_t index)
{
    if (index < color_attachments.size())
        return color_attachments.at(index).ID();
    return 0;
}
