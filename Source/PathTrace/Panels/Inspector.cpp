#include"Inspector.h"
#include"UI/Widgets/Texts/TextColored.h"
#include"UI/Widgets/Sliders/SliderFloat.h"
#include"UI/Widgets/Sliders/SliderInt.h"
#include"../PathTrace.h"
using namespace UI::Widgets;
using namespace PathTrace;

Inspector::Inspector
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) : PanelWindow(p_title, p_opened, p_windowSettings)
{

	m_inspectorHeader = &CreateWidget<UI::Widgets::Layout::Group>();
	m_inspectorHeader->enabled = false;
	m_inspectorHeader->CreateWidget<UI::Widgets::Visual::Separator>();
	auto& Sample=(CreateWidget<UI::Widgets::Texts::TextColored>("Samples", UI::Types::Color(1.0f, 1.0f, 0.0f, 1.0f))).AddPlugin<UI::Plugins::DataDispatcher<std::string>>();
	Sample.RegisterGatherer([]() {
		std::string a = "Samples: ";
		a = a + to_string(GetRenderer()->GetSampleCount());
		return a;
		});

	
	//ÇÐ»»³¡¾°
	auto& sceneSelectorWidget = CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	//sceneSelectorWidget.lineBreak = false;
	for (int i = 0; i < sceneFiles.size(); ++i) {
		sceneSelectorWidget.choices.emplace(i, sceneFiles[i]);

	}
	sceneSelectorWidget.ValueChangedEvent += [](int cur) {
			LoadScene(sceneFiles[cur]);
			InitRenderer();
	};
	//ÇÐ»»»·¾³ÌùÍ¼
	auto& envSelectorWidget = CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	//envSelectorWidget.lineBreak = false;
	for (int i = 0; i < envMaps.size(); i++) {
		envSelectorWidget.choices.emplace(i,envMaps[i]);
	}
	envSelectorWidget.ValueChangedEvent += [](int cur) {
		GetScene()->AddEnvMap(envMaps[cur]);
	};

	auto& mouseDelta=CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.001f,1.0f,0.5f,UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Mouse sensitivity").AddPlugin<UI::Plugins::DataDispatcher<float>>();
	mouseDelta.RegisterReference(mouseSensitivity);

	bool optionsChanged = false;
	bool reloadShaders = false;
	auto render=&CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Render");
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(-1,256,-1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Spp").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().maxSpp);
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 10, 1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Spp").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().maxDepth);

	
	


}

Inspector::~Inspector()
{

}
