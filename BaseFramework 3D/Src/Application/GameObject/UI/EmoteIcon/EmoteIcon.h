#pragma once

#include "../../Character/CharacterBase.h"

class EmoteIcon : public KdGameObject
{
public:

	void Init() override;
	void Update() override;
	void DrawUnLit() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Effect;
	}

	enum class IconType
	{
		None,
		Exclamation, // !
		Question     // ?
	};

	void SetIcon(IconType iconType);

	void SetOwner(std::shared_ptr<CharacterBase> chara)
	{
		m_owner = chara;
	}

	void SetIconScale(float scale)
	{
		m_iconScale = scale;
	}

private:

	std::shared_ptr<KdSquarePolygon> m_spPoly = nullptr;

	std::weak_ptr<CharacterBase> m_owner;

	IconType m_iconType = IconType::None;

	void UpdateAnimation();
	void UpdateWorldMatrix() override;

	void StartAnimation();

	float m_animTime = 0.0f;
	float m_animDuration = 0.3f;

	bool m_isAnimating = false;

	float m_iconScale = 1.0f;
};