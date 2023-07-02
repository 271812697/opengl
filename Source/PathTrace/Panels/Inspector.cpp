#include"Inspector.h"
#include"UI/Widgets/Texts/TextColored.h"
#include"UI/Widgets/Sliders/SliderFloat.h"
#include"UI/Widgets/Sliders/SliderInt.h"
#include"UI/Widgets/Selection/ColorEdit.h"
#include"UI/Widgets/CustomWidget.h"
#include"UI/Widgets/Buttons/Button.h"
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
	m_inspectorHeader->CreateWidget<UI::Widgets::Visual::Separator>();
	m_inspectorHeader->enabled = true;
	m_inspectorHeader->CreateWidget<UI::Widgets::Buttons::Button>("Update").ClickedEvent += []() {
		GetScene()->renderOptions = GetRenderOptions();
		GetScene()->dirty = true;
		GetRenderer()->ReloadShaders();
		
	};
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
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 10, 1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Depth").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().maxDepth);
	render->CreateWidget<UI::Widgets::Selection::CheckBox>(false,"Roulette").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableRR);
	render->CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 10, 1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Rouletee Depth").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().RRDepth);
	render->CreateWidget<UI::Widgets::Selection::CheckBox>(false,"Roughness Mollification").AddPlugin<UI::Plugins::DataDispatcher<bool>>().RegisterReference(GetRenderOptions().enableRoughnessMollification);
	render->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0,1,0, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Roughness Mollification Amount").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().roughnessMollificationAmt);
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
	envir->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.1,10,0.1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Intensity").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().envMapIntensity);
	envir->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0,360, 0, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Rotation").AddPlugin<UI::Plugins::DataDispatcher<float>>().RegisterReference(GetRenderOptions().envMapRot);
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
	denoiser->CreateWidget<UI::Widgets::Sliders::SliderInt>(5,50 , -1, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Skip Frames").AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(GetRenderOptions().denoiserFrameCnt);
	 
	auto camera = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Camera settings");
	auto& fov=camera->CreateWidget<UI::Widgets::Sliders::SliderFloat>(10,90,60,UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Fov").AddPlugin<UI::Plugins::DataDispatcher<float>>();
	fov.RegisterGatherer([]() {
		return Math::Degrees(GetScene()->camera->fov);
		});
	fov.RegisterProvider([](float a) {
		GetScene()->camera->SetFov(a);
		});
	auto& aperture=camera->CreateWidget<UI::Widgets::Sliders::SliderFloat>(0.0f,10.8f,0.01f,UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL,"Aperture").AddPlugin<UI::Plugins::DataDispatcher<float>>();
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
	auto object = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Mesh Instance");
	auto& MeshInstance=object->CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	for (int i = 0; i < GetScene()->meshInstances.size();i++) {
		MeshInstance.choices.emplace(i, GetScene()->meshInstances[i].name);
	}
	
	object->CreateWidget<UI::Widgets::CustomWidget>().DrawIn += [&MeshInstance]() {

		bool objectPropChanged = false;
		ImGui::Text("Material Properties");
		Material* mat = &GetScene()->materials[GetScene()->meshInstances[MeshInstance.currentChoice].materialID];
		// Gamma correction for color picker. Internally, the renderer uses linear RGB values for colors
		Vec3 albedo = Vec3::Pow(mat->baseColor, 1.0 / 2.2);
		objectPropChanged |= ImGui::ColorEdit3("Albedo (Gamma Corrected)", (float*)(&albedo), 0);
		mat->baseColor = Vec3::Pow(albedo, 2.2);

		objectPropChanged |= ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Roughness", &mat->roughness, 0.001f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SpecularTint", &mat->specularTint, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Subsurface", &mat->subsurface, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Anisotropic", &mat->anisotropic, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Sheen", &mat->sheen, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SheenTint", &mat->sheenTint, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Clearcoat", &mat->clearcoat, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("ClearcoatGloss", &mat->clearcoatGloss, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SpecTrans", &mat->specTrans, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Ior", &mat->ior, 1.001f, 2.0f);
		int mediumType = (int)mat->mediumType;
		if (ImGui::Combo("Medium Type", &mediumType, "None\0Absorb\0Scatter\0Emissive\0"))
		{
			objectPropChanged = true;
			mat->mediumType = mediumType;
		}

		if (mediumType != MediumType::None)
		{
			Vec3 mediumColor = Vec3::Pow(mat->mediumColor, 1.0 / 2.2);
			objectPropChanged |= ImGui::ColorEdit3("Medium Color (Gamma Corrected)", (float*)(&mediumColor), 0);
			mat->mediumColor = Vec3::Pow(mediumColor, 2.2);

			objectPropChanged |= ImGui::SliderFloat("Medium Density", &mat->mediumDensity, 0.0f, 5.0f);

			if (mediumType == MediumType::Scatter)
				objectPropChanged |= ImGui::SliderFloat("Medium Anisotropy", &mat->mediumAnisotropy, -0.9f, 0.9f);
		}

		int alphaMode = (int)mat->alphaMode;
		if (ImGui::Combo("Alpha Mode", &alphaMode, "Opaque\0Blend"))
		{
			objectPropChanged = true;
			mat->alphaMode = alphaMode;
		}

		if (alphaMode != AlphaMode::Opaque)
			objectPropChanged |= ImGui::SliderFloat("Opacity", &mat->opacity, 0.0f, 1.0f);
		if (objectPropChanged)
			GetScene()->RebuildInstances();
	};

}

Inspector::~Inspector()
{

}
