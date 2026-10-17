#pragma once

class GameScene;
class ScoreManager;

class CharacterBase;

class ConfettiUI;

class ResultUI : public KdGameObject
{
public:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetGameScene(std::shared_ptr<GameScene> gameScene)
	{
		m_wpGameScene = gameScene;
	}

	void SetConfetti(const std::shared_ptr<ConfettiUI>& confetti) { m_wpConfetti = confetti; }

private:

	void ShowResult();

	void GetMousePos(POINT* mousePos);

	std::weak_ptr<GameScene> m_wpGameScene;

	std::vector<std::shared_ptr<CharacterBase>> m_ranking;

	bool m_visible = false;
	bool m_resultInitialized = false;

	float m_timer = 0.0f;

	int m_displayRankCount = 0;


	bool m_fadeOut = false;
	float m_alpha = 0.0f;


	float m_mousePosX = 0.0f;
	float m_mousePosY = 0.0f;

	std::shared_ptr<KdTexture> m_pointerTex;

	// Titleボタン
	Math::Vector2 m_titleButtonPos = { 0.0f, -270.0f };
	Math::Vector2 m_titleButtonSize = { 400.0f, 60.0f };

	float m_animTime = 0.0f;
	float m_animOffsetY = 0.0f;

	bool m_isTitleButtonHover = false;

	std::weak_ptr<ConfettiUI> m_wpConfetti;
	bool m_confettiStarted = false;
};
