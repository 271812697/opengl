

#include <UI/Plugins/DDTarget.h>
#include "Editor/Core/EditorRenderer.h"
#include "Editor/Core/EditorActions.h"
#include "Editor/Panels/SceneView.h"
#include "Editor/Panels/GameView.h"
#include "Editor/Settings/EditorSettings.h"
#include "Opengl//asset/shader.h"
#include "Rendering/Resources/Texture2D.h"

std::shared_ptr<asset::Shader>skys;
std::shared_ptr<Rendering::Resources::Texture2D>env_map;
std::shared_ptr<asset::CShader>bloom_shader;
std::shared_ptr<asset::Shader>postprocess_shader;
std::shared_ptr<asset::Shader>shadow_shader;
unsigned int Quad=0;

Editor::Panels::SceneView::SceneView
(
    const std::string& p_title,
    bool p_opened,
    const UI::Settings::PanelWindowSettings& p_windowSettings
) : AViewControllable(p_title, p_opened, p_windowSettings, true),
m_sceneManager(EDITOR_CONTEXT(sceneManager))

{
    m_mulfbo = std::make_unique<Rendering::Buffers::Framebuffer>(1, 1);
    m_mulfbo->AddColorTexture(2, true);
    m_mulfbo->AddDepStRenderBuffer(true);

    m_resfbo = std::make_unique<Rendering::Buffers::Framebuffer>(1, 1);
    m_resfbo->AddColorTexture(2);

    m_bloomfbo = std::make_unique<Rendering::Buffers::Framebuffer>(1, 1);
    m_bloomfbo->AddColorTexture(2);
    for (int i = 0; i < m_shadowfbo.size(); i++) {
        m_shadowfbo[i] = std::make_unique<Rendering::Buffers::Framebuffer>(1024, 1024);
        
        m_shadowfbo[i]->AddDepthCubemap();
    }


    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    std::tie(irradiance_map, prefiltered_map, BRDF_LUT) =
        Ext::PrecomputeIBL("res\\texture\\HDRI\\Field-Path-Fence-Steinbacher-Street-4K.hdr");
    skys = std::make_shared<asset::Shader>("res\\shaders\\skybox01.glsl");
    skys->SetUniform(0, 1.0f);
    skys->SetUniform(1, 0.0f);

    glBindTextureUnit(0, irradiance_map->ID());
    glBindTextureUnit(1, prefiltered_map->ID());
    glBindTextureUnit(2, BRDF_LUT->ID());
   


    m_actorPickingFramebuffer = std::make_unique<Rendering::Buffers::Framebuffer>(1, 1);
    m_actorPickingFramebuffer->AddColorTexture(1);
    m_actorPickingFramebuffer->AddDepStRenderBuffer();
    m_camera.SetClearColor({ 0.098f, 0.098f, 0.098f });
    m_camera.SetFar(5000.0f);
    m_image->AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [this](auto p_data)
    {
        std::string path = p_data.first;

        switch (Tools::Utils::PathParser::GetFileType(path))
        {
        case Tools::Utils::PathParser::EFileType::SCENE:	EDITOR_EXEC(LoadSceneFromDisk(path));			break;
        case Tools::Utils::PathParser::EFileType::MODEL:	EDITOR_EXEC(CreateActorWithModel(path, true));	break;
        }
    };
    ;

    bloom_shader = std::make_shared<asset::CShader>("res\\shaders\\bloom.glsl");
    postprocess_shader = std::make_shared<asset::Shader>("res\\shaders\\post_process05.glsl");
    glCreateVertexArrays(1, &Quad);

    shadow_shader= std::make_shared<asset::Shader>("res\\shaders\\shadow.glsl");

}

void Editor::Panels::SceneView::Update(float p_deltaTime)
{
    AViewControllable::Update(p_deltaTime);
    PrepareCamera();

    using namespace Windowing::Inputs;

    if (IsFocused() && !m_cameraController.IsRightMousePressed())
    {
        if (EDITOR_CONTEXT(inputManager)->IsKeyPressed(EKey::KEY_W))
        {
            m_currentOperation = ImGuizmo::TRANSLATE;;
        }

        if (EDITOR_CONTEXT(inputManager)->IsKeyPressed(EKey::KEY_E))
        {
            m_currentOperation = ImGuizmo::ROTATE;
        }

        if (EDITOR_CONTEXT(inputManager)->IsKeyPressed(EKey::KEY_R))
        {
            m_currentOperation = ImGuizmo::SCALE;
        }
    }
}

