#pragma once

class TitleUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

private:
	void GetMousePos(POINT* mousePos);

	bool m_start = false;
	float m_alpha = 0.0f;

	bool m_isHover = false;

	Math::Vector2 m_startPos = { 0.0f, -250.0f };
	Math::Vector2 m_startSize = { 300.0f, 80.0f };

	float m_animTime = 0.0f;
	float m_animOffsetY = 0.0f;

	float m_mousePosX = 0.0f;
	float m_mousePosY = 0.0f;

	std::shared_ptr<KdTexture> m_pointerTex = nullptr;
};