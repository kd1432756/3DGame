#pragma once

// ResultUI::DrawSprite からフェード部分を削除し、FadeUI を新設
class ResultFadeUI : public KdGameObject
{
public:
	void Init() override;
	void DrawSprite() override;
	void SetAlpha(float a) { m_alpha = a; }
private:
	float m_alpha = 0.0f;
};