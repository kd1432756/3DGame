#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 交互押し
// AとDを交互に押してゲージを満タンにする(同じキーの連続は無効)
//  速く満タン = Perfect / 制限時間内 = Good / 間に合わない = Miss
// 画面の左右の四角が「次に押すキー」(左=A / 右=D)
//==================================================
class SkillCheckAltMash : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::AltMash; }

protected:
	void OnStart() override;
	
	void OnUpdate(float dt) override;
	
	void OnDraw() override;	

private:
	int   m_required = 10;
	float m_gauge = 0.0f;
	float m_inputTime = 0.0f;
	float m_perfectTime = 1.5f;
	float m_wrongFlash = 0.0f;
	bool  m_expectLeft = true;	// true=次はA / false=次はD

	static constexpr float kDecay = 0.25f;
	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kY = -120.0f;
	static constexpr float kSideHalf = 18.0f;
	static constexpr float kSideOffset = 30.0f;
};