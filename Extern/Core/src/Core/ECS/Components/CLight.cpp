

#include <UI/Widgets/Texts/Text.h>
#include <UI/Widgets/Drags/DragFloat.h>
#include <UI/Widgets/Selection/ColorEdit.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Layout/Group.h>

#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CLight.h"

Core::ECS::Components::CLight::CLight(ECS::Actor & p_owner) :
	AComponent(p_owner),
	m_data(p_owner.transform.GetFTransform(), {})
{
}

const Rendering::Entities::Light& Core::ECS::Components::CLight::GetData() const
{
	return m_data;
}

const Maths::FVector3& Core::ECS::Components::CLight::GetColor() const
{
	return m_data.color;
}

float Core::ECS::Components::CLight::GetIntensity() const
{
	return m_data.intensity;
}

void Core::ECS::Components::CLight::SetColor(const Maths::FVector3& p_color)
{
	m_data.color = p_color;
}

void Core::ECS::Components::CLight::SetIntensity(float p_intensity)
{
	m_data.intensity = p_intensity;
}

void Core::ECS::Components::CLight::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	using namespace Core::Helpers;

	Serializer::SerializeVec3(p_doc, p_node, "color", m_data.color);
	Serializer::SerializeFloat(p_doc, p_node, "intensity", m_data.intensity);
}

void Core::ECS::Components::CLight::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	using namespace Core::Helpers;

	Serializer::DeserializeVec3(p_doc, p_node, "color", m_data.color);
	Serializer::DeserializeFloat(p_doc, p_node, "intensity", m_data.intensity);
}

void Core::ECS::Components::CLight::OnInspector(UI::Internal::WidgetContainer& p_root)
{
	using namespace Core::Helpers;

	GUIDrawer::DrawColor(p_root, "Color", reinterpret_cast<UI::Types::Color&>(m_data.color));
	GUIDrawer::DrawScalar<float>(p_root, "Intensity", m_data.intensity, 0.005f, GUIDrawer::_MIN_FLOAT, GUIDrawer::_MAX_FLOAT);
}
