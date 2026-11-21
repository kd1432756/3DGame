#pragma once

#include "EditorContext.h"
#include "EditorIO.h"
#include "EditorGrid.h"
#include "Mode/IEditorMode.h"

class Editor
{
public:
	enum class EditMode
	{
		Object,
		Stage,
		Navigation,
		Location,

		Max
	};

	void Init();
	void Update();
	void Draw();
	void DrawDebug();
	void DrawGrid();

	void SetOpen(bool open);
	bool IsOpen() const { return m_isOpen; }

	EditMode GetEditMode() const { return m_editMode; }
	void SetEditMode(EditMode mode);

private:
	Editor() = default;

	IEditorMode& GetCurrentMode()
	{
		return *m_modes[static_cast<int>(m_editMode)];
	}

	// メニュー
	void DrawMenuBar();
	void DrawModeMenu();
	void DrawModeMenuItem(const char* label, EditMode mode);
	void DrawFileMenu();

	void DrawStageMenu();
	void DrawStageMenuItem(SceneManager::StageType type);
	void DrawStageSwitchPopup();

	// 確認ダイアログ
	void RequestStageLoad(SceneManager::StageType stage);

	void SwitchStage(SceneManager::StageType stage);
	void NotifyDataReloaded();

	// 共有状態
	EditorContext m_context;
	EditorIO m_io;
	EditorGrid m_grid;

	// モードのポインタの固定配列
	std::array<std::unique_ptr<IEditorMode>,	static_cast<size_t>(EditMode::Max)> m_modes;

	EditMode m_editMode = EditMode::Object;

	// 開閉
	bool m_isOpen = false;

	SceneManager::StageType m_pendingStage = SceneManager::StageType::Stage1;
	SceneManager::StageType m_gameStageAtClose = SceneManager::StageType::Stage1;
	bool m_openStagePopup = false;

	std::weak_ptr<CameraBase> m_previousCamera;

public:
	// シングルトン
	static Editor& Instance()
	{
		static Editor instance;
		return instance;
	}

};