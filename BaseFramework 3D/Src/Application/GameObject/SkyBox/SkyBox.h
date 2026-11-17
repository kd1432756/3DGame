#pragma once

class SkyBox : public KdGameObject
{
public:

	void Init() override;
	void Update() override;
	void DrawUnLit() override;

	const char* GetTypeName() const override
	{
		return "SkyBox";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::System;
	}

private:

	std::array<std::shared_ptr<KdModelData>, 3>  m_spModel;

	static constexpr float kDayStart = 6.0f;
	static constexpr float kEveningStart = 17.0f;
	static constexpr float kNightStart = 20.0f;
	static constexpr float kNightEnd = 22.0f;

	float m_dayAlpha = 0.0f;
	float m_eveningAlpha = 0.0f;
	float m_nightAlpha = 0.0f;
};