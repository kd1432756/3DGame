#pragma once
#include "../SkillCheckBase.h"

//==================================================
// ゲージ止め
// 左右に動くカーソルを、スペースで緑ゾーンに止める
//  中心の黄色 = Perfect / 緑 = Good / 外 = Miss
//==================================================
class SkillCheckGaugeStop : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::GaugeStop; }

protected:
	void OnStart() override;
	void OnUpdate(float dt) override;
	void OnDraw() override;

private:
	float m_cursor = 0.0f;	// 0.0〜1.0
	float m_dir = 1.0f;	// 移動方向(+1 / -1)
	float m_speed = 1.0f;	// 1秒あたりの移動量(1.0で端から端まで1秒)

	float m_zoneCenter = 0.5f;	// ゾーン中心(0〜1)
	float m_goodWidth = 0.25f;	// Good範囲の幅(全体を1として)
	float m_perfWidth = 0.09f;	// Perfect範囲の幅

	// 表示設定(画面中央原点・ピクセル)
	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};