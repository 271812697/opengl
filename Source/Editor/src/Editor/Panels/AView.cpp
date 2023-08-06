

#include <glad/glad.h>

#include "Editor/Panels/AView.h"
#include "Editor/Core/EditorActions.h"

Editor::Panels::AView::AView
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) : PanelWindow(p_title, p_opened, p_windowSettings), m_editorRenderer(EDITOR_RENDERER())
{
	m_cameraPosition = { -10.0f, 3.0f, 10.0f };
	m_cameraRotation = Maths::FQuaternion({0.0f, 135.0f, 0.0f});
	m_fbo = std::make_unique<Rendering::Buffers::Framebuffer>(1,1);
	m_fbo->AddColorTexture(1);
	m_fbo->AddDepStRenderBuffer();

	m_image = &CreateWidget<UI::Widgets::Visual::Image>(m_fbo->GetTextureID(), Maths::FVector2{ 0.f, 0.f });

    scrollable = false;
}

void Editor::Panels::AView::Update(float p_deltaTime)
{
	auto[winWidth, winHeight] = GetSafeSize();

	m_image->size = Maths::FVector2(static_cast<float>(winWidth), static_cast<float>(winHeight));

	m_fbo->Resize(winWidth, winHeight);
}

void Editor::Panels::AView::_Draw_Impl()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	UI::Panels::PanelWindow::_Draw_Impl();

	ImGui::PopStyleVar();
}

void Editor::Panels::AView::Render()
{
	FillEngineUBO();

	auto [winWidth, winHeight] = GetSafeSize();

	EDITOR_CONTEXT(shapeDrawer)->SetViewProjection(m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix());

	EDITOR_CONTEXT(renderer)->SetViewPort(0, 0, winWidth, winHeight);

	_Render_Impl();
}

void Editor::Panels::AView::SetCameraPosition(const Maths::FVector3 & p_position)
{
	m_cameraPosition = p_position;
}

void Editor::Panels::AView::SetCameraRotation(const Maths::FQuaternion& p_rotation)
{
	m_cameraRotation = p_rotation;
}

const Maths::FVector3 & Editor::Panels::AView::GetCameraPosition() const
{
	return m_cameraPosition;
}

const Maths::FQuaternion& Editor::Panels::AView::GetCameraRotation() const
{
	return m_cameraRotation;
}

Rendering::LowRenderer::Camera & Editor::Panels::AView::GetCamera()
{
	return m_camera;
}

std::pair<uint16_t, uint16_t> Editor::Panels::AView::GetSafeSize() const
{
	auto result = GetSize() - Maths::FVector2{ 0.f, 25.f }; // 25 == title bar height
	return { static_cast<uint16_t>(result.x), static_cast<uint16_t>(result.y) };
}

const Maths::FVector3& Editor::Panels::AView::GetGridColor() const
{
	return m_gridColor;
}

void Editor::Panels::AView::SetGridColor(const Maths::FVector3& p_color)
{
	m_gridColor = p_color;
}

void Editor::Panels::AView::FillEngineUBO()
{
	auto& engineUBO = *EDITOR_CONTEXT(engineUBO);

	size_t offset = sizeof(Maths::FMatrix4); // We skip the model matrix (Which is a special case, modified every draw calls)
	engineUBO.SetSubData(Maths::FMatrix4::Transpose(m_camera.GetViewMatrix()), std::ref(offset));
	engineUBO.SetSubData(Maths::FMatrix4::Transpose(m_camera.GetProjectionMatrix()), std::ref(offset));
	engineUBO.SetSubData(m_cameraPosition, std::ref(offset));
}

void Editor::Panels::AView::PrepareCamera()
{
	auto [winWidth, winHeight] = GetSafeSize();
	m_camera.CacheMatrices(winWidth, winHeight, m_cameraPosition, m_cameraRotation);
}
