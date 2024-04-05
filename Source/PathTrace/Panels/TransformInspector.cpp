#include"TransformInspector.h"
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
UI::Widgets::Selection::ComboBox* meshchoice;
UI::Widgets::Selection::ComboBox* matchoice;
TransformInspector::TransformInspector
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) :UI::Panels::Inspector(p_title, p_opened, p_windowSettings)
{


}

TransformInspector::~TransformInspector()
{

}

void TransformInspector::InstallUI()
{
	m_inspectorHeader = &CreateWidget<UI::Widgets::Layout::Group>();
	m_inspectorHeader->CreateWidget<UI::Widgets::Visual::Separator>();
	m_inspectorHeader->enabled = true;
	auto& MeshInstance = CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	meshchoice = &MeshInstance;
	MeshInstance.AddPlugin<UI::Plugins::DataDispatcher<int>>().RegisterReference(selectedInstance);

	for (int i = 0; i < GetScene()->meshInstances.size(); i++) {
		std::string name = GetScene()->meshInstances[i].name;
		if (name.find("glass_sphere") == std::string::npos)
			meshchoice->choices.emplace(i, GetScene()->meshInstances[i].name);
	}



	CreateWidget<UI::Widgets::CustomWidget>().DrawIn += []() {
		objectPropChanged = false;
		ImGui::Text("Material Properties");
		Material* mat = &GetScene()->materials[GetScene()->meshInstances[selectedInstance].materialID];
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
		ImGui::Checkbox("Show Transform", &showTransform);
		if (showTransform) {
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			Mat4 xform = GetScene()->meshInstances[selectedInstance].localform;
			ImGuizmo::DecomposeMatrixToComponents((float*)&xform, matrixTranslation, matrixRotation, matrixScale);
			ImGui::DragScalarN("T", ImGuiDataType_Float, matrixTranslation, 3, 0.025f);
			ImGui::DragScalarN("R", ImGuiDataType_Float, matrixRotation, 3, 0.025f);
			ImGui::DragScalarN("S", ImGuiDataType_Float, matrixScale, 3, 0.025f);
			ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&xform);
			if (memcmp(&xform, &GetScene()->meshInstances[selectedInstance].localform, sizeof(float) * 16))
			{
				GetScene()->meshInstances[selectedInstance].localform = xform;
				objectPropChanged = true;
			}
		}
		};
	/*
	auto& MatInstance = CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	matchoice = &MatInstance;
	MatInstance.ValueChangedEvent += [](int v) {
		selectedMat = v;
		};
	for (int i = 0; i < GetScene()->materials.size(); i++) {
		std::string na = " Mat" + std::to_string(i);
		matchoice->choices.emplace(i, na);
	}
	auto MatI = &CreateWidget<UI::Widgets::Layout::GroupCollapsable>("Material Instance");
	auto& MAt= MatI->CreateWidget<UI::Widgets::CustomWidget>();
	MAt.DrawIn += []() {
		objectPropChanged = false;

		ImGui::Text("Material Instance");
		Material* mat = &GetScene()->materials[selectedMat];
		// Gamma correction for color picker. Internally, the renderer uses linear RGB values for colors
		Vec3 albedo = Vec3::Pow(mat->baseColor, 1.0 / 2.2);
		objectPropChanged |= ImGui::ColorEdit3("Albedo", (float*)(&albedo), 0);
		mat->baseColor = Vec3::Pow(albedo, 2.2);

		objectPropChanged |= ImGui::SliderFloat("Meta", &mat->metallic, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Rough", &mat->roughness, 0.001f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SpecularTin", &mat->specularTint, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Subsurfac", &mat->subsurface, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Anisotropi", &mat->anisotropic, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Shee", &mat->sheen, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SheenTin", &mat->sheenTint, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Clearcoa", &mat->clearcoat, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("ClearcoatGlos", &mat->clearcoatGloss, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("SpecTran", &mat->specTrans, 0.0f, 1.0f);
		objectPropChanged |= ImGui::SliderFloat("Io", &mat->ior, 1.001f, 2.0f);
		int mediumType = (int)mat->mediumType;
		if (ImGui::Combo("Medium Typ", &mediumType, "None\0Absorb\0Scatter\0Emissive\0"))
		{
			objectPropChanged = true;
			mat->mediumType = mediumType;
		}
		if (mediumType != MediumType::None)
		{
			Vec3 mediumColor = Vec3::Pow(mat->mediumColor, 1.0 / 2.2);
			objectPropChanged |= ImGui::ColorEdit3("Medium Color ", (float*)(&mediumColor), 0);
			mat->mediumColor = Vec3::Pow(mediumColor, 2.2);

			objectPropChanged |= ImGui::SliderFloat("Medium Densit", &mat->mediumDensity, 0.0f, 5.0f);

			if (mediumType == MediumType::Scatter)
				objectPropChanged |= ImGui::SliderFloat("Medium Anisotrop", &mat->mediumAnisotropy, -0.9f, 0.9f);
		}
		int alphaMode = (int)mat->alphaMode;
		if (ImGui::Combo("Alpha Mod", &alphaMode, "Opaqu\0Blend"))
		{
			objectPropChanged = true;
			mat->alphaMode = alphaMode;
		}
		if (alphaMode != AlphaMode::Opaque)
			objectPropChanged |= ImGui::SliderFloat("Opacit", &mat->opacity, 0.0f, 1.0f);

	};	
	
	
	
	
	*/

}
