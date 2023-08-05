

#include <array>

#include"../../tools/PathParser.h"

#include <UI/Widgets/Texts/TextColored.h>
#include <UI/Widgets/Drags/DragSingleScalar.h>
#include <UI/Widgets/Drags/DragMultipleScalars.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Selection/ColorEdit.h>
#include <UI/Widgets/Layout/Group.h>
#include <UI/Widgets/Layout/Columns.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Buttons/ButtonSmall.h>
#include <UI/Plugins/DDTarget.h>

#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/SoundManager.h>

#include "Core/Helpers/GUIDrawer.h"

const UI::Types::Color Core::Helpers::GUIDrawer::TitleColor = { 0.85f, 0.65f, 0.0f };
const UI::Types::Color Core::Helpers::GUIDrawer::ClearButtonColor = { 0.5f, 0.0f, 0.0f };
const float Core::Helpers::GUIDrawer::_MIN_FLOAT = -999999999.f;
const float Core::Helpers::GUIDrawer::_MAX_FLOAT = +999999999.f;
Rendering::Resources::Texture* Core::Helpers::GUIDrawer::__EMPTY_TEXTURE = nullptr;

void Core::Helpers::GUIDrawer::ProvideEmptyTexture(Rendering::Resources::Texture& p_emptyTexture)
{
	__EMPTY_TEXTURE = &p_emptyTexture;
}

void Core::Helpers::GUIDrawer::CreateTitle(UI::Internal::WidgetContainer& p_root, const std::string & p_name)
{
	p_root.CreateWidget<UI::Widgets::Texts::TextColored>(p_name, TitleColor);
}

void Core::Helpers::GUIDrawer::DrawBoolean(UI::Internal::WidgetContainer & p_root, const std::string & p_name, bool & p_data)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Selection::CheckBox>();
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<bool>>();
	dispatcher.RegisterReference(reinterpret_cast<bool&>(p_data));
}

void Core::Helpers::GUIDrawer::DrawVec2(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Maths::FVector2 & p_data, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 2>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 2>>>();
	dispatcher.RegisterReference(reinterpret_cast<std::array<float, 2>&>(p_data));
}

void Core::Helpers::GUIDrawer::DrawVec3(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Maths::FVector3 & p_data, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 3>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 3>>>();
	dispatcher.RegisterReference(reinterpret_cast<std::array<float, 3>&>(p_data));
}

void Core::Helpers::GUIDrawer::DrawVec4(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Maths::FVector4& p_data, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 4>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 4>>>();
	dispatcher.RegisterReference(reinterpret_cast<std::array<float, 4>&>(p_data));
}

void Core::Helpers::GUIDrawer::DrawQuat(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Maths::FQuaternion & p_data, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 4>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 4>>>();
	dispatcher.RegisterReference(reinterpret_cast<std::array<float, 4>&>(p_data));
}

void Core::Helpers::GUIDrawer::DrawString(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::string & p_data)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::InputFields::InputText>("");
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::string>>();
	dispatcher.RegisterReference(p_data);
}

void Core::Helpers::GUIDrawer::DrawColor(UI::Internal::WidgetContainer & p_root, const std::string & p_name, UI::Types::Color & p_color, bool p_hasAlpha)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Selection::ColorEdit>(p_hasAlpha);
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<UI::Types::Color>>();
	dispatcher.RegisterReference(p_color);
}

UI::Widgets::Texts::Text& Core::Helpers::GUIDrawer::DrawMesh(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Rendering::Resources::Model *& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
	CreateTitle(p_root, p_name);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto& rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto& widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::MODEL)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::ModelManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.content = p_receivedData.first;
				if (p_updateNotifier)
					p_updateNotifier->Invoke();
			}
		}
	};

	widget.lineBreak = false;

	auto& resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
	resetButton.idleBackgroundColor = ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
	{
		p_data = nullptr;
		widget.content = "Empty";
		if (p_updateNotifier)
			p_updateNotifier->Invoke();
	};

	return widget;
}

UI::Widgets::Visual::Image& Core::Helpers::GUIDrawer::DrawTexture(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Rendering::Resources::Texture *& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
	CreateTitle(p_root, p_name);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto& rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto& widget = rightSide.CreateWidget<UI::Widgets::Visual::Image>(p_data ? p_data->id : (__EMPTY_TEXTURE ? __EMPTY_TEXTURE->id : 0), Maths::FVector2{ 75, 75 });

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::TEXTURE)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::TextureManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.textureID.id = resource->id;
				if (p_updateNotifier)
					p_updateNotifier->Invoke();
			}
		}
	};

	widget.lineBreak = false;

	auto& resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::Button>("Clear");
	resetButton.idleBackgroundColor = ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
	{
		p_data = nullptr;
		widget.textureID.id = (__EMPTY_TEXTURE ? __EMPTY_TEXTURE->id : 0);
		if (p_updateNotifier)
			p_updateNotifier->Invoke();
	};

	return widget;
}

