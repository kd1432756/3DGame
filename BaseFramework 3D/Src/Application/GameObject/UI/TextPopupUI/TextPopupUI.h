#pragma once

#include "../UIBase/UIBase.h"

//==================================================
// 一定時間だけ出て消える文字UIの基底クラス
//  対象: FloatingTextUI / ShakeTextUI / SlideTextUI / NotificationUI
//
//  共通化した処理
//   ・テキスト / 位置 / 色 / フォント / α / 回転 の保持と setter
//   ・経過時間の加算
//   ・寿命が来たら m_isExpired を立てる
//   ・文字の描画
//
//  派生クラスがやること
//   ・コンストラクタで寿命・フォント・色の初期値を決める
//   ・OnUpdate() に「その演出固有の動き」だけを書く
//     (t = 経過時間 / 寿命 の 0〜1)
//
//  ※ 4クラスとも UpdateGroup::Always だったので、
//    UIBase(true) で常時更新にしてある
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
	// 演出固有の更新
	//  dt : 1フレームの時間
	//  t  : 経過割合 (0〜1)
	virtual void OnUpdate(float dt, float t) {}

	std::string   m_text;
	Math::Vector2 m_pos = {};
	Math::Color   m_color = { 1.0f, 1.0f, 1.0f, 1.0f };	// w は基本 1.0 のまま。フェードは m_alpha で行う
	FontID        m_fontId = FontID::Future48;
	Math::Vector2 m_pivot = { 0.5f, 0.5f };

	float m_alpha = 1.0f;		// 描画時に m_color.w へ掛ける
	float m_angle = 0.0f;		// 回転

	float m_lifeTime = 1.0f;	// 寿命(秒)
	float m_elapsedTime = 0.0f;
};
