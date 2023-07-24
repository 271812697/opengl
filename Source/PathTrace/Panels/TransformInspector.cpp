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
	auto& MeshInstance =CreateWidget<UI::Widgets::Selection::ComboBox>(0);
	MeshInstance.ValueChangedEvent += [](int v) {
		selectedInstance = v;
	};
	for (int i = 0; i < GetScene()->meshInstances.size(); i++) {
		MeshInstance.choices.emplace(i, GetScene()->meshInstances[i].name);
	}
	CreateWidget<UI::Widgets::CustomWidget>().DrawIn += [&MeshInstance]() {
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
		Mat4 xform = GetScene()->meshInstances[selectedInstance].transform;
		ImGuizmo::DecomposeMatrixToComponents((float*)&xform, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragScalarN("T", ImGuiDataType_Float, matrixTranslation, 3, 0.025f);
		ImGui::DragScalarN("R", ImGuiDataType_Float, matrixRotation, 3, 0.025f);
		ImGui::DragScalarN("S", ImGuiDataType_Float, matrixScale, 3, 0.025f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&xform);
		if (memcmp(&xform, &GetScene()->meshInstances[selectedInstance].transform, sizeof(float) * 16))
		{
			GetScene()->meshInstances[selectedInstance].transform = xform;
			objectPropChanged = true;
		}
	    }
	};
}
