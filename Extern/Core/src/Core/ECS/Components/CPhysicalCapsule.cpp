
#include <Physics/Entities/PhysicalCapsule.h>
#include <UI/Widgets/Drags/DragFloat.h>

#include "Core/ECS/Components/CPhysicalCapsule.h"
#include "Core/ECS/Actor.h"

using namespace Physics::Entities;

Core::ECS::Components::CPhysicalCapsule::CPhysicalCapsule(ECS::Actor & p_owner) :
	CPhysicalObject(p_owner)
{
	m_physicalObject = std::make_unique<PhysicalCapsule>(p_owner.transform.GetFTransform());

	m_physicalObject->SetUserData<std::reference_wrapper<CPhysicalObject>>(*this);

	BindListener();
	Init();
}

std::string Core::ECS::Components::CPhysicalCapsule::GetName()
{
	return "Physical Capsule";
}

void Core::ECS::Components::CPhysicalCapsule::SetRadius(float p_radius)
{
	GetPhysicalObjectAs<PhysicalCapsule>().SetRadius(p_radius);
}

void Core::ECS::Components::CPhysicalCapsule::SetHeight(float p_height)
{
	GetPhysicalObjectAs<PhysicalCapsule>().SetHeight(p_height);
}

float Core::ECS::Components::CPhysicalCapsule::GetRadius() const
{
	return GetPhysicalObjectAs<PhysicalCapsule>().GetRadius();
}

float Core::ECS::Components::CPhysicalCapsule::GetHeight() const
{
	return GetPhysicalObjectAs<PhysicalCapsule>().GetHeight();
}

void Core::ECS::Components::CPhysicalCapsule::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	CPhysicalObject::OnSerialize(p_doc, p_node);

	Helpers::Serializer::SerializeFloat(p_doc, p_node, "radius", GetRadius());
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "height", GetHeight());
}

void Core::ECS::Components::CPhysicalCapsule::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	CPhysicalObject::OnDeserialize(p_doc, p_node);

	SetRadius(Helpers::Serializer::DeserializeFloat(p_doc, p_node, "radius"));
	SetHeight(Helpers::Serializer::DeserializeFloat(p_doc, p_node, "height"));
}

void Core::ECS::Components::CPhysicalCapsule::OnInspector(UI::Internal::WidgetContainer & p_root)
{
	CPhysicalObject::OnInspector(p_root);

	Helpers::GUIDrawer::DrawScalar<float>(p_root, "Radius", std::bind(&CPhysicalCapsule::GetRadius, this), std::bind(&CPhysicalCapsule::SetRadius, this, std::placeholders::_1), 0.1f, 0.f, 100000.f);
	Helpers::GUIDrawer::DrawScalar<float>(p_root, "Height", std::bind(&CPhysicalCapsule::GetHeight, this), std::bind(&CPhysicalCapsule::SetHeight, this, std::placeholders::_1), 0.1f, 0.f, 100000.f);
}
