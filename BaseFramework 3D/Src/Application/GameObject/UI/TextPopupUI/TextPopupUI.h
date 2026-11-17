#pragma once

#include "../UIBase/UIBase.h"

//==================================================
// 文字UIの基底クラス
//==================================================
class TextPopupUI : public UIBase
{
public:
	TextPopupUI() : UIBase(true) {}
	~TextPopupUI() override {}

	void Update() override;
	void DrawSprite() override;

	void SetText(const std::string& text) { m_text = text; }
	void SetPosition(const Math::Vector2& pos) { m_pos = pos; }
	void SetColor(const Math::Color& color) { m_color = color; }
	void SetFont(const FontID& fontId) { m_fontId = fontId; }

protected:

	//  dt : 1フレームの時間,
	//  t  : 経過割合 (0〜1)
	virtual void OnUpdate(float dt, float t) {}

	std::string   m_text;
	Math::Vector2 m_pos = {};
	Math::Color   m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	FontID        m_fontId = FontID::Future48;
	Math::Vector2 m_pivot = { 0.5f, 0.5f };

	float m_alpha = 1.0f;
	float m_angle = 0.0f;

	float m_lifeTime = 1.0f;
	float m_elapsedTime = 0.0f;
};
