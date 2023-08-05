
#include <UI/Widgets/InputFields/InputInt.h>
#include <UI/Widgets/Layout/Group.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Buttons/ButtonSmall.h>
#include <UI/Widgets/Texts/TextColored.h>
#include <UI/Plugins/DDTarget.h>

#include"../../tools/PathParser.h"

#include "Core/ECS/Actor.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CModelRenderer.h"
#include "Core/ResourceManagement/MaterialManager.h"
#include "Core/Global/ServiceLocator.h"

Core::ECS::Components::CMaterialRenderer::CMaterialRenderer(ECS::Actor & p_owner) : AComponent(p_owner)
{
	m_materials.fill(nullptr);

	for (uint8_t i = 0; i < MAX_MATERIAL_COUNT; ++i)
		m_materialFields[i].fill(nullptr);

	UpdateMaterialList();
}

std::string Core::ECS::Components::CMaterialRenderer::GetName()
{
	return "Material Renderer";
}

void Core::ECS::Components::CMaterialRenderer::FillWithMaterial(Core::Resources::Material & p_material)
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		m_materials[i] = &p_material;
}

void Core::ECS::Components::CMaterialRenderer::SetMaterialAtIndex(uint8_t p_index, Core::Resources::Material& p_material)
{
	m_materials[p_index] = &p_material;
}

Core::Resources::Material* Core::ECS::Components::CMaterialRenderer::GetMaterialAtIndex(uint8_t p_index)
{
	return m_materials.at(p_index);
}

void Core::ECS::Components::CMaterialRenderer::RemoveMaterialAtIndex(uint8_t p_index)
{
	if (p_index < m_materials.size())
	{
		m_materials[p_index] = nullptr;;
	}
}

void Core::ECS::Components::CMaterialRenderer::RemoveMaterialByInstance(Core::Resources::Material& p_instance)
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		if (m_materials[i] == &p_instance)
			m_materials[i] = nullptr;
}

void Core::ECS::Components::CMaterialRenderer::RemoveAllMaterials()
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		m_materials[i] = nullptr;
}

const Maths::FMatrix4 & Core::ECS::Components::CMaterialRenderer::GetUserMatrix() const
{
	return m_userMatrix;
}

const Core::ECS::Components::CMaterialRenderer::MaterialList& Core::ECS::Components::CMaterialRenderer::GetMaterials() const
{
	return m_materials;
}

void Core::ECS::Components::CMaterialRenderer::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	tinyxml2::XMLNode* materialsNode = p_doc.NewElement("materials");
	p_node->InsertEndChild(materialsNode);

	auto modelRenderer = owner.GetComponent<CModelRenderer>();
	uint8_t elementsToSerialize = modelRenderer->GetModel() ? (uint8_t)std::min(modelRenderer->GetModel()->GetMaterialNames().size(), (size_t)MAX_MATERIAL_COUNT) : 0;

	for (uint8_t i = 0; i < elementsToSerialize; ++i)
	{
		Core::Helpers::Serializer::SerializeMaterial(p_doc, materialsNode, "material", m_materials[i]);
	}
}

void Core::ECS::Components::CMaterialRenderer::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	tinyxml2::XMLNode* materialsRoot = p_node->FirstChildElement("materials");
	if (materialsRoot)
	{
		tinyxml2::XMLElement* currentMaterial = materialsRoot->FirstChildElement("material");

		uint8_t materialIndex = 0;

		while (currentMaterial)
		{
			if (auto material = Global::ServiceLocator::Get<ResourceManagement::MaterialManager>()[currentMaterial->GetText()])
				m_materials[materialIndex] = material;

			currentMaterial = currentMaterial->NextSiblingElement("material");
			++materialIndex;
		}
	}

	UpdateMaterialList();
}

std::array<UI::Widgets::AWidget*, 3> CustomMaterialDrawer(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Core::Resources::Material*& p_data)
{
	using namespace Core::Helpers;

	std::array<UI::Widgets::AWidget*, 3> widgets;

	widgets[0] = &p_root.CreateWidget<UI::Widgets::Texts::TextColored>(p_name, GUIDrawer::TitleColor);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto & rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto& widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

	widgets[1] = &widget;

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::MATERIAL)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::MaterialManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.content = p_receivedData.first;
			}
		}
	};

	widget.lineBreak = false;

	auto & resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
	resetButton.idleBackgroundColor = GUIDrawer::ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data]
	{
		p_data = nullptr;
		widget.content = "Empty";
	};

	widgets[2] = &resetButton;

	return widgets;
}

void Core::ECS::Components::CMaterialRenderer::OnInspector(UI::Internal::WidgetContainer & p_root)
{
	using namespace Core::Helpers;

	for (uint8_t i = 0; i < m_materials.size(); ++i)
		m_materialFields[i] = CustomMaterialDrawer(p_root, "Material", m_materials[i]);

	UpdateMaterialList();
}

void Core::ECS::Components::CMaterialRenderer::UpdateMaterialList()
{
	if (auto modelRenderer = owner.GetComponent<CModelRenderer>(); modelRenderer && modelRenderer->GetModel())
	{
		uint8_t materialIndex = 0;

		for (const std::string& materialName : modelRenderer->GetModel()->GetMaterialNames())
		{
			m_materialNames[materialIndex++] = materialName;
		}

		for (uint8_t i = materialIndex; i < MAX_MATERIAL_COUNT; ++i)
			m_materialNames[i] = "";
	}

	for (uint8_t i = 0; i < m_materialFields.size(); ++i)
	{
		if (m_materialFields[i][0])
		{
			bool enabled = !m_materialNames[i].empty();
			m_materialFields[i][0]->enabled = enabled;
			m_materialFields[i][1]->enabled = enabled;
			m_materialFields[i][2]->enabled = enabled;
			reinterpret_cast<UI::Widgets::Texts::Text*>(m_materialFields[i][0])->content = m_materialNames[i];
		}
	}
}

void Core::ECS::Components::CMaterialRenderer::SetUserMatrixElement(uint32_t p_row, uint32_t p_column, float p_value)
{
	if (p_row < 4 && p_column < 4)
		m_userMatrix.data[4 * p_row + p_column] = p_value;
}

float Core::ECS::Components::CMaterialRenderer::GetUserMatrixElement(uint32_t p_row, uint32_t p_column) const
{
	if (p_row < 4 && p_column < 4)
		return m_userMatrix.data[4 * p_row + p_column];
	else
		return 0.0f;
}
