#pragma once

class SlideTextUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetText(const std::string& text)
	{
		m_text = text;
	}

	void SetPosition(const Math::Vector2& pos)
	{
		m_pos = pos;
	}

	enum class State
	{
		SlideIn,
		Stay,
		SlideOut
	};

private:
	std::string m_text;

	Math::Vector2 m_pos = {};
	float m_lifeTime = 1.0f;

	State m_state = State::SlideIn;

	float m_elapsedTime = 0.0f;

	float m_slideInTime = 0.4f;
	float m_stayTime = 1.0f;
	float m_slideOutTime = 0.4f;

	float m_startX = -700.0f;
	float m_centerX = 0.0f;
	float m_endX = 700.0f;
};