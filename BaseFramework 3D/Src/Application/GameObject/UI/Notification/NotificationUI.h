#pragma once

class NotificationUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetText(const std::string& text) { m_text = text; }

	// 生成時に呼ぶ。位置を即座に合わせる
	void SetStartY(float y) { m_pos.y = y; m_targetY = y; }

	// スタック位置が変わったときに呼ぶ。以降は滑らかに移動
	void SetTargetY(float y) { m_targetY = y; }

private:
	enum class State { SlideIn, Stay, FadeOut };
	State m_state = State::SlideIn;

	std::string   m_text;
	Math::Vector2 m_pos = {};
	float         m_targetY = 0.0f;
	float         m_alpha = 1.0f;
	float         m_elapsedTime = 0.0f;

	// ---- 調整用パラメータ ----
	const float m_startX = -900.0f;	// スライド開始X
	const float m_centerX = 0.0f;		// 停止X
	const float m_slideInTime = 0.4f;
	const float m_stayTime = 3.0f;		// 元の値に合わせて調整してください
	const float m_fadeOutTime = 0.4f;
	const float m_moveSpeed = 0.2f;		// Y移動の追従速度（0〜1、大きいほど速い）
};