void Editor::Panels::SceneView::_Render_Impl()
{


    auto& baseRenderer = *EDITOR_CONTEXT(renderer).get();

    uint8_t glState = baseRenderer.FetchGLState();
    baseRenderer.ApplyStateMask(glState);
    HandleActorPicking();
    baseRenderer.ApplyStateMask(glState);
    RenderScene(glState);
    baseRenderer.ApplyStateMask(glState);
}

void Editor::Panels::SceneView::RenderScene(uint8_t p_defaultRenderState)
{

    auto& baseRenderer = *EDITOR_CONTEXT(renderer).get();
    auto& currentScene = *m_sceneManager.GetCurrentScene();
    auto& gameView = EDITOR_PANEL(Editor::Panels::GameView, "Game View");

    // If the game is playing, and ShowLightFrustumCullingInSceneView is true, apply the game view frustum culling to the scene view (For debugging purposes)
    if (auto gameViewFrustum = gameView.GetActiveFrustum(); gameViewFrustum.has_value() && gameView.GetCamera().HasFrustumLightCulling() && Settings::EditorSettings::ShowLightFrustumCullingInSceneView)
    {
        m_editorRenderer.UpdateLightsInFrustum(currentScene, gameViewFrustum.value());
    }
    else
    {
        m_editorRenderer.UpdateLights(currentScene);
    }

    auto [winWidth, winHeight] = GetSafeSize();

    m_resfbo->Resize(winWidth, winHeight);
    m_bloomfbo->Resize(winWidth / 2, winHeight / 2);
    m_mulfbo->Resize(winWidth, winHeight);
    //Shadow Pass
    
    {
     auto scene=m_sceneManager.GetCurrentScene();
     auto& fac = scene->GetFastAccessComponents();
     auto projection = Maths::FMatrix4::CreatePerspective(90,1.0,0.1,100.0); 
     EDITOR_CONTEXT(renderer)->SetViewPort(0, 0,1024 ,1024);
     for (auto light : fac.lights) {
         if (light->GetData().type == 0.0) {
             unsigned index = light->GetData().shadowindex;
             m_shadowfbo[index]->Clear(-1);
             m_shadowfbo[index]->Bind();
             glEnable(GL_DEPTH_TEST);
             auto pos = light->owner.transform.GetWorldPosition();
             std::vector<Maths::FMatrix4> pl_transform = {
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x + 1, pos.y, pos.z, 0.0f, -1.0f, 0.0f)),
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x - 1, pos.y, pos.z, 0.0f, -1.0f, 0.0f)),
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x, pos.y + 1, pos.z, 0.0f, 0.0f, 1.0f)),
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x, pos.y - 1, pos.z, 0.0f, 0.0f, -1.0f)),
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x, pos.y, pos.z + 1, 0.0f, -1.0f, 0.0f)),
             Maths::FMatrix4::Transpose(projection * Maths::FMatrix4::CreateView(pos.x,pos.y,pos.z, pos.x, pos.y, pos.z - 1, 0.0f, -1.0f, 0.0f))
             };      
             shadow_shader->Bind();
             for (int i = 0; i < pl_transform.size(); i++) {
                 glProgramUniformMatrix4fv(shadow_shader->ID(), 250 + i, 1, GL_FALSE, pl_transform[i].data);
             }        
             glProgramUniform3fv(shadow_shader->ID(), 1008, 1, &pos.x);
             for (auto modelRenderer : fac.modelRenderers) {
                 if (modelRenderer->owner.IsActive())
                 {
                     const auto& transform=modelRenderer->owner.transform.GetFTransform();
                     ::Core::Global::ServiceLocator::Get<Editor::Core::EditorActions>().GetContext().engineUBO->SetSubData(Maths::FMatrix4::Transpose(transform.GetWorldMatrix()), 0);
                     if (auto model = modelRenderer->GetModel())
                     {
                         for (auto mesh : model->GetMeshes()) {
                             mesh->Bind();
                             if (mesh->GetIndexCount() > 0) {
                                glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
                             }
                             else {
                                 glDrawArrays(GL_TRIANGLES, 0, mesh->GetVertexCount());
                             }      
                             mesh->Unbind();
                         }
                     }
                 }
             }
             shadow_shader->Unbind();
             glDisable(GL_DEPTH_TEST);
             m_shadowfbo[index]->Unbind();
             m_shadowfbo[index]->GetDepthTexture().Bind(3+index);
         }  
     } 
     EDITOR_CONTEXT(renderer)->SetViewPort(0, 0,winWidth , winHeight);  

    }
    

    //MRT Pass
    {
    m_mulfbo->Clear();
    m_mulfbo->Bind();
    baseRenderer.SetStencilMask(0xFF);
    baseRenderer.Clear(m_camera);
    baseRenderer.SetStencilMask(0x00);
    baseRenderer.SetCapability(Rendering::Settings::ERenderingCapability::MULTISAMPLE, true);
    
    skys->Bind();
    auto model = EDITOR_CONTEXT(editorResources)->GetModel("Cube")->GetMeshes();
    for (auto mesh : model) {
        mesh->Bind();
        glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        mesh->Unbind();
    }
    skys->Unbind();
    m_editorRenderer.RenderGrid(m_cameraPosition, m_gridColor);
    m_editorRenderer.RenderCameras();
    // If the game is playing, and ShowGeometryFrustumCullingInSceneView is true, apply the game view frustum culling to the scene view (For debugging purposes)
    if (auto gameViewFrustum = gameView.GetActiveFrustum(); gameViewFrustum.has_value() && gameView.GetCamera().HasFrustumGeometryCulling() && Settings::EditorSettings::ShowGeometryFrustumCullingInSceneView)
    {
        m_camera.SetFrustumGeometryCulling(gameView.HasCamera() ? gameView.GetCamera().HasFrustumGeometryCulling() : false);
        m_editorRenderer.RenderScene(m_cameraPosition, m_camera, &gameViewFrustum.value());
        m_camera.SetFrustumGeometryCulling(false);
    }
    else
    {
        m_editorRenderer.RenderScene(m_cameraPosition, m_camera);
    }
    m_editorRenderer.RenderLights();
    if (EDITOR_EXEC(IsAnyActorSelected()))
    {
        auto& selectedActor = EDITOR_EXEC(GetSelectedActor());
        if (selectedActor.IsActive())
        {
            m_editorRenderer.RenderActorOutlinePass(selectedActor, true, true);
            baseRenderer.ApplyStateMask(p_defaultRenderState);
            m_editorRenderer.RenderActorOutlinePass(selectedActor, false, true);
        }
    }
    if (m_highlightedActor.has_value())
    {
        m_editorRenderer.RenderActorOutlinePass(m_highlightedActor.value().get(), true, false);
        baseRenderer.ApplyStateMask(p_defaultRenderState);
        m_editorRenderer.RenderActorOutlinePass(m_highlightedActor.value().get(), false, false);
    }
    m_mulfbo->Unbind();
    }
    //MSAA pass
    {
    Rendering::Buffers::Framebuffer::CopyColor(*m_mulfbo.get(), 0, *m_resfbo.get(), 0);
    Rendering::Buffers::Framebuffer::CopyColor(*m_mulfbo.get(), 1, *m_resfbo.get(), 1);
    }

    //Bloom pass
    {
    Rendering::Buffers::Framebuffer::CopyColor(*m_resfbo.get(), 1, *m_bloomfbo.get(), 0);
    auto& ping = m_bloomfbo->GetColorTexture(0);// .GetColorTexture(0);
    auto& pong = m_bloomfbo->GetColorTexture(1);
    bloom_shader->Bind();
    ping.BindILS(0, 0, GL_READ_WRITE);
    pong.BindILS(0, 1, GL_READ_WRITE);
    for (int i = 0; i < 6; ++i) {
        bloom_shader->SetUniform(0, i % 2 == 0);
        bloom_shader->Dispatch(30, 30);
        bloom_shader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }
    }

    //postprocess pass
    {
    m_fbo->Clear();
    m_fbo->Bind();
    m_resfbo->GetColorTexture(0).Bind(0);
    m_bloomfbo->GetColorTexture(0).Bind(1);
    postprocess_shader->Bind();
    postprocess_shader->SetUniform(0, 3);  // select tone-mapping operator
    glBindVertexArray(Quad);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    postprocess_shader->Unbind();
    m_fbo->Unbind();
    }
}
void Editor::Panels::SceneView::RenderSceneForActorPicking()
{
    auto& baseRenderer = *EDITOR_CONTEXT(renderer).get();
    auto [winWidth, winHeight] = GetSafeSize();
    m_actorPickingFramebuffer->Resize(winWidth, winHeight);
    m_actorPickingFramebuffer->Bind();
    baseRenderer.SetClearColor(1.0f, 1.0f, 1.0f);
    baseRenderer.Clear();
    m_editorRenderer.RenderSceneForActorPicking();
    m_actorPickingFramebuffer->Unbind();
}
void Editor::Panels::SceneView::_Draw_ImplInWindow() {

    if (EDITOR_EXEC(IsAnyActorSelected()))
    {
        auto& selectedActor = EDITOR_EXEC(GetSelectedActor());
        Maths::FMatrix4 model = selectedActor.transform.GetWorldMatrix();

        model = Maths::FMatrix4::Transpose(model);
        // ImGuizmo::BeginFrame();
        auto pos = this->GetPosition();
        auto [winWidth, winHeight] = GetSafeSize();
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(pos.x, pos.y + 25, winWidth, winHeight);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::Manipulate(Maths::FMatrix4::Transpose(m_camera.GetViewMatrix()).data,
            Maths::FMatrix4::Transpose(m_camera.GetProjectionMatrix()).data, static_cast<ImGuizmo::OPERATION>(m_currentOperation), ImGuizmo::WORLD,
            model.data);
        Maths::FVector3 p, r, s;
        if (selectedActor.HasParent()) {
            auto Local = Maths::FMatrix4::Inverse(selectedActor.GetParent()->transform.GetWorldMatrix()) * Maths::FMatrix4::Transpose(model);
            Local = Maths::FMatrix4::Transpose(Local);
            ImGuizmo::DecomposeMatrixToComponents(Local.data, (float*)&p, (float*)&r, (float*)&s);
        }
        else {
            ImGuizmo::DecomposeMatrixToComponents(model.data, (float*)&p, (float*)&r, (float*)&s);  
        }
        selectedActor.transform.GetFTransform().GenerateMatrices(p, r, s);
    }
}
bool IsResizing()
{
    auto cursor = ImGui::GetMouseCursor();

    return
        cursor == ImGuiMouseCursor_ResizeEW ||
        cursor == ImGuiMouseCursor_ResizeNS ||
        cursor == ImGuiMouseCursor_ResizeNWSE ||
        cursor == ImGuiMouseCursor_ResizeNESW ||
        cursor == ImGuiMouseCursor_ResizeAll;;
}
void Editor::Panels::SceneView::HandleActorPicking()
{
    using namespace Windowing::Inputs;
    auto& inputManager = *EDITOR_CONTEXT(inputManager);
    if (IsHovered() && !IsResizing())
    {
        RenderSceneForActorPicking();
        // Look actor under mouse
        auto [mouseX, mouseY] = inputManager.GetMousePosition();
        mouseX -= m_position.x;
        mouseY -= m_position.y;
        mouseY = GetSafeSize().second - mouseY + 25;
        m_actorPickingFramebuffer->Bind();
        uint8_t pixel[3];
        EDITOR_CONTEXT(renderer)->ReadPixels(static_cast<int>(mouseX), static_cast<int>(mouseY), 1, 1, Rendering::Settings::EPixelDataFormat::RGB, Rendering::Settings::EPixelDataType::UNSIGNED_BYTE, pixel);
        m_actorPickingFramebuffer->Unbind();
        uint32_t actorID = (0 << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0] << 0);
        auto actorUnderMouse = EDITOR_CONTEXT(sceneManager).GetCurrentScene()->FindActorByID(actorID);
        /* Click */
        if (inputManager.IsMouseButtonPressed(EMouseButton::MOUSE_BUTTON_LEFT) && !m_cameraController.IsRightMousePressed())
        {
                if (actorUnderMouse)
                {
                    EDITOR_EXEC(SelectActor(*actorUnderMouse));
                }
    
        }
        if(inputManager.IsMouseButtonPressed(EMouseButton::MOUSE_BUTTON_RIGHT)&&!actorUnderMouse)
        {
            EDITOR_EXEC(UnselectActor());
        }
        m_highlightedActor={};
        if (actorUnderMouse) {
            m_highlightedActor = *actorUnderMouse;
        }
    }
}