UI::Widgets::Texts::Text& Core::Helpers::GUIDrawer::DrawShader(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Rendering::Resources::Shader *& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
	CreateTitle(p_root, p_name);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto& rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto& widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::SHADER)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::ShaderManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.content = p_receivedData.first;
				if (p_updateNotifier)
					p_updateNotifier->Invoke();
			}
		}
	};

	widget.lineBreak = false;

	auto& resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
	resetButton.idleBackgroundColor = ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
	{
		p_data = nullptr;
		widget.content = "Empty";
		if (p_updateNotifier)
			p_updateNotifier->Invoke();
	};

	return widget;
}

UI::Widgets::Texts::Text& Core::Helpers::GUIDrawer::DrawMaterial(UI::Internal::WidgetContainer & p_root, const std::string & p_name, Core::Resources::Material *& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
	CreateTitle(p_root, p_name);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto& rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto& widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::MATERIAL)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::MaterialManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.content = p_receivedData.first;
				if (p_updateNotifier)
					p_updateNotifier->Invoke();
			}
		}
	};

	widget.lineBreak = false;

	auto& resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
	resetButton.idleBackgroundColor = ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
	{
		p_data = nullptr;
		widget.content = "Empty";
		if (p_updateNotifier)
			p_updateNotifier->Invoke();
	};

	return widget;
}

UI::Widgets::Texts::Text& Core::Helpers::GUIDrawer::DrawSound(UI::Internal::WidgetContainer& p_root, const std::string& p_name, Audio::Resources::Sound*& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
	CreateTitle(p_root, p_name);

	std::string displayedText = (p_data ? p_data->path : std::string("Empty"));
	auto & rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

	auto & widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

	widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
	{
		if (Tools::Utils::PathParser::GetFileType(p_receivedData.first) == Tools::Utils::PathParser::EFileType::SOUND)
		{
			if (auto resource = OVSERVICE(Core::ResourceManagement::SoundManager).GetResource(p_receivedData.first); resource)
			{
				p_data = resource;
				widget.content = p_receivedData.first;
				if (p_updateNotifier)
					p_updateNotifier->Invoke();
			}
		}
	};

	widget.lineBreak = false;

	auto & resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
	resetButton.idleBackgroundColor = ClearButtonColor;
	resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
	{
		p_data = nullptr;
		widget.content = "Empty";
		if (p_updateNotifier)
			p_updateNotifier->Invoke();
	};

	return widget;
}

UI::Widgets::Texts::Text& Core::Helpers::GUIDrawer::DrawAsset(UI::Internal::WidgetContainer& p_root, const std::string& p_name, std::string& p_data, Tools::Eventing::Event<>* p_updateNotifier)
{
    CreateTitle(p_root, p_name);

    const std::string displayedText = (p_data.empty() ? std::string("Empty") : p_data);
    auto& rightSide = p_root.CreateWidget<UI::Widgets::Layout::Group>();

    auto& widget = rightSide.CreateWidget<UI::Widgets::Texts::Text>(displayedText);

    widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>("File").DataReceivedEvent += [&widget, &p_data, p_updateNotifier](auto p_receivedData)
    {
        p_data = p_receivedData.first;
        widget.content = p_receivedData.first;
        if (p_updateNotifier)
            p_updateNotifier->Invoke();
    };

    widget.lineBreak = false;

    auto& resetButton = rightSide.CreateWidget<UI::Widgets::Buttons::ButtonSmall>("Clear");
    resetButton.idleBackgroundColor = ClearButtonColor;
    resetButton.ClickedEvent += [&widget, &p_data, p_updateNotifier]
    {
        p_data = "";
        widget.content = "Empty";
        if (p_updateNotifier)
            p_updateNotifier->Invoke();
    };

    return widget;
}

void Core::Helpers::GUIDrawer::DrawBoolean(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<bool(void)> p_gatherer, std::function<void(bool)> p_provider)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Selection::CheckBox>();
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<bool>>();

	dispatcher.RegisterGatherer([p_gatherer]()
	{
		bool value = p_gatherer();
		return reinterpret_cast<bool&>(value);
	});

	dispatcher.RegisterProvider([p_provider](bool p_value)
	{
		p_provider(reinterpret_cast<bool&>(p_value));
	});
}

