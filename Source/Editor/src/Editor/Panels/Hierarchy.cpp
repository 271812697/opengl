

#include "Editor/Panels/Hierarchy.h"
#include "Editor/Core/EditorActions.h"

#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Plugins/DDSource.h>
#include <UI/Plugins/DDTarget.h>

#include <Debug/Logger.h>

#include <Core/Global/ServiceLocator.h>

#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CSpotLight.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CPhysicalBox.h>
#include <Core/ECS/Components/CPhysicalSphere.h>
#include <Core/ECS/Components/CPhysicalCapsule.h>
#include <Core/ECS/Components/CAudioSource.h>
#include <Core/ECS/Components/CAudioListener.h>

#include <UI/Plugins/ContextualMenu.h>

#include "Editor/Utils/ActorCreationMenu.h"

class HierarchyContextualMenu : public UI::Plugins::ContextualMenu
{
public:
    HierarchyContextualMenu(Core::ECS::Actor* p_target, UI::Widgets::Layout::TreeNode& p_treeNode, bool p_panelMenu = false) :
        m_target(p_target),
        m_treeNode(p_treeNode)
    {
        using namespace UI::Panels;
        using namespace UI::Widgets;
        using namespace UI::Widgets::Menu;
        using namespace Core::ECS::Components;

        if (m_target)
        {
            auto& focusButton = CreateWidget<UI::Widgets::Menu::MenuItem>("Focus");
            focusButton.ClickedEvent += [this]
            {
                EDITOR_EXEC(MoveToTarget(*m_target));
            };

            auto& duplicateButton = CreateWidget<UI::Widgets::Menu::MenuItem>("Duplicate");
            duplicateButton.ClickedEvent += [this]
            {
                EDITOR_EXEC(DelayAction(EDITOR_BIND(DuplicateActor, std::ref(*m_target), nullptr, true), 0));
            };

            auto& deleteButton = CreateWidget<UI::Widgets::Menu::MenuItem>("Delete");
            deleteButton.ClickedEvent += [this]
            {
                EDITOR_EXEC(DestroyActor(std::ref(*m_target)));
            };
        }

		auto& createActor = CreateWidget<UI::Widgets::Menu::MenuList>("Create...");
        Editor::Utils::ActorCreationMenu::GenerateActorCreationMenu(createActor, m_target, std::bind(&UI::Widgets::Layout::TreeNode::Open, &m_treeNode));
	}

	virtual void Execute() override
	{
		if (m_widgets.size() > 0)
			UI::Plugins::ContextualMenu::Execute();
	}

private:
	Core::ECS::Actor* m_target;
	UI::Widgets::Layout::TreeNode& m_treeNode;
};

void ExpandTreeNode(UI::Widgets::Layout::TreeNode& p_toExpand, const UI::Widgets::Layout::TreeNode* p_root)
{
	p_toExpand.Open();

	if (&p_toExpand != p_root && p_toExpand.HasParent())
	{
		ExpandTreeNode(*static_cast<UI::Widgets::Layout::TreeNode*>(p_toExpand.GetParent()), p_root);
	}
}

std::vector<UI::Widgets::Layout::TreeNode*> nodesToCollapse;
std::vector<UI::Widgets::Layout::TreeNode*> founds;

void ExpandTreeNodeAndEnable(UI::Widgets::Layout::TreeNode& p_toExpand, const UI::Widgets::Layout::TreeNode* p_root)
{
	if (!p_toExpand.IsOpened())
	{
		p_toExpand.Open();
		nodesToCollapse.push_back(&p_toExpand);
	}

	p_toExpand.enabled = true;

	if (&p_toExpand != p_root && p_toExpand.HasParent())
	{
		ExpandTreeNodeAndEnable(*static_cast<UI::Widgets::Layout::TreeNode*>(p_toExpand.GetParent()), p_root);
	}
}

Editor::Panels::Hierarchy::Hierarchy
(
	const std::string & p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) : PanelWindow(p_title, p_opened, p_windowSettings)
{
	auto& searchBar = CreateWidget<UI::Widgets::InputFields::InputText>();
	searchBar.ContentChangedEvent += [this](const std::string& p_content)
	{
		founds.clear();
		auto content = p_content;
		std::transform(content.begin(), content.end(), content.begin(), ::tolower);

		for (auto& [actor, item] : m_widgetActorLink)
		{
			if (!p_content.empty())
			{
				auto itemName = item->name;
				std::transform(itemName.begin(), itemName.end(), itemName.begin(), ::tolower);

				if (itemName.find(content) != std::string::npos)
				{
					founds.push_back(item);
				}

				item->enabled = false;
			}
			else
			{
				item->enabled = true;
			}
		}

		for (auto node : founds)
		{
			node->enabled = true;

			if (node->HasParent())
			{
				ExpandTreeNodeAndEnable(*static_cast<UI::Widgets::Layout::TreeNode*>(node->GetParent()), m_sceneRoot);
			}
		}

		if (p_content.empty())
		{
			for (auto node : nodesToCollapse)
			{
				node->Close();
			}

			nodesToCollapse.clear();
		}
	};

	m_sceneRoot = &CreateWidget<UI::Widgets::Layout::TreeNode>("Root", true);
	static_cast<UI::Widgets::Layout::TreeNode*>(m_sceneRoot)->Open();
	m_sceneRoot->AddPlugin<UI::Plugins::DDTarget<std::pair<::Core::ECS::Actor*, UI::Widgets::Layout::TreeNode*>>>("Actor").DataReceivedEvent += [this](std::pair<::Core::ECS::Actor*, UI::Widgets::Layout::TreeNode*> p_element)
	{
		if (p_element.second->HasParent())
			p_element.second->GetParent()->UnconsiderWidget(*p_element.second);

		m_sceneRoot->ConsiderWidget(*p_element.second);

		p_element.first->DetachFromParent();
	};
    m_sceneRoot->AddPlugin<HierarchyContextualMenu>(nullptr, *m_sceneRoot);

	EDITOR_EVENT(ActorUnselectedEvent) += std::bind(&Hierarchy::UnselectActorsWidgets, this);
	EDITOR_CONTEXT(sceneManager).SceneUnloadEvent += std::bind(&Hierarchy::Clear, this);
	::Core::ECS::Actor::CreatedEvent += std::bind(&Hierarchy::AddActorByInstance, this, std::placeholders::_1);
	::Core::ECS::Actor::DestroyedEvent += std::bind(&Hierarchy::DeleteActorByInstance, this, std::placeholders::_1);
	EDITOR_EVENT(ActorSelectedEvent) += std::bind(&Hierarchy::SelectActorByInstance, this, std::placeholders::_1);
	::Core::ECS::Actor::AttachEvent += std::bind(&Hierarchy::AttachActorToParent, this, std::placeholders::_1);
	::Core::ECS::Actor::DettachEvent += std::bind(&Hierarchy::DetachFromParent, this, std::placeholders::_1);
}

