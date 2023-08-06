

#pragma once

#include <Rendering/Resources/Loaders/TextureLoader.h>

#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Layout/GroupCollapsable.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/ComboBox.h>

#include "Editor/Panels/Hierarchy.h"
#include "Editor/Panels/AssetBrowser.h"

namespace Editor::Panels
{
	class Inspector : public UI::Panels::PanelWindow
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		Inspector
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings
		);

		/**
		* Destructor
		*/
		~Inspector();

		/**
		* Focus the given actor
		* @param p_target
		*/
		void FocusActor(::Core::ECS::Actor& p_target);

		/**
		* Unfocus the currently targeted actor
		*/
		void UnFocus();

		/**
		* Unfocus the currently targeted actor without removing listeners attached to this actor
		*/
		void SoftUnFocus();

		/**
		* Returns the currently selected actor
		*/
		::Core::ECS::Actor* GetTargetActor() const;

		/**
		* Create the actor inspector for the given actor
		*/
		void CreateActorInspector(::Core::ECS::Actor& p_target);

		/**
		* Draw the given component in inspector
		*/
		void DrawComponent(::Core::ECS::Components::AComponent& p_component);

		/**
		* Draw the given behaviour in inspector
		*/
		void DrawBehaviour(::Core::ECS::Components::Behaviour& p_behaviour);

		/**
		* Refresh the inspector
		*/
		void Refresh();

	private:
		::Core::ECS::Actor* m_targetActor = nullptr;
		UI::Widgets::Layout::Group* m_actorInfo;
		UI::Widgets::Layout::Group* m_inspectorHeader;
		UI::Widgets::Selection::ComboBox* m_componentSelectorWidget;
        UI::Widgets::InputFields::InputText* m_scriptSelectorWidget;

		uint64_t m_componentAddedListener	= 0;
		uint64_t m_componentRemovedListener = 0;
		uint64_t m_behaviourAddedListener	= 0;
		uint64_t m_behaviourRemovedListener = 0;
		uint64_t m_destroyedListener		= 0;
	};
}