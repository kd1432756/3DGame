#pragma once

#include "../UIBase/UIBase.h"

//==================================================
// マウス操作のメニュー系UIの基底クラス
//==================================================
class MenuUI : public UIBase
{
public:
	MenuUI(bool alwaysUpdate = false) : UIBase(alwaysUpdate) {}
	~MenuUI() override {}

	void Init() override;

protected:

	struct MenuButton
	{
		Math::Vector2 pos;
		Math::Vector2 size; // 当たり判定のサイズ
		const char* text = "";
		FontID        font = FontID::Future48;
		bool          hover = false;
	};

	void UpdateMouse();

	const Math::Vector2& GetMouse() const { return m_mousePos; }

	bool IsClickTriggered() const { return m_clickTrigger; }

	void UpdateHover(MenuButton* buttons, int count);

	template<size_t N>
	void UpdateHover(MenuButton(&buttons)[N])
	{
		UpdateHover(buttons, static_cast<int>(N));
	}

	// hoverしなくする
	static void ClearHover(MenuButton* buttons, int count);

	void DrawButtons(const MenuButton* buttons, int count,
		const Math::Color& normalColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		const Math::Color& hoverColor = { 1.0f, 0.8f, 0.2f, 1.0f },
		float offsetY = 0.0f) const;

	template<size_t N>
	void DrawButtons(const MenuButton(&buttons)[N],
		const Math::Color& normalColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		const Math::Color& hoverColor = { 1.0f, 0.8f, 0.2f, 1.0f },
		float offsetY = 0.0f) const
	{
		DrawButtons(buttons, static_cast<int>(N), normalColor, hoverColor, offsetY);
	}

	void DrawPointer() const;

	void StartFadeOut() { m_fadingOut = true; }
	bool IsFadingOut() const { return m_fadingOut; }
	float GetFadeAlpha() const { return m_fadeAlpha; }

	// フェードでBGMの音を下げる
	void SetFadeBgm(const std::shared_ptr<KdSoundInstance>& bgm, float volume)
	{
		m_fadeBgm = bgm;
		m_fadeBgmVolume = volume;
	}

	void UpdateFade();

	// シーンの遷移
	virtual void OnFadeOutFinished() {}

	// 黒フェード
	void DrawFadeOverlay() const;

private:
	Math::Vector2 m_mousePos = {};
	bool m_prevClick = false;
	bool m_clickTrigger = false;
	std::shared_ptr<KdTexture> m_pointerTex;

	bool  m_fadingOut = false;
	bool  m_fadeFinished = false;
	float m_fadeAlpha = 0.0f;
	std::shared_ptr<KdSoundInstance> m_fadeBgm;
	float m_fadeBgmVolume = 0.3f;

	static constexpr float kFadeSpeed = 1.2f;
};
