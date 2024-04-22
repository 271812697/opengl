
#pragma once

#include "Editor/Panels/AViewControllable.h"
#include "Editor/Core/GizmoBehaviour.h"
#include "Editor/Utils/ext.h"

namespace Editor::Panels
{
	class SceneView : public Editor::Panels::AViewControllable
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		SceneView
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings
		);
        void  _Draw_ImplInWindow()  override;
       // void _Draw_Impl() override;
		/**
		* Update the scene view
		*/
		virtual void Update(float p_deltaTime) override;

		/**
		* Custom implementation of the render method
		*/
		virtual void _Render_Impl() override;

		/**
		* Render the actual scene
		* @param p_defaultRenderState
		*/
		void RenderScene(uint8_t p_defaultRenderState);

		/**
		* Render the scene for actor picking (Using unlit colors)
		*/
		void RenderSceneForActorPicking();

		/**
		* Render the scene for actor picking and handle the logic behind it
		*/
		void HandleActorPicking();

	private:
		std::shared_ptr<asset::Shader>skys;
		std::shared_ptr<Rendering::Resources::Texture2D>env_map;
		std::shared_ptr<asset::CShader>bloom_shader;
		std::shared_ptr<asset::Shader>postprocess_shader;
		std::shared_ptr<asset::Shader>shadow_shader;
		unsigned int Quad = 0;

        std::unique_ptr<Rendering::Buffers::Framebuffer> m_mulfbo;
        std::unique_ptr<Rendering::Buffers::Framebuffer> m_resfbo;
        std::unique_ptr<Rendering::Buffers::Framebuffer> m_bloomfbo;
        std::array<std::unique_ptr<Rendering::Buffers::Framebuffer> ,7>m_shadowfbo;
		::Core::SceneSystem::SceneManager& m_sceneManager;
		std::unique_ptr<Rendering::Buffers::Framebuffer> m_actorPickingFramebuffer;
        unsigned m_currentOperation = 7;
		std::shared_ptr<asset::Texture>irradiance_map;
		std::shared_ptr<asset::Texture>prefiltered_map;
		std::shared_ptr<asset::Texture>BRDF_LUT;
		std::optional<std::reference_wrapper<::Core::ECS::Actor>> m_highlightedActor;
	};
}