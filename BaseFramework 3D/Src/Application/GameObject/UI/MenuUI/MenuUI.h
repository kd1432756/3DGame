#pragma once

#include "../UIBase/UIBase.h"

//==================================================
// マウス操作のメニュー系UIの基底クラス
//  対象: TitleUI / ResultUI
//
//  共通化した処理
//   ・マウス座標 / クリックした瞬間の判定
//   ・ポインタ画像の読み込みと描画
//   ・ホバー付きボタン(更新 + 描画)
//   ・フェードアウト(α上昇 / BGM音量 / 完了通知)
//
//  使い方(派生側)
//   Init()   : MenuUI::Init() を呼んでからボタンなどを作る
//   Update() : 入力を受けるときに UpdateMouse() → UpdateHover(buttons)
//              毎フレーム UpdateFade() (フェード中は入力処理を return する)
//   Draw     : DrawButtons() / DrawPointer() / DrawFadeOverlay()
//   終了処理 : OnFadeOutFinished() をオーバーライドして次のシーンへ
//==================================================
class MenuUI : public UIBase
{
public:
	MenuUI(bool alwaysUpdate = false) : UIBase(alwaysUpdate) {}
	~MenuUI() override {}

	void Init() override;

protected:
	//----------------------------------------------
	// ボタン
	//----------------------------------------------
	struct MenuButton
	{
		Math::Vector2 pos;			// 中心
		Math::Vector2 size;			// 当たり判定の全サイズ
		const char* text = "";
		FontID        font = FontID::Future48;
		bool          hover = false;
	};

	//----------------------------------------------
	// 入力
	//----------------------------------------------
	// マウス座標とクリック状態を更新する(毎フレーム、入力を受けるときに呼ぶ)
	void UpdateMouse();

	const Math::Vector2& GetMouse() const { return m_mousePos; }

	// 左クリックした瞬間だけ true
	bool IsClickTriggered() const { return m_clickTrigger; }

	// 全ボタンの hover を更新
	void UpdateHover(MenuButton* buttons, int count);

	template<size_t N>
	void UpdateHover(MenuButton(&buttons)[N])
	{
		UpdateHover(buttons, static_cast<int>(N));
	}

	// ボタンの hover を全部 false に戻す(入力を受けない間など)
	static void ClearHover(MenuButton* buttons, int count);

	//----------------------------------------------
	// 描画
	//----------------------------------------------
	// ボタンの文字を描く。offsetY でフワフワ演出ができる
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

	// マウスポインタ
	void DrawPointer() const;

	//----------------------------------------------
	// フェードアウト
	//----------------------------------------------
	void StartFadeOut() { m_fadingOut = true; }
	bool IsFadingOut() const { return m_fadingOut; }
	float GetFadeAlpha() const { return m_fadeAlpha; }

	// フェード中に音量を下げたい BGM を登録(volume = フェード前の音量)
	void SetFadeBgm(const std::shared_ptr<KdSoundInstance>& bgm, float volume)
	{
		m_fadeBgm = bgm;
		m_fadeBgmVolume = volume;
	}

	// 毎フレーム呼ぶ。フェード中でなければ何もしない
	void UpdateFade();

	// 完了時に1回だけ呼ばれる(次のシーンへの遷移などを書く)
	virtual void OnFadeOutFinished() {}

	// 画面全体の黒フェード(別オブジェクトで描く場合は不要)
	void DrawFadeOverlay() const;

private:
	// マウス
	Math::Vector2 m_mousePos = {};
	bool m_prevClick = false;
	bool m_clickTrigger = false;
	std::shared_ptr<KdTexture> m_pointerTex;

	// フェード
	bool  m_fadingOut = false;
	bool  m_fadeFinished = false;
	float m_fadeAlpha = 0.0f;
	std::shared_ptr<KdSoundInstance> m_fadeBgm;
	float m_fadeBgmVolume = 0.3f;

	static constexpr float kFadeSpeed = 1.2f;	// 1秒あたりのα増加量 (従来の 0.02f/フレーム @60fps)
};
