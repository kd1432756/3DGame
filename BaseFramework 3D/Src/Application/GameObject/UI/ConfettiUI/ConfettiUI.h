#pragma once

#include "../UIBase/UIBase.h"

class ConfettiUI : public UIBase
{
public:
	// 常時更新
	ConfettiUI() : UIBase(true) {}
	~ConfettiUI() override {}

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void Start();
	void Stop();

private:
	struct Particle
	{
		Math::Vector2 pos;
		Math::Vector2 vel;
		float rot = 0.0f;
		float rotSpeed = 0.0f;
		float flip = 0.0f;
		float flipSpeed = 0.0f;
		float swayAmp = 0.0f;
		float w = 0.0f;
		float h = 0.0f;
		Math::Color color;
	};

	void Burst(bool isLeft);
	void SpawnRain();
	Particle MakeBase();

	std::vector<Particle> m_particles;

	bool  m_active = false;
	float m_rainTimer = 0.0f;

	static constexpr int   kBurstCount = 80;
	static constexpr int   kMaxParticles = 400;
	static constexpr float kGravity = 300.0f;
	static constexpr float kDrag = 2.5f;
	static constexpr float kRainInterval = 0.04f;
	static constexpr float kBottom = -400.0f;

	static inline Math::Color kColors[] =
	{
		{ 1.0f, 0.3f, 0.3f, 1.0f },	// 赤
		{ 1.0f, 0.8f, 0.2f, 1.0f },	// 黄
		{ 0.3f, 0.8f, 0.4f, 1.0f },	// 緑
		{ 0.3f, 0.6f, 1.0f, 1.0f },	// 青
		{ 1.0f, 0.5f, 0.8f, 1.0f },	// ピンク
		{ 0.7f, 0.4f, 1.0f, 1.0f },	// 紫
	};
};
