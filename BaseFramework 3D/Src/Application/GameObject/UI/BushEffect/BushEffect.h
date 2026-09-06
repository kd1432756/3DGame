#pragma once

class BushEffect : public KdGameObject
{
public:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

private:

	std::shared_ptr<KdTexture> m_tex = nullptr;

	bool m_visible = false;

	float m_bushAlpha = 0.0f;
};