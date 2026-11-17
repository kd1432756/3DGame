#include "ConfettiUI.h"

void ConfettiUI::Init()
{
	m_particles.clear();
	m_particles.reserve(kMaxParticles);
	m_active = false;
	m_rainTimer = 0.0f;
}

void ConfettiUI::Start()
{
	m_active = true;
	m_rainTimer = 0.0f;

	Burst(true);	// 左下
	Burst(false);	// 右下
}

void ConfettiUI::Stop()
{
	m_active = false;
}

ConfettiUI::Particle ConfettiUI::MakeBase()
{
	Particle p;
	p.rot = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);
	p.rotSpeed = KdRandom::GetFloat(-6.0f, 6.0f);
	p.flip = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);
	p.flipSpeed = KdRandom::GetFloat(4.0f, 10.0f);
	p.swayAmp = KdRandom::GetFloat(20.0f, 60.0f);
	p.w = KdRandom::GetFloat(10.0f, 18.0f);
	p.h = KdRandom::GetFloat(5.0f, 9.0f);
	p.color = kColors[KdRandom::GetInt(0, static_cast<int>(sizeof(kColors) / sizeof(kColors[0])) - 1)];
	return p;
}

void ConfettiUI::Burst(bool isLeft)
{
	const float originX = isLeft ? -640.0f : 640.0f;

	for (int i = 0; i < kBurstCount; ++i)
	{
		if (static_cast<int>(m_particles.size()) >= kMaxParticles)
		{
			return;
		}

		Particle p = MakeBase();
		p.pos = { originX, -360.0f };

		float angle = DirectX::XMConvertToRadians(KdRandom::GetFloat(50.0f, 85.0f));
		float speed = KdRandom::GetFloat(1500.0f, 3000.0f);

		float dirX = cosf(angle) * (isLeft ? 1.0f : -1.0f);
		float dirY = sinf(angle);

		p.vel = { dirX * speed, dirY * speed };

		m_particles.push_back(p);
	}
}

void ConfettiUI::SpawnRain()
{
	if (static_cast<int>(m_particles.size()) >= kMaxParticles)
	{
		return;
	}

	Particle p = MakeBase();
	p.pos = { KdRandom::GetFloat(-640.0f, 640.0f), 380.0f };
	p.vel = { 0.0f, KdRandom::GetFloat(-120.0f, -40.0f) };

	m_particles.push_back(p);
}

void ConfettiUI::Update()
{
	if (!m_active && m_particles.empty())
	{
		return;
	}

	const float dt = DeltaTime();

	if (m_active)
	{
		m_rainTimer += dt;
		while (m_rainTimer >= kRainInterval)
		{
			m_rainTimer -= kRainInterval;
			SpawnRain();
		}
	}

	for (auto& p : m_particles)
	{
		p.vel.y -= kGravity * dt;
		p.vel -= p.vel * (kDrag * dt);

		p.pos += p.vel * dt;

		// 回転
		p.rot += p.rotSpeed * dt;
		// 裏帰り
		p.flip += p.flipSpeed * dt;

		// 横揺れ
		p.pos.x += sinf(p.flip) * p.swayAmp * dt;
	}

	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const Particle& p) { return p.pos.y < kBottom; }),
		m_particles.end());
}

void ConfettiUI::DrawSprite()
{
	for (const auto& p : m_particles)
	{
		// cosカーブで伸縮
		const float flipScale = cosf(p.flip);
		const float hw = p.w * 0.5f * flipScale;
		const float hh = p.h * 0.5f;

		// 裏面は暗くする
		Math::Color color = p.color;
		if (flipScale < 0.0f)
		{
			color.x *= 0.7f;
			color.y *= 0.7f;
			color.z *= 0.7f;
		}

		const float c = cosf(p.rot);
		const float s = sinf(p.rot);

		// 回転角度から四隅の座標を計算
		auto corner = [&](float lx, float ly, int& outX, int& outY)
			{
				outX = static_cast<int>(p.pos.x + lx * c - ly * s);
				outY = static_cast<int>(p.pos.y + lx * s + ly * c);
			};

		int x1, y1, x2, y2, x3, y3, x4, y4;
		corner(-hw, -hh, x1, y1);
		corner(-hw, hh, x2, y2);
		corner(hw, hh, x3, y3);
		corner(hw, -hh, x4, y4);

		KdShaderManager::Instance().m_spriteShader.DrawTriangle(x1, y1, x2, y2, x3, y3, &color, true);
		KdShaderManager::Instance().m_spriteShader.DrawTriangle(x1, y1, x3, y3, x4, y4, &color, true);
	}
}