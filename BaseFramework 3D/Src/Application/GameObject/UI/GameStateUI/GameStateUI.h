#pragma once

class GameScene;

class GameStateUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetGameScene(std::shared_ptr<GameScene> gameScene)
	{
		m_wpGameScene = gameScene;
	}

	enum class DisplayState
	{
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

	DisplayState m_displayState = DisplayState::CountDown;
};