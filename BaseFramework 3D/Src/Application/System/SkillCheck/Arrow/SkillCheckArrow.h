#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 方向入力
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
	static SkillKey KeyOf(Dir d)
	{
		switch (d)
		{
		case Up:   return SkillKey::W;
		case Down: return SkillKey::S;
		case Left: return SkillKey::A;
		default:   return SkillKey::D;
		}
	}

	std::vector<Dir> m_seq;
	int   m_index = 0;
	float m_inputTime = 0.0f;
	float m_perfectTime = 1.5f;

	static constexpr float kY = -120.0f;
	static constexpr float kSpacing = 80.0f;
	static constexpr float kSlotHalf = 28.0f;
	static constexpr float kMarkerOffset = 14.0f;
};