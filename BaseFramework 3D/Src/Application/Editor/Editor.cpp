#include "Editor.h"

#include "../Scene/CameraManager/CameraManager.h"

#include "Mode/ObjectMode.h"
#include "Mode/StageMode.h"
#include "Mode/LocationMode.h"
#include "Mode/Navigation/NavigationMode.h"

//============================================================
// 初期化・更新・描画
//============================================================

void Editor::Init()
{
	m_context.camera = std::make_shared<EditorCamera>();
	m_context.camera->Init();

	m_modes[static_cast<int>(EditMode::Object)] = std::make_unique<ObjectMode>();
	m_modes[static_cast<int>(EditMode::Stage)] = std::make_unique<StageMode>();
	m_modes[static_cast<int>(EditMode::Navigation)] = std::make_unique<NavigationMode>();
	m_modes[static_cast<int>(EditMode::Location)] = std::make_unique<LocationMode>();

	m_gameStageAtClose = SceneManager::Instance().GetStageType();
}

void Editor::Update()
{
	// 文字入力中はショートカットを無効にする
	if (ImGui::GetIO().WantTextInput)
	{
		return;
	}

	// Ctrl+Sで保存
/*	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
	{
		m_io.Save();
	}*/

	GetCurrentMode().Update(m_context, m_io);
}

void Editor::Draw()
{
	if (!m_isOpen) return;

	DrawMenuBar();

	IEditorMode& mode = GetCurrentMode();

	mode.ApplyLayout(m_context);

	m_context.sceneWindow.Draw(m_context, mode);

	mode.DrawWindows(m_context);
}

void Editor::DrawDebug()
{
	if (!m_isOpen) return;

	GetCurrentMode().DrawDebug(m_context);
}

void Editor::DrawGrid()
{
	if (!m_isOpen || !m_context.camera) return;

	m_grid.Draw(*m_context.camera);
}

//============================================================
// エディタの開閉・モード切替
//============================================================

void Editor::SetOpen(bool open)
{
	if (m_isOpen == open) return;

	m_isOpen = open;

	if (open)
	{
		m_returnCamera = CameraManager::Instance().GetCamera();
		CameraManager::Instance().SetCamera(m_context.camera);

		if (SceneManager::Instance().GetStageType() != m_gameStageAtClose)
		{
			// ステージの対象を合わせる
			m_io.SyncStageWithGame();
		}

		GetCurrentMode().OnEnter(m_context);
	}
	else
	{
		GetCurrentMode().OnExit(m_context);

		m_gameStageAtClose = SceneManager::Instance().GetStageType();

		CameraManager::Instance().SetCamera(m_returnCamera.lock());
	}
}

void Editor::SetEditMode(EditMode mode)
{
	if (mode == m_editMode) return;

	GetCurrentMode().OnExit(m_context);

	m_editMode = mode;

	GetCurrentMode().OnEnter(m_context);
}

//============================================================
// ステージの切り替え・再読込
//============================================================

void Editor::RequestStageLoad(SceneManager::StageType stage)
{
	m_pendingStage = stage;
	m_openStagePopup = true;
}

void Editor::SwitchStage(SceneManager::StageType stage)
{
	m_io.SetStage(stage);
	m_io.Load();

	NotifyDataReloaded();
}

void Editor::NotifyDataReloaded()
{
	m_context.ClearSelection();

	for (auto& mode : m_modes)
	{
		if (mode)
		{
			mode->OnDataReloaded(m_context);
		}
	}
}

//============================================================
// メニュー
//============================================================

void Editor::DrawMenuBar()
{
	if (!ImGui::BeginMainMenuBar()) return;

	DrawModeMenu();
	DrawFileMenu();

	GetCurrentMode().DrawMenuBar(m_context);

	ImGui::TextDisabled("| Target: %s  %s",
		m_io.GetStageName(),
		m_io.GetStatus().c_str());

	DrawStageSwitchPopup();

	ImGui::EndMainMenuBar();
}

void Editor::DrawModeMenu()
{
	if (!ImGui::BeginMenu("Mode")) return;

	DrawModeMenuItem("Object", EditMode::Object);
	DrawModeMenuItem("Stage", EditMode::Stage);
	DrawModeMenuItem("Navigation", EditMode::Navigation);
	DrawModeMenuItem("Location", EditMode::Location);

	ImGui::EndMenu();
}

void Editor::DrawModeMenuItem(const char* label, EditMode mode)
{
	if (ImGui::MenuItem(label, nullptr, m_editMode == mode))
	{
		SetEditMode(mode);
	}
}

void Editor::DrawFileMenu()
{
	if (!ImGui::BeginMenu("File")) return;

	DrawStageMenu();

	ImGui::Separator();

	if (ImGui::MenuItem("Save Stage", "Ctrl+S"))
	{
		m_io.Save();
	}

	if (ImGui::MenuItem("Load Stage"))
	{
		RequestStageLoad(m_io.GetStage());
	}

	ImGui::Separator();

	ImGui::TextDisabled("Scene:");
	ImGui::Text("%s", m_io.GetScenePath().c_str());

	ImGui::Separator();

	GetCurrentMode().DrawFileMenu(m_io);

	ImGui::EndMenu();
}

void Editor::DrawStageMenu()
{
	if (!ImGui::BeginMenu("Target Stage")) return;

	DrawStageMenuItem(SceneManager::StageType::Stage1);
	DrawStageMenuItem(SceneManager::StageType::Stage2);
	DrawStageMenuItem(SceneManager::StageType::Stage3);

	ImGui::EndMenu();
}

void Editor::DrawStageMenuItem(SceneManager::StageType type)
{
	const bool current = (m_io.GetStage() == type);

	if (ImGui::MenuItem(EditorIO::StageName(type), nullptr, current) && !current)
	{
		RequestStageLoad(type);
	}
}

void Editor::DrawStageSwitchPopup()
{
	if (m_openStagePopup)
	{
		ImGui::OpenPopup("Load Stage");
		m_openStagePopup = false;
	}

	if (!ImGui::BeginPopupModal(
		"Load Stage",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::Text("Load %s?", EditorIO::StageName(m_pendingStage));
	ImGui::Text("Unsaved changes will be discarded.");

	ImGui::Separator();

	if (ImGui::Button("Load", { 120, 0 }))
	{
		SwitchStage(m_pendingStage);

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", { 120, 0 }))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}