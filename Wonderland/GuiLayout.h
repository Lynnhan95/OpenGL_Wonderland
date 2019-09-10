#pragma once
#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_glfw_gl3.h" // Functions for using imgui UI in GLFW programs
#include "SimParameters.h"
#include "Camera.h"
#include "Scene.h"
#include "MapWriter.h"
#include <GLFW/glfw3.h>

void ReloadScene(const char* addrstr);
void ShowInteractable(std::vector<Handle> handles);
void VisualizeTexture(GLuint texture);
static std::vector<std::string> imComboContent;

void ImGui_Construct_ComboList(Scene* currentScene)
{
	imComboContent.clear();
	for (int i = 0; i < currentScene->GetModelCount() + currentScene->GetParticleSystemCount() + currentScene->GetLightCount(); i++)
	{
		imComboContent.push_back(currentScene->GetObjectNameByIndex(i));
	}
}

// Wrappers for implementing vector<string> in ImGui Combo
// Reference from https://eliasdaler.github.io/using-imgui-with-sfml-pt2/

static auto vector_getter = [](void* vec, int idx, const char** out_text)
{
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
	*out_text = vector.at(idx).c_str();
	return true;
};

bool ComboWrapper(const char* label, int* currIndex, std::vector<std::string>& values)
{
	if (values.empty()) { return false; }
	return ImGui::Combo(label, currIndex, vector_getter,
		static_cast<void*>(&values), values.size());
}

// Initialize ImGui with current window
void Init_Gui(GLFWwindow* window)
{
	ImGui::SetCurrentContext(ImGui::CreateContext());
	ImGui_ImplGlfwGL3_Init(window, true);
}

// Draw the ImGui user interface
void Draw_Gui(Camera* camera, Scene* currentScene)
{
	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	// Camera
	ImGui::SliderInt("Camera Speed", &cam_speed, 1, 5);
	ImGui::SliderInt("Camera Sensitivity", &cam_rot_speed, 1, 3);
	if (ImGui::Button("Reset Camera"))
	{
		camera->Reset();
	}
	ImGui::End();

	ImGui::Begin("Obejct Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	// Interactable components
	static int selection;
	ComboWrapper("Objects", &selection, imComboContent);
	ShowInteractable(currentScene->GetObjectHandleByIndex(selection));
	if (ImGui::Button("Duplicate Object"))
	{
		currentScene->DuplicateObject(selection);
		ImGui_Construct_ComboList(currentScene);
	}
	ImGui::End();

	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	// Map
	if (ImGui::Button("Reload Map"))
	{
		ReloadScene(MAP_NAME.c_str());
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Map"))
	{
		WriteMap(currentScene, MAP_NAME.c_str());
	}
	ImGui::SliderFloat3("Wind Direction", glm::value_ptr(currentScene->simParams.wind_direction), -1.0f, 1.0f);
	ImGui::SliderFloat("Wind Strength", &currentScene->simParams.wind_strength, 0.0f, 5.0f);
	ImGui::End();

	// For debug usage
	VisualizeTexture(currentScene->mainLightMap.texture_id);

	ImGui::Render();
}

// Draw modifiable parameters
void ShowInteractable(std::vector<Handle> handles)
{
	for (Handle handle : handles)
	{
		switch (handle.type)
		{
		case HandleType::VEC3:
			switch (handle.opType)
			{
			case OperationType::OT_INPUT:
				ImGui::InputFloat3(handle.name.c_str(), handle.v1);
				break;
			case OperationType::SLIDE:
				ImGui::SliderFloat3(handle.name.c_str(), handle.v1, handle.min, handle.max);
				break;
			}
			break;
		case HandleType::FLT1:
			switch (handle.opType)
			{
			case OperationType::OT_INPUT:
				ImGui::InputFloat(handle.name.c_str(), handle.v1);
				break;
			case OperationType::SLIDE:
				ImGui::SliderFloat(handle.name.c_str(), handle.v1, handle.min, handle.max);
				break;
			}
			break;
		case HandleType::COL3:
			ImGui::ColorEdit3(handle.name.c_str(), handle.v1);
			break;
		case HandleType::HT_BOOL:
			ImGui::Checkbox(handle.name.c_str(), handle.b1);
			break;
		}
	}
}

void VisualizeTexture(GLuint texture)
{
	ImGui::Begin("Texture Visualizer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((void*)(intptr_t)texture, ImVec2(512, 512));
	ImGui::End();
}