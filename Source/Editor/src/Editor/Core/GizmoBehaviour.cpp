

#include "Editor/Core/GizmoBehaviour.h"
#include "Editor/Core/EditorActions.h"
#include "Editor/Settings/EditorSettings.h"

float SnapValue(float p_value, float p_step)
{
	return p_value - std::fmod(p_value, p_step);
}

bool Editor::Core::GizmoBehaviour::IsSnappedBehaviourEnabled() const
{
	using namespace Windowing::Inputs;

	const auto& inputManager = EDITOR_CONTEXT(inputManager);
	return inputManager->GetKeyState(EKey::KEY_LEFT_CONTROL) == EKeyState::KEY_DOWN || inputManager->GetKeyState(EKey::KEY_RIGHT_CONTROL) == EKeyState::KEY_DOWN;
}

void Editor::Core::GizmoBehaviour::StartPicking(::Core::ECS::Actor& p_target, const Maths::FVector3& p_cameraPosition, EGizmoOperation p_operation, EDirection p_direction)
{
	m_target = &p_target;
	m_firstMouse = true;
	m_originalTransform = p_target.transform.GetFTransform();
	m_distanceToActor = Maths::FVector3::Distance(p_cameraPosition, m_target->transform.GetWorldPosition());
	m_currentOperation = p_operation;
	m_direction = p_direction;
}

void Editor::Core::GizmoBehaviour::StopPicking()
{
	m_target = nullptr;
}

Maths::FVector3 Editor::Core::GizmoBehaviour::GetFakeDirection() const
{
	auto result = Maths::FVector3();

	switch (m_direction)
	{
	case Editor::Core::GizmoBehaviour::EDirection::X:
		result = Maths::FVector3::Right;
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Y:
		result = Maths::FVector3::Up;
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Z:
		result = Maths::FVector3::Forward;
		break;
	}

	return result;
}

Maths::FVector3 Editor::Core::GizmoBehaviour::GetRealDirection(bool p_relative) const
{
	auto result = Maths::FVector3();

	switch (m_direction)
	{
	case Editor::Core::GizmoBehaviour::EDirection::X:
		result = p_relative ? m_originalTransform.GetWorldRight() : m_originalTransform.GetLocalRight();
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Y:
		result = p_relative ? m_originalTransform.GetWorldUp() : m_originalTransform.GetLocalUp();
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Z:
		result = p_relative ? m_originalTransform.GetWorldForward() : m_originalTransform.GetLocalForward();
		break;
	}

	return result;
}

Maths::FVector2 Editor::Core::GizmoBehaviour::GetScreenDirection(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto start = m_originalTransform.GetWorldPosition();
	auto end = m_originalTransform.GetWorldPosition() + GetRealDirection(true) * 0.01f;

	auto start2D = Maths::FVector2();
	{
		auto clipSpacePos = p_projectionMatrix * (p_viewMatrix * Maths::FVector4{ start.x, start.y, start.z, 1.0f });
		auto ndcSpacePos = Maths::FVector3{ clipSpacePos.x, clipSpacePos.y, clipSpacePos.z } / clipSpacePos.w;
		auto windowSpacePos = ((Maths::FVector2{ ndcSpacePos.x, ndcSpacePos.y } + 1.0f) / 2.0f);
		windowSpacePos.x *= p_viewSize.x;
		windowSpacePos.y *= p_viewSize.y;
		start2D = windowSpacePos;
	}

	auto end2D = Maths::FVector2();
	{
		auto clipSpacePos = p_projectionMatrix * (p_viewMatrix * Maths::FVector4{ end.x, end.y, end.z, 1.0f });
		auto ndcSpacePos = Maths::FVector3{ clipSpacePos.x, clipSpacePos.y, clipSpacePos.z } / clipSpacePos.w;
		auto windowSpacePos = ((Maths::FVector2{ ndcSpacePos.x, ndcSpacePos.y } + 1.0f) / 2.0f);
		windowSpacePos.x *= p_viewSize.x;
		windowSpacePos.y *= p_viewSize.y;
		end2D = windowSpacePos;
	}

	auto result = end2D - start2D;

	result.y *= -1; // Screen coordinates are reversed, so we inverse the Y

	return Maths::FVector2::Normalize(result);
}

