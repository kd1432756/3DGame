#include "ConfettiUI.h"

namespace
{
	float Rand(float minV, float maxV)
	{
		static std::mt19937 engine{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(minV, maxV);
		return dist(engine);
	}

	const Math::Color kColors[] =
	{
		{ 1.0f, 0.3f, 0.3f, 1.0f },	// 赤
		{ 1.0f, 0.8f, 0.2f, 1.0f },	// 黄
		{ 0.3f, 0.8f, 0.4f, 1.0f },	// 緑
		{ 0.3f, 0.6f, 1.0f, 1.0f },	// 青
		{ 1.0f, 0.5f, 0.8f, 1.0f },	// ピンク
		{ 0.7f, 0.4f, 1.0f, 1.0f },	// 紫
	};
}

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
	p.rot = Rand(0.0f, DirectX::XM_2PI);
	p.rotSpeed = Rand(-6.0f, 6.0f);
	p.flip = Rand(0.0f, DirectX::XM_2PI);
	p.flipSpeed = Rand(4.0f, 10.0f);
	p.swayAmp = Rand(20.0f, 60.0f);
	p.w = Rand(10.0f, 18.0f);
	p.h = Rand(5.0f, 9.0f);
	p.color = kColors[rand() % (sizeof(kColors) / sizeof(kColors[0]))];
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

		// 画面中央へ向かって斜め上に発射（水平から50〜85度）
		float angle = DirectX::XMConvertToRadians(Rand(50.0f, 85.0f));
		//float speed = Rand(700.0f, 1500.0f);
		float speed = Rand(1500.0f, 2500.0f);

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
	p.pos = { Rand(-640.0f, 640.0f), 380.0f };
	p.vel = { 0.0f, Rand(-120.0f, -40.0f) };

	m_particles.push_back(p);
}

void ConfettiUI::Update()
{
	if (!m_active && m_particles.empty())
	{
		return;
	}

	const float dt = Application::Instance().GetDeltaTime();

	// 上から降らせる
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
		// 重力 + 空気抵抗（勢いよく飛び出して、ゆっくり落ちる動きになる）
		p.vel.y -= kGravity * dt;
		p.vel -= p.vel * (kDrag * dt);

		p.pos += p.vel * dt;

		p.rot += p.rotSpeed * dt;
		p.flip += p.flipSpeed * dt;

		// ひらひら横揺れ
		p.pos.x += sinf(p.flip) * p.swayAmp * dt;
	}

	// 画面下に落ちたものを削除
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const Particle& p) { return p.pos.y < kBottom; }),
		m_particles.end());
}

void ConfettiUI::DrawSprite()
{
	for (const auto& p : m_particles)
	{
		// 紙が裏返る表現：横幅を cos で伸縮
		const float flipScale = cosf(p.flip);
		const float hw = p.w * 0.5f * flipScale;
		const float hh = p.h * 0.5f;

		// 裏面は少し暗く
		Math::Color color = p.color;
		if (flipScale < 0.0f)
		{
			color.x *= 0.7f;
			color.y *= 0.7f;
			color.z *= 0.7f;
		}

		const float c = cosf(p.rot);
		const float s = sinf(p.rot);

		// 回転させた四隅
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

		// 四角形 = 三角形2枚
		KdShaderManager::Instance().m_spriteShader.DrawTriangle(
			x1, y1, x2, y2, x3, y3, &color, true);
		KdShaderManager::Instance().m_spriteShader.DrawTriangle(
			x1, y1, x3, y3, x4, y4, &color, true);
	}
}