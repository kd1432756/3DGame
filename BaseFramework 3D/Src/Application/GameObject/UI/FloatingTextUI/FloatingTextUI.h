#pragma once

class FloatingTextUI : public KdGameObject
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

	void SetFont(const FontID& fontId)
	{
		m_fontId = fontId;
	}

private:
	std::string m_text;

	Math::Vector2 m_pos = {};
	
	FontID m_fontId = FontID::Future48;

	float m_alpha = 1.0f;

	float m_lifeTime = 1.0f;
	float m_elapsedTime = 0.0f;
};