#include"PathInspector.h"
#include"UI/Widgets/Texts/TextColored.h"
#include"UI/Widgets/Sliders/SliderFloat.h"
#include"UI/Widgets/Sliders/SliderInt.h"
#include"UI/Widgets/Selection/ColorEdit.h"
#include"UI/Widgets/CustomWidget.h"
#include"UI/Widgets/Buttons/Button.h"
#include"../PathTrace.h"
#include"ImGuizmo.h"
using namespace UI::Widgets;
using namespace PathTrace;
inline  bool raster ;

extern UI::Widgets::Selection::ComboBox* meshchoice;
PathInspector::PathInspector
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) :UI::Panels::Inspector(p_title, p_opened, p_windowSettings)
{


}

PathInspector::~PathInspector()
{

}

void PathInspector::InstallUI()
{
	m_inspectorHeader = &CreateWidget<UI::Widgets::Layout::Group>();
	m_inspectorHeader->CreateWidget<UI::Widgets::Visual::Separator>();
	m_inspectorHeader->enabled = true;
	m_inspectorHeader->CreateWidget<UI::Widgets::Buttons::Button>("Update").ClickedEvent += []() {
		GetScene()->renderOptions = GetRenderOptions();
		GetScene()->dirty = true;
		GetRenderer()->ReloadShaders();

	};
	m_inspectorHeader->CreateWidget<UI::Widgets::Buttons::Button>("Save Frame").ClickedEvent += []() {

		GetRenderer()->SaveFrame();

	};
	CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Raster").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(raster);
	auto& Sample = (CreateWidget<UI::Widgets::Texts::TextColored>("Samples", UI::Types::Color(1.0f, 1.0f, 0.0f, 1.0f))).AddPlugin<UI::Plugins::DataDispatcher<std::string>>();
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
		meshchoice->choices.clear();
		for (int i = 0; i < GetScene()->meshInstances.size(); i++) {
			std::string name = GetScene()->meshInstances[i].name;
			if(name.find("glass_sphere")==std::string::npos)
			meshchoice->choices.emplace(i, GetScene()->meshInstances[i].name);
		}

	};
	//ÇÐ»»»·¾³ÌùÍ¼
	auto& envSelectorWidget = CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	//envSelectorWidget.lineBreak = false;
	for (int i = 0; i < envMaps.size(); i++) {
		envSelectorWidget.choices.emplace(i, envMaps[i]);
	}
	envSelectorWidget.ValueChangedEvent += [](int cur) {
		GetScene()->AddEnvMap(envMaps[cur]);
	};

	auto& moveSpeed = CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.001f, 10.0f, 3.0f, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Mouse sensitivity").AddPlugin<UI::Plugins::DataDispatcher<float>>();
	moveSpeed.RegisterReference(CameraMoveSpeed);

	bool optionsChanged = false;
	bool reloadShaders = false;
	auto render = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Render");
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(-1, 256, -1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Spp").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().maxSpp);
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 10, 1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Depth").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().maxDepth);
	render->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Roulette").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableRR);
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 10, 1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Rouletee Depth").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().RRDepth);
	render->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Roughness Mollification").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableRoughnessMollification);
	render->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0, 1, 0, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Roughness Mollification Amount").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().roughnessMollificationAmt);
	render->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Enable Volume MIS").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableVolumeMIS);


	auto envir = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Environment");
	envir->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Uniform Light").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableUniformLight);
	auto& uniform = envir->CreateWidget<UI::Widgets::Selection::ColorEdit>().AddPlugin<UI::Plugins::DataDispatcher<UI::Types::Color>>();
	uniform.RegisterGatherer([]() {
		Vec3 uniformLightCol = Vec3::Pow(GetRenderOptions().uniformLightCol, 1.0 / 2.2);
		return UI::Types::Color(uniformLightCol.x, uniformLightCol.y, uniformLightCol.z);
		});
	uniform.RegisterProvider([](UI::Types::Color a) {
		GetRenderOptions().uniformLightCol = Vec3(pow(a.r, 2.2), pow(a.g, 2.2), pow(a.b, 2.2));
		});
	envir->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Environment Map").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableEnvMap);
	envir->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.1, 10, 0.1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Intensity").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().envMapIntensity);
	envir->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0, 360, 0, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Rotation").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().envMapRot);
	envir->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Emitters").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().hideEmitters);
	envir->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "BackGround").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableBackground);
	envir->CreateWidget<UI::Widgets::Selection::ColorEdit>().AddPlugin<UI::Plugins::DataDispatcher<UI::Types::Color>>().RegisterReference(*((UI::Types::Color*)(&GetRenderOptions().backgroundCol)));
	envir->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Transparent").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().transparentBackground);

	auto tonemap = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Tonemapping");
	tonemap->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Enbale").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableTonemap);
	tonemap->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "ACES").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableAces);
	tonemap->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "ACES Fit").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().simpleAcesFit);

	auto denoiser = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Denoiser");
	denoiser->CreateWidget<UI::Widgets::Selection::CheckBox>(false, "Enbale").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableDenoiser);
	denoiser->CreateWidget<UI::Widgets::Sliders::SliderInt>(5, 50, -1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Interval Frames").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().denoiserFrameCnt);

	auto camera = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Camera settings");
	auto& fov = camera->CreateWidget<UI::Widgets::Sliders::SliderFloat>(10, 90, 60, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Fov").AddPlugin<UI::Plugins::DataDispatcher<float>>();
	fov.RegisterGatherer([]() {
		return Math::Degrees(GetScene()->camera->fov);
		});
	fov.RegisterProvider([](float a) {
		GetScene()->camera->SetFov(a);
		});
	auto& aperture = camera->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.0f, 10.8f, 0.01f, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Aperture").AddPlugin<UI::Plugins::DataDispatcher<float>>();
	aperture.RegisterGatherer([]() {
		return GetScene()->camera->aperture * 1000.0f;
		});
	aperture.RegisterProvider([](float a) {
		GetScene()->camera->aperture = a / 1000.0f;

		});
	camera->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.01f, 50.0f, 0.01f, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Focal Distance").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetScene()->camera->focalDist);
	camera->CreateWidget<UI::Widgets::Texts::TextColored>("", UI::Types::Color(1.0f, 0.0f, 0.0f, 1.0f)).AddPlugin<UI::Plugins::DataDispatcher<std::string>>().RegisterGatherer([]() {
		std::string  temp;
		temp.reserve(30);
		sprintf(temp.data(), "Pos: %.2f, %.2f, %.2f", GetScene()->camera->position.x, GetScene()->camera->position.y, GetScene()->camera->position.z);
		return temp;
		});

}
