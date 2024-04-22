

#include"tools/PathParser.h"
#include <UI/Plugins/DDTarget.h>

#include "Editor/Core/EditorRenderer.h"
#include "Editor/Core/EditorActions.h"
#include "Editor/Panels/AssetView.h"

Editor::Panels::AssetView::AssetView
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) : AViewControllable(p_title, p_opened, p_windowSettings)
{
	m_camera.SetClearColor({ 0.098f, 0.098f, 0.098f });
	m_camera.SetFar(5000.0f);

	m_resource = static_cast<Rendering::Resources::Model*>(nullptr);
	m_image->AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [this](auto p_data)
	{
		std::string path = p_data.first;

		switch (Tools::Utils::PathParser::GetFileType(path))
		{
		case Tools::Utils::PathParser::EFileType::MODEL:
			if (auto resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().GetResource(path); resource)
				m_resource = resource;
			break;
		case Tools::Utils::PathParser::EFileType::TEXTURE:
			if (auto resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::TextureManager>().GetResource(path); resource)
				m_resource = resource;
			break;

		case Tools::Utils::PathParser::EFileType::MATERIAL:
			if (auto resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().GetResource(path); resource)
				m_resource = resource;
			break;
		}
	};
    m_resfbo = std::make_unique<Rendering::Buffers::Framebuffer>(1, 1);
    m_resfbo->AddColorTexture(1);
    m_resfbo->AddDepStRenderBuffer();
    glCreateVertexArrays(1, &Quad);
	auto& context = ::Core::Global::ServiceLocator::Get<::Editor::Core::Context>();
    postprocess_shader = std::make_shared<asset::Shader>(context.editorAssetsPath+"shaders\\post_process.glsl");

}

void Editor::Panels::AssetView::_Render_Impl()
{
	PrepareCamera();

	auto& baseRenderer = *EDITOR_CONTEXT(renderer).get();
    auto [winWidth, winHeight] = GetSafeSize();

    m_resfbo->Resize(winWidth, winHeight);
    m_resfbo->Bind();
	//m_fbo->Bind();

	baseRenderer.SetStencilMask(0xFF);
	baseRenderer.Clear(m_camera);
	baseRenderer.SetStencilMask(0x00);

	uint8_t glState = baseRenderer.FetchGLState();
	baseRenderer.ApplyStateMask(glState);

	m_editorRenderer.RenderGrid(m_cameraPosition, m_gridColor);

	if (auto pval = std::get_if<Rendering::Resources::Model*>(&m_resource); pval && *pval)
		m_editorRenderer.RenderModelAsset(**pval);
	
	if (auto pval = std::get_if<Rendering::Resources::Texture*>(&m_resource); pval && *pval)
		m_editorRenderer.RenderTextureAsset(**pval);
	 
     if (auto pval = std::get_if<::Core::Resources::Material*>(&m_resource); pval && *pval) {
 
		m_editorRenderer.RenderMaterialAsset(**pval);


    }


	

    m_resfbo->Unbind();
    //postprocess pass
    {
        m_fbo->Clear();
        m_fbo->Bind();
        m_resfbo->GetColorTexture(0).Bind(0);
       
        postprocess_shader->Bind();
        postprocess_shader->SetUniform(0, 3);  // select tone-mapping operator
        glBindVertexArray(Quad);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        postprocess_shader->Unbind();
        m_fbo->Unbind();
    }
    baseRenderer.ApplyStateMask(glState);
	//m_fbo->Unbind();
}

void Editor::Panels::AssetView::SetResource(ViewableResource p_resource)
{
	m_resource = p_resource;
}

Editor::Panels::AssetView::ViewableResource Editor::Panels::AssetView::GetResource() const
{
	return m_resource;
}
