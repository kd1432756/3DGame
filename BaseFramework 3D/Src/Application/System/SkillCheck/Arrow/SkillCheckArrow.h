#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 方向入力
// 表示された方向(W:上 S:下 A:左 D:右)を順番に入力する
//  速く全部入力 = Perfect / 制限時間内に入力 = Good / 間違えたら = Miss
// ※ 方向は四角の中の小さな四角のズレで表示(文字描画に依存しない)
//==================================================
class SkillCheckArrow : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::Arrow; }

protected:
	enum Dir { Up, Down, Left, Right };

	void OnStart() override;

	void OnUpdate(float dt) override;

	void OnDraw() override;

private:
	std::vector<Dir> m_seq;
	int   m_index = 0;
	float m_inputTime = 0.0f;
	float m_perfectTime = 1.5f;

	static constexpr float kY = -120.0f;
	static constexpr float kSpacing = 80.0f;
	static constexpr float kSlotHalf = 28.0f;
	static constexpr float kMarkerOffset = 14.0f;
};