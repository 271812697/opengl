

#pragma once

#include "Editor/Core/EditorActions.h"

namespace Editor::Core
{
	template<typename T>
	inline ::Core::ECS::Actor & EditorActions::CreateMonoComponentActor(bool p_focusOnCreation, ::Core::ECS::Actor* p_parent)
	{
		auto& instance = CreateEmptyActor(false, p_parent);

		T& component = instance.AddComponent<T>();

        instance.SetName(component.GetName());

		if (p_focusOnCreation)
			SelectActor(instance);

		return instance;
	}
}