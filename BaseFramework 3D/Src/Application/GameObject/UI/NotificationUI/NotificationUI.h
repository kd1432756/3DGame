#pragma once

#include "../TextPopupUI/TextPopupUI.h"

// 左から滑り込み、しばらく留まり、フェードアウトする通知文字
// (複数ある場合はスタック位置へ滑らかに移動する)
class NotificationUI : public TextPopupUI
{
public:
	NotificationUI();

	void Init() override;

	// 生成時に呼ぶ。位置を即座に合わせる
	void SetStartY(float y) { m_pos.y = y; m_targetY = y; }

	// スタック位置が変わったときに呼ぶ。以降は滑らかに移動
	void SetTargetY(float y) { m_targetY = y; }

protected:
	void OnUpdate(float dt, float t) override;

private:
	float m_targetY = 0.0f;

	// ---- 調整用パラメータ ----
	static constexpr float kStartX = -900.0f;	// スライド開始X
	static constexpr float kCenterX = 0.0f;		// 停止X
	static constexpr float kSlideInTime = 0.4f;
	static constexpr float kStayTime = 3.0f;
	static constexpr float kFadeOutTime = 0.4f;
	static constexpr float kMoveSpeed = 0.2f;	// Y移動の追従速度（0〜1、大きいほど速い）
};
