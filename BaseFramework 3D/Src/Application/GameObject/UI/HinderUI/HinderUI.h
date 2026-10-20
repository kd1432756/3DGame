#pragma once

class Player;

class HinderUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetPlayer(const std::shared_ptr<Player>& p) { m_wpPlayer = p; }

private:

	void DrawKeyText(const KdTexture& tex, const Math::Vector2& pos,
		const char* text, const Math::Color& color);

	std::weak_ptr<Player> m_wpPlayer;

	bool  m_canHinder = false;
	bool  m_isHindering = false;
	float m_hinderRatio = 0.0f;
	float m_reportRemaining = 0.0f;
	float m_reportRatio = 0.0f;
	int   m_frame = 0;   // 連打表示の点滅用

	KdTexture m_texKeyF;
	KdTexture m_texKeySpace;
};