void Core::Helpers::GUIDrawer::DrawVec2(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<Maths::FVector2(void)> p_gatherer, std::function<void(Maths::FVector2)> p_provider, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 2>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 2>>>();
	
	dispatcher.RegisterGatherer([p_gatherer]()
	{
		Maths::FVector2 value = p_gatherer();
		return reinterpret_cast<const std::array<float, 2>&>(value);
	});

	dispatcher.RegisterProvider([p_provider](std::array<float, 2> p_value)
	{
		p_provider(reinterpret_cast<Maths::FVector2&>(p_value));
	});
}

void Core::Helpers::GUIDrawer::DrawVec3(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<Maths::FVector3(void)> p_gatherer, std::function<void(Maths::FVector3)> p_provider, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 3>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 3>>>();

	dispatcher.RegisterGatherer([p_gatherer]()
	{
		Maths::FVector3 value = p_gatherer();
		return reinterpret_cast<const std::array<float, 3>&>(value);
	});

	dispatcher.RegisterProvider([p_provider](std::array<float, 3> p_value)
	{
		p_provider(reinterpret_cast<Maths::FVector3&>(p_value));
	});
}

void Core::Helpers::GUIDrawer::DrawVec4(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<Maths::FVector4(void)> p_gatherer, std::function<void(Maths::FVector4)> p_provider, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 4>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 4>>>();
	
	dispatcher.RegisterGatherer([p_gatherer]()
	{
		Maths::FVector4 value = p_gatherer();
		return reinterpret_cast<const std::array<float, 4>&>(value);
	});

	dispatcher.RegisterProvider([p_provider](std::array<float, 4> p_value)
	{
		p_provider(reinterpret_cast<Maths::FVector4&>(p_value));
	});
}

void Core::Helpers::GUIDrawer::DrawQuat(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<Maths::FQuaternion(void)> p_gatherer, std::function<void(Maths::FQuaternion)> p_provider, float p_step, float p_min, float p_max)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Drags::DragMultipleScalars<float, 4>>(GetDataType<float>(), p_min, p_max, 0.f, p_step, "", GetFormat<float>());
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::array<float, 4>>>();
	
	dispatcher.RegisterGatherer([p_gatherer]()
	{
		Maths::FQuaternion value = p_gatherer();
		return reinterpret_cast<const std::array<float, 4>&>(value);
	});

	dispatcher.RegisterProvider([&dispatcher, p_provider](std::array<float, 4> p_value)
	{
		p_provider(Maths::FQuaternion::Normalize(reinterpret_cast<Maths::FQuaternion&>(p_value)));
	});
}

void Core::Helpers::GUIDrawer::DrawDDString(UI::Internal::WidgetContainer& p_root, const std::string& p_name,
	std::function<std::string()> p_gatherer, std::function<void(std::string)> p_provider,
	const std::string& p_identifier)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::InputFields::InputText>("");
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::string>>();
	dispatcher.RegisterGatherer(p_gatherer);
	dispatcher.RegisterProvider(p_provider);

	auto& ddTarget = widget.AddPlugin<UI::Plugins::DDTarget<std::pair<std::string, UI::Widgets::Layout::Group*>>>(p_identifier);
	ddTarget.DataReceivedEvent += [&widget, &dispatcher](std::pair<std::string, UI::Widgets::Layout::Group*> p_data)
	{
		widget.content = p_data.first;
		dispatcher.NotifyChange();
	};
}

void Core::Helpers::GUIDrawer::DrawString(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<std::string(void)> p_gatherer, std::function<void(std::string)> p_provider)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::InputFields::InputText>("");
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<std::string>>();
	dispatcher.RegisterGatherer(p_gatherer);
	dispatcher.RegisterProvider(p_provider);
}

void Core::Helpers::GUIDrawer::DrawColor(UI::Internal::WidgetContainer & p_root, const std::string & p_name, std::function<UI::Types::Color(void)> p_gatherer, std::function<void(UI::Types::Color)> p_provider, bool p_hasAlpha)
{
	CreateTitle(p_root, p_name);
	auto& widget = p_root.CreateWidget<UI::Widgets::Selection::ColorEdit>(p_hasAlpha);
	auto& dispatcher = widget.AddPlugin<UI::Plugins::DataDispatcher<UI::Types::Color>>();
	dispatcher.RegisterGatherer(p_gatherer);
	dispatcher.RegisterProvider(p_provider);
}
