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
		Sad,			// 泣いてる顔
		Angry,			// 怒った顔
		Alert,			// 捕まった！
		Stars,			// きらきら
		Music,			// るんるん
		Anger,			// むかむか
		Idea,			// ひらめき
		Laugh,			// 笑ってる顔
	};

	void SetIcon(IconType iconType, float duration = 0.0f, bool force = false);

	void SetOwner(const std::shared_ptr<CharacterBase>& character)
	{
		m_ownerCharacter = character;
	}

private:

	//========================================
	// 更新
	//========================================

	void UpdateIconAnimation();
	void UpdateIconWorldMatrix();

	//========================================
	// アニメーション
	//========================================

	void StartIconAnimation();

	//========================================
	// 描画
	//========================================

	std::shared_ptr<KdSquarePolygon> m_iconPolygon = nullptr;

	// アイコンを表示する対象キャラクター
	std::weak_ptr<CharacterBase> m_ownerCharacter;

	// 現在表示しているアイコン
	IconType m_currentIcon = IconType::None;

	//========================================
	// アニメーション状態
	//========================================

	float m_animationElapsed = 0.0f;
	float m_animationDuration = 0.3f;

	bool m_isAnimationPlaying = false;

	//========================================
	// 表示設定
	//========================================

	// 通常時のアイコンサイズ
	float m_baseScale = 1.0f;

	// キャラクターからの高さ
	float m_heightOffset = 1.5f;

	//========================================
	// 表示時間
	//========================================

	// 残り表示時間(秒)
	float m_lifeTime = 0.0f;

	// true: 時間経過で自動的に消える
	bool  m_hasLife = false;
};