void Editor::Panels::Hierarchy::Clear()
{
	EDITOR_EXEC(UnselectActor());

	m_sceneRoot->RemoveAllWidgets();
	m_widgetActorLink.clear();
}

void Editor::Panels::Hierarchy::UnselectActorsWidgets()
{
	for (auto& widget : m_widgetActorLink)
		widget.second->selected = false;
}

void Editor::Panels::Hierarchy::SelectActorByInstance(::Core::ECS::Actor& p_actor)
{
	if (auto result = m_widgetActorLink.find(&p_actor); result != m_widgetActorLink.end())
		if (result->second)
			SelectActorByWidget(*result->second);
}

void Editor::Panels::Hierarchy::SelectActorByWidget(UI::Widgets::Layout::TreeNode & p_widget)
{
	UnselectActorsWidgets();

	p_widget.selected = true;

	if (p_widget.HasParent())
	{
		ExpandTreeNode(*static_cast<UI::Widgets::Layout::TreeNode*>(p_widget.GetParent()), m_sceneRoot);
	}
}

void Editor::Panels::Hierarchy::AttachActorToParent(::Core::ECS::Actor & p_actor)
{
	auto actorWidget = m_widgetActorLink.find(&p_actor);

	if (actorWidget != m_widgetActorLink.end())
	{
		auto widget = actorWidget->second;

		if (widget->HasParent())
			widget->GetParent()->UnconsiderWidget(*widget);

		if (p_actor.HasParent())
		{
			auto parentWidget = m_widgetActorLink.at(p_actor.GetParent());
			parentWidget->leaf = false;
			parentWidget->ConsiderWidget(*widget);
		}
	}
}

void Editor::Panels::Hierarchy::DetachFromParent(::Core::ECS::Actor & p_actor)
{
	if (auto actorWidget = m_widgetActorLink.find(&p_actor); actorWidget != m_widgetActorLink.end())
	{
		if (p_actor.HasParent() && p_actor.GetParent()->GetChildren().size() == 1)
		{
			if (auto parentWidget = m_widgetActorLink.find(p_actor.GetParent()); parentWidget != m_widgetActorLink.end())
			{
				parentWidget->second->leaf = true;
			}
		}

		auto widget = actorWidget->second;

		if (widget->HasParent())
			widget->GetParent()->UnconsiderWidget(*widget);

		m_sceneRoot->ConsiderWidget(*widget);
	}
}

void Editor::Panels::Hierarchy::DeleteActorByInstance(::Core::ECS::Actor& p_actor)
{
	if (auto result = m_widgetActorLink.find(&p_actor); result != m_widgetActorLink.end())
	{
		if (result->second)
		{
			result->second->Destroy();
		}

		m_widgetActorLink.erase(result);
	}
}

void Editor::Panels::Hierarchy::AddActorByInstance(::Core::ECS::Actor & p_actor)
{
	auto& textSelectable = m_sceneRoot->CreateWidget<UI::Widgets::Layout::TreeNode>(p_actor.GetName(), true);
	textSelectable.leaf = true;
	textSelectable.AddPlugin<HierarchyContextualMenu>(&p_actor, textSelectable);
	textSelectable.AddPlugin<UI::Plugins::DDSource<std::pair<::Core::ECS::Actor*, UI::Widgets::Layout::TreeNode*>>>("Actor", "Attach to...", std::make_pair(&p_actor, &textSelectable));
	textSelectable.AddPlugin<UI::Plugins::DDTarget<std::pair<::Core::ECS::Actor*, UI::Widgets::Layout::TreeNode*>>>("Actor").DataReceivedEvent += [&p_actor, &textSelectable](std::pair<::Core::ECS::Actor*, UI::Widgets::Layout::TreeNode*> p_element)
	{
		if (p_element.second->HasParent())
			p_element.second->GetParent()->UnconsiderWidget(*p_element.second);

		textSelectable.ConsiderWidget(*p_element.second);

		p_element.first->SetParent(p_actor);
	};
	auto& dispatcher = textSelectable.AddPlugin<UI::Plugins::DataDispatcher<std::string>>();

	::Core::ECS::Actor* targetPtr = &p_actor;
	dispatcher.RegisterGatherer([targetPtr] { return targetPtr->GetName(); });

	m_widgetActorLink[targetPtr] = &textSelectable;

	textSelectable.ClickedEvent += EDITOR_BIND(SelectActor, std::ref(p_actor));
	textSelectable.DoubleClickedEvent += EDITOR_BIND(MoveToTarget, std::ref(p_actor));
}