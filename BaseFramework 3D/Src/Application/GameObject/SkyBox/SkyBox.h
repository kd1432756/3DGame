#pragma once

class SkyBox : public KdGameObject
{
public:

	SkyBox() {}
	~SkyBox() { Release(); }

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

	static constexpr float DayStart = 6.0f;
	static constexpr float EveningStart = 17.0f;
	static constexpr float NightStart = 20.0f;
	static constexpr float NightEnd = 22.0f;

	float m_dayAlpha = 0.0f;
	float m_eveningAlpha = 0.0f;
	float m_nightAlpha = 0.0f;
};