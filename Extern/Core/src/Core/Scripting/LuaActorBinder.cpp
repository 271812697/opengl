

#include "Core/Scripting/LuaActorBinder.h"

#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CTransform.h"
#include "Core/ECS/Components/CCamera.h"
#include "Core/ECS/Components/CPhysicalBox.h"
#include "Core/ECS/Components/CPhysicalSphere.h"
#include "Core/ECS/Components/CPhysicalCapsule.h"
#include "Core/ECS/Components/CDirectionalLight.h"
#include "Core/ECS/Components/CPointLight.h"
#include "Core/ECS/Components/CSpotLight.h"
#include "Core/ECS/Components/CAmbientBoxLight.h"
#include "Core/ECS/Components/CAmbientSphereLight.h"
#include "Core/ECS/Components/CModelRenderer.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CAudioSource.h"
#include "Core/ECS/Components/CAudioListener.h"

void Core::Scripting::LuaActorBinder::BindActor(sol::state & p_luaState)
{
	using namespace Core::ECS;
	using namespace Core::ECS::Components;

	p_luaState.new_usertype<Actor>("Actor",
		/* Methods */
		"GetName", &Actor::GetName,
		"SetName", &Actor::SetName,
		"GetTag", &Actor::GetTag,
		"GetChildren", &Actor::GetChildren,
		"SetTag", &Actor::SetTag,
		"GetID", &Actor::GetID,
		"GetParent", &Actor::GetParent,
		"SetParent", &Actor::SetParent,
		"DetachFromParent", &Actor::DetachFromParent,
		"Destroy", &Actor::MarkAsDestroy,
		"IsSelfActive", &Actor::IsSelfActive, // TODO: Add to doc
		"IsActive", &Actor::IsActive,
		"SetActive", &Actor::SetActive,


		/* Components Getters */
		"GetTransform", &Actor::GetComponent<CTransform>,
		"GetPhysicalObject", &Actor::GetComponent<CPhysicalObject>,
		"GetPhysicalBox", &Actor::GetComponent<CPhysicalBox>,
		"GetPhysicalSphere", &Actor::GetComponent<CPhysicalSphere>,
		"GetPhysicalCapsule", &Actor::GetComponent<CPhysicalCapsule>,
		"GetCamera", &Actor::GetComponent<CCamera>,
		"GetLight", &Actor::GetComponent<CLight>,
		"GetPointLight", &Actor::GetComponent<CPointLight>,
		"GetSpotLight", &Actor::GetComponent<CSpotLight>,
		"GetDirectionalLight", &Actor::GetComponent<CDirectionalLight>,
		"GetAmbientBoxLight", &Actor::GetComponent<CAmbientBoxLight>,
		"GetAmbientSphereLight", &Actor::GetComponent<CAmbientSphereLight>,
		"GetModelRenderer", &Actor::GetComponent<CModelRenderer>,
		"GetMaterialRenderer", &Actor::GetComponent<CMaterialRenderer>,
		"GetAudioSource", &Actor::GetComponent<CAudioSource>,
		"GetAudioListener", &Actor::GetComponent<CAudioListener>,

		/* Behaviours relatives */
		"GetBehaviour", [](Actor& p_this, const std::string& p_name) -> sol::table
		{
			auto behaviour = p_this.GetBehaviour(p_name);
			if (behaviour)
				return behaviour->GetTable();
			else
				return sol::nil;
		},

		/* Components Creators */
		"AddTransform", &Actor::AddComponent<CTransform>,
		"AddModelRenderer", &Actor::AddComponent<CModelRenderer>,
		"AddPhysicalBox", &Actor::AddComponent<CPhysicalBox>,
		"AddPhysicalSphere", &Actor::AddComponent<CPhysicalSphere>,
		"AddPhysicalCapsule", &Actor::AddComponent<CPhysicalCapsule>,
		"AddCamera", &Actor::AddComponent<CCamera>,
		"AddPointLight", &Actor::AddComponent<CPointLight>,
		"AddSpotLight", &Actor::AddComponent<CSpotLight>,
		"AddDirectionalLight", &Actor::AddComponent<CDirectionalLight>,
		"AddAmbientBoxLight", &Actor::AddComponent<CAmbientBoxLight>,
		"AddAmbientSphereLight", &Actor::AddComponent<CAmbientSphereLight>,
		"AddMaterialRenderer", &Actor::AddComponent<CMaterialRenderer>,
		"AddAudioSource", &Actor::AddComponent<CAudioSource>,
		"AddAudioListener", &Actor::AddComponent<CAudioListener>,

		/* Components Destructors */
		"RemoveModelRenderer", &Actor::RemoveComponent<CModelRenderer>,
		"RemovePhysicalBox", &Actor::RemoveComponent<CPhysicalBox>,
		"RemovePhysicalSphere", &Actor::RemoveComponent<CPhysicalSphere>,
		"RemovePhysicalCapsule", &Actor::RemoveComponent<CPhysicalCapsule>,
		"RemoveCamera", &Actor::RemoveComponent<CCamera>,
		"RemovePointLight", &Actor::RemoveComponent<CPointLight>,
		"RemoveSpotLight", &Actor::RemoveComponent<CSpotLight>,
		"RemoveDirectionalLight", &Actor::RemoveComponent<CDirectionalLight>,
		"RemoveAmbientBoxLight", &Actor::RemoveComponent<CAmbientBoxLight>,
		"RemoveAmbientSphereLight", &Actor::RemoveComponent<CAmbientSphereLight>,
		"RemoveMaterialRenderer", &Actor::RemoveComponent<CMaterialRenderer>,
		"RemoveAudioSource", &Actor::RemoveComponent<CAudioSource>,
		"RemoveAudioListener", &Actor::RemoveComponent<CAudioListener>,

		/* Behaviour management */
		"AddBehaviour", &Actor::AddBehaviour,
		"RemoveBehaviour", sol::overload
		(
			sol::resolve<bool(Behaviour&)>(&Actor::RemoveBehaviour),
			sol::resolve<bool(const std::string&)>(&Actor::RemoveBehaviour)
		)
	);
}
