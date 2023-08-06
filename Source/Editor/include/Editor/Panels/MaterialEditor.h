

#pragma once

#include <UI/Widgets/Texts/Text.h>
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Layout/Group.h>
#include <UI/Widgets/Layout/Columns.h>
#include <Rendering/Resources/Shader.h>

namespace Core::Resources { class Material; }

namespace Editor::Panels
{
	class MaterialEditor : public UI::Panels::PanelWindow
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		MaterialEditor
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings
		);

		/**
		* Refresh the material editor
		*/
		void Refresh();

		/**
		* Defines the target material of the material editor
		* @param p_newTarget
		*/
		void SetTarget(::Core::Resources::Material& p_newTarget);

		/**
		* Returns the target of the material editor
		*/
		::Core::Resources::Material* GetTarget() const;

		/**
		* Remove the target of the material editor (Clear the material editor)
		*/
		void RemoveTarget();

		/**
		* Launch the preview of the currently targeted material
		*/
		void Preview();

		/**
		* Reset material
		*/
		void Reset();
		
	private:
		void OnMaterialDropped();
		void OnShaderDropped();

		void CreateHeaderButtons();
		void CreateMaterialSelector();
		void CreateShaderSelector();
		void CreateMaterialSettings();
		void CreateShaderSettings();

		void GenerateShaderSettingsContent();
		void GenerateMaterialSettingsContent();

	private:
		::Core::Resources::Material* m_target		= nullptr;
		Rendering::Resources::Shader* m_shader	= nullptr;

		UI::Widgets::Texts::Text* m_targetMaterialText	= nullptr;
		UI::Widgets::Texts::Text* m_shaderText			= nullptr;

		Tools::Eventing::Event<> m_materialDroppedEvent;
		Tools::Eventing::Event<> m_shaderDroppedEvent;

		UI::Widgets::Layout::Group* m_settings			= nullptr;
		UI::Widgets::Layout::Group* m_materialSettings	= nullptr;
		UI::Widgets::Layout::Group* m_shaderSettings		= nullptr;

		UI::Widgets::Layout::Columns<2>* m_shaderSettingsColumns = nullptr;
		UI::Widgets::Layout::Columns<2>* m_materialSettingsColumns = nullptr;
	};
}