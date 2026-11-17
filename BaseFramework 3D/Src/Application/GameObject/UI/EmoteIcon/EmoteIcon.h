#pragma once

class CharacterBase;

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
		Exclamation,	// !
		Question,		// ?
		Sad,				// 泣いてる顔
		Angry,			// 怒った顔
		Alert,			// 捕まった！
		Stars,			// きらきら
		Music,			// るんるん
		Anger,			// むかむか
		Idea,				// ひらめき
		Laugh,			// 笑ってる顔
	};

	bool SetIcon(IconType iconType, float duration = 0.0f, bool force = false);

	void SetOwner(const std::shared_ptr<CharacterBase>& character)
	{
		m_ownerCharacter = character;
	}

	enum class AnimationState
	{
		None,
		Appear,
		Disappear
	};

private:

	void UpdateIconAnimation();
	void UpdateIconWorldMatrix();

	void StartIconAnimation();
	void StartIconDisappearAnimation();

	std::shared_ptr<KdSquarePolygon> m_iconPolygon = nullptr;

	std::weak_ptr<CharacterBase> m_ownerCharacter;

	IconType m_currentIcon = IconType::None;

	AnimationState m_animationState = AnimationState::None;

	float m_animationElapsed = 0.0f;
	float m_animationDuration = 0.3f;

	float m_baseScale = 1.0f;

	float m_heightOffset = 1.5f;

	float m_lifeTime = 0.0f;

	bool  m_hasLife = false;
};