void Editor::Core::GizmoBehaviour::ApplyTranslation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto unitsPerPixel = 0.001f * m_distanceToActor;
	auto originPosition = m_originalTransform.GetLocalPosition();

	auto screenDirection = GetScreenDirection(p_viewMatrix, p_projectionMatrix, p_viewSize);

	auto totalDisplacement = m_currentMouse - m_originMouse;
	auto translationCoefficient = Maths::FVector2::Dot(totalDisplacement, screenDirection) * unitsPerPixel;

	if (IsSnappedBehaviourEnabled())
	{
		translationCoefficient = SnapValue(translationCoefficient, Editor::Settings::EditorSettings::TranslationSnapUnit);
	}

	m_target->transform.SetLocalPosition(originPosition + GetRealDirection() * translationCoefficient);
}

void Editor::Core::GizmoBehaviour::ApplyRotation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto unitsPerPixel = 0.2f;
	auto originRotation = m_originalTransform.GetLocalRotation();

	auto screenDirection = GetScreenDirection(p_viewMatrix, p_projectionMatrix, p_viewSize);
	screenDirection = Maths::FVector2(-screenDirection.y, screenDirection.x);

	auto totalDisplacement = m_currentMouse - m_originMouse;
	auto rotationCoefficient = Maths::FVector2::Dot(totalDisplacement, screenDirection) * unitsPerPixel;

	if (IsSnappedBehaviourEnabled())
	{
		rotationCoefficient = SnapValue(rotationCoefficient, Editor::Settings::EditorSettings::RotationSnapUnit);
	}

	auto rotationToApply = Maths::FQuaternion(Maths::FVector3(GetFakeDirection() * rotationCoefficient));
	m_target->transform.SetLocalRotation(originRotation * rotationToApply);
}

void Editor::Core::GizmoBehaviour::ApplyScale(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto unitsPerPixel = 0.01f;
	auto originScale = m_originalTransform.GetLocalScale();

	auto screenDirection = GetScreenDirection(p_viewMatrix, p_projectionMatrix, p_viewSize);

	auto totalDisplacement = m_currentMouse - m_originMouse;
	auto scaleCoefficient = Maths::FVector2::Dot(totalDisplacement, screenDirection) * unitsPerPixel;

	if (IsSnappedBehaviourEnabled())
	{
		scaleCoefficient = SnapValue(scaleCoefficient, Editor::Settings::EditorSettings::ScalingSnapUnit);
	}

	auto newScale = originScale + GetFakeDirection() * scaleCoefficient;

	/* Prevent scale from being negative*/
	newScale.x = std::max(newScale.x, 0.0001f);
	newScale.y = std::max(newScale.y, 0.0001f);
	newScale.z = std::max(newScale.z, 0.0001f);

	m_target->transform.SetLocalScale(newScale);
}

void Editor::Core::GizmoBehaviour::ApplyOperation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize)
{
	switch (m_currentOperation)
	{
	case EGizmoOperation::TRANSLATE:
		ApplyTranslation(p_viewMatrix, p_projectionMatrix, p_viewSize);
		break;

	case EGizmoOperation::ROTATE:
		ApplyRotation(p_viewMatrix, p_projectionMatrix, p_viewSize);
		break;

	case EGizmoOperation::SCALE:
		ApplyScale(p_viewMatrix, p_projectionMatrix, p_viewSize);
		break;
	}
}

void Editor::Core::GizmoBehaviour::SetCurrentMouse(const Maths::FVector2& p_mousePosition)
{
	if (m_firstMouse)
	{
		m_currentMouse = m_originMouse = p_mousePosition;
		m_firstMouse = false;
	}
	else
	{
		m_currentMouse = p_mousePosition;
	}
}

bool Editor::Core::GizmoBehaviour::IsPicking() const
{
	return m_target;
}

Editor::Core::GizmoBehaviour::EDirection Editor::Core::GizmoBehaviour::GetDirection() const
{
	return m_direction;
}