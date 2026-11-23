#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

// 自宅の方向を画面端の三角形で示すUI
class HomeDirectionUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	// 自宅がいたずらされた時の強調表示を開始
	void StartAlert() { m_alertTimer = kAlertTime; }

private:
	// ワールド座標 → スクリーン座標
	bool WorldToScreen(const Math::Vector3& worldPos, Math::Vector2& outScreen) const;

	// 三角形描画
	void DrawArrow(const Math::Vector2& center, const Math::Vector2& dir, const Math::Color& color);

	std::shared_ptr<KdTexture> m_texHouse;   // 家アイコン

	Math::Vector3 m_homePos = Math::Vector3::Zero;
	bool m_hasHomePos = false;

	float m_sceneScale = 1.0f;

	// 状態
	float m_distance = 0.0f;
	bool  m_isVisible = false;	// ヒステリシス込みの表示状態
	float m_alpha = 0.0f;	// フェード用
	float m_alertTimer = 0.0f;

	// 描画用
	Math::Vector2 m_uiPos = Math::Vector2::Zero;	// 中心原点の座標
	Math::Vector2 m_arrowDir = { 0.0f, 1.0f };
	bool m_onScreen = false;
	Math::Color   m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

	int   m_count = 0;        // 現在の被害数

	// ---- 調整用パラメータ ----
	static constexpr float kShowDistance = 25.0f;		// この距離以上で表示
	static constexpr float kHideDistance = 20.0f;		// この距離以下で非表示（ヒステリシス）
	static constexpr float kFadeSpeed = 1.0f / 0.3f;	// 0.3秒でフェード
	static constexpr float kAlertTime = 3.0f;		// 強調表示の長さ
	static constexpr float kArrowSize = 40.0f;
	static constexpr float kHouseSize = 32.0f;
	static constexpr float kEdgeMargin = 0.08f;		// 端からのマージン
	static constexpr float kBaseAlpha = 0.6f;	// 普段の半透明
};
