#pragma once

#include"../BaseScene/BaseScene.h"

class FPSCamera;

class ScoreManager;
class DayCycle;
class GameStateUI;

class GameScene : public BaseScene
{
public :

	GameScene() { Init(); }
	~GameScene() {}

	void Update() override;
	void PostUpdate() override;

	void SetGameUpdate(bool flg)
	{
		m_gameUpdate = flg;
	}

	enum class GameState
	{
		Ready,
		Playing,
		Finished,
		Result
	};

	void SetGameState(GameState state)
	{
		m_state = state;
	}

	GameState GetGameState() const
	{
		return m_state;
	}

private:

	void Event() override;
	void Init()  override;
	void Start() override;

	std::shared_ptr<FPSCamera> m_spGameCamera = nullptr;

	std::shared_ptr<DayCycle> m_dayCycle;

	bool m_gameUpdate = false;

	GameState m_state = GameState::Ready;

	std::shared_ptr<GameStateUI> m_gameStateUI;
};
