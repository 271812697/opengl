

#pragma once

#include <Maths/FVector2.h>
#include <Maths/FVector3.h>
#include <Maths/FVector4.h>
#include <Maths/FQuaternion.h>

#include <UI/Internal/WidgetContainer.h>
#include <UI/Widgets/Texts/Text.h>
#include <UI/Widgets/Drags/DragSingleScalar.h>
#include <UI/Widgets/Drags/DragMultipleScalars.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Visual/Image.h>
#include <UI/Types/Color.h>



namespace Core::Resources
{
	class Material;
}

namespace Audio::Resources
{
	class Sound;
}

namespace Rendering::Resources
{
	class Model;
	class Shader;
	class Texture;
}

namespace Core::Helpers
{
	/**
	* Provide some helpers to draw UI elements
	*/
	class GUIDrawer
	{
	public:
		static const UI::Types::Color TitleColor;
		static const UI::Types::Color ClearButtonColor;

		static const float _MIN_FLOAT;
		static const float _MAX_FLOAT;

		/**
		* Defines the texture to use when there is no texture in a texture resource field
		* @param p_emptyTexture
		*/
		static void ProvideEmptyTexture(Rendering::Resources::Texture& p_emptyTexture);

		/**
		* Draw a title with the title color
		* @param p_root
		* @param p_name
		*/
		static void CreateTitle(UI::Internal::WidgetContainer& p_root, const std::string& p_name);
	
		template <typename T>
		static void DrawScalar(UI::Internal::WidgetContainer& p_root, const std::string& p_name, T& p_data, float p_step = 1.f, T p_min = std::numeric_limits<T>::min(), T p_max = std::numeric_limits<T>::max());
		static void DrawBoolean(UI::Internal::WidgetContainer& p_root, const std::string& p_name, bool& p_data);
        template <typename T, size_t _Size>
        static void DrawScalar();


		static void DrawVec2(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Maths::FVector2& p_data, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawVec3(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Maths::FVector3& p_data, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawVec4(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Maths::FVector4& p_data, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawQuat(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Maths::FQuaternion& p_data, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawString(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::string& p_data);
		static void DrawColor(UI::Internal::WidgetContainer& p_root, const std::string& p_name, UI::Types::Color& p_color, bool p_hasAlpha = false);
		static UI::Widgets::Texts::Text& DrawMesh(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Rendering::Resources::Model*& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);
		static UI::Widgets::Visual::Image& DrawTexture(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Rendering::Resources::Texture*& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);
		static UI::Widgets::Texts::Text& DrawShader(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Rendering::Resources::Shader*& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);
		static UI::Widgets::Texts::Text& DrawMaterial(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Core::Resources::Material*& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);
		static UI::Widgets::Texts::Text& DrawSound(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Audio::Resources::Sound*& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);
		static UI::Widgets::Texts::Text& DrawAsset(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::string& p_data, Tools::Eventing::Event<>* p_updateNotifier = nullptr);

		template <typename T>
		static void DrawScalar(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<T(void)> p_gatherer, std::function<void(T)> p_provider, float p_step = 1.f, T p_min = std::numeric_limits<T>::min(), T p_max = std::numeric_limits<T>::max());
		static void DrawBoolean(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<bool(void)> p_gatherer, std::function<void(bool)> p_provider);
		static void DrawVec2(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<Maths::FVector2(void)> p_gatherer, std::function<void(Maths::FVector2)> p_provider, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawVec3(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<Maths::FVector3(void)> p_gatherer, std::function<void(Maths::FVector3)> p_provider, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawVec4(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<Maths::FVector4(void)> p_gatherer, std::function<void(Maths::FVector4)> p_provider, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawQuat(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<Maths::FQuaternion(void)> p_gatherer, std::function<void(Maths::FQuaternion)> p_provider, float p_step = 1.f, float p_min = _MIN_FLOAT, float p_max = _MAX_FLOAT);
		static void DrawDDString(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<std::string(void)> p_gatherer, std::function<void(std::string)> p_provider, const std::string& p_identifier);
		static void DrawString(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<std::string(void)> p_gatherer, std::function<void(std::string)> p_provider);
		static void DrawColor(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::function<UI::Types::Color(void)> p_gatherer, std::function<void(UI::Types::Color)> p_provider, bool p_hasAlpha = false);

		template <typename T>
		static ImGuiDataType_ GetDataType();

		template <typename T>
		static std::string GetFormat();

	private:
		static Rendering::Resources::Texture* __EMPTY_TEXTURE;
	};
}

#include "Core/Helpers/GUIDrawer.inl"