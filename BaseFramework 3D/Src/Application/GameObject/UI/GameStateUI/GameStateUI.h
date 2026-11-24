#pragma once

#include "../UIBase/UIBase.h"

class GameScene;

class GameStateUI : public UIBase
{
public:
	// 常時更新
	GameStateUI() : UIBase(true) {}

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetGameScene(std::shared_ptr<GameScene> gameScene)
	{
		m_wpGameScene = gameScene;
	}

	enum class DisplayState
	{
		WaitClick,
		CountDown,
		Start,
		Finish,
		None
	};

	void ShowFinish()
	{
		m_displayState = DisplayState::Finish;
		m_timer = 0.0f;
	}

private:
	std::weak_ptr<GameScene> m_wpGameScene;

	float m_timer = 0.0f;
	int m_count = 3;

	DisplayState m_displayState = DisplayState::WaitClick;

	std::shared_ptr<KdSoundInstance> m_bgm;
};
