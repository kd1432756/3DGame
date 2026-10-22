#include "DamageIndicatorUI.h"
#include "../../Character/Player/Player.h"

void DamageIndicatorUI::Init()
{
	m_texHouse = std::make_shared<KdTexture>();
	m_texHouse->Load("Asset/Textures/UI/House.png");

	m_texBadge = std::make_shared<KdTexture>();
	m_texBadge->Load("Asset/Textures/UI/Badge.png");
}

void DamageIndicatorUI::Update()
{
	auto owner = m_owner.lock();
	if (!owner) return;

	m_count = owner->GetDamagedCount();

	// 被害が増えた瞬間だけバウンド開始
	if (m_count > m_prevCount)
	{
		m_bounceTimer = m_bounceTime;
	}
	m_prevCount = m_count;

	// バウンド演出(1.0 → 1.5 → 1.0)
	if (m_bounceTimer > 0.0f)
	{
		m_bounceTimer -= 1 / 60.0f;
		if (m_bounceTimer < 0.0f) m_bounceTimer = 0.0f;

		const float t = 1.0f - (m_bounceTimer / m_bounceTime);   // 0→1
		m_badgeScale = 1.0f + 0.5f * sinf(t * DirectX::XM_PI);
	}
	else
	{
		m_badgeScale = 1.0f;
	}

	// 被害ありなら家アイコンを赤みがかった色に
	m_houseColor = (m_count > 0)
		? Math::Color{ 1.0f, 0.6f, 0.6f, 1.0f }
	: Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f };
}

void DamageIndicatorUI::DrawSprite()
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	Math::Color ring = Math::Color{ 0.3f, 0.3f, 0.3f, 1.0f };
	Math::Color circle = Math::Color{ 0.6f, 0.6f, 0.6f, 1.0f };
		
	// 家アイコン(常時表示)
	sprite.DrawTex(m_texHouse,
		(int)m_uiPos.x, (int)m_uiPos.y,
		(int)kHouseSize, (int)kHouseSize,
		nullptr, &m_houseColor);

	// 被害0ならバッジは出さない
	if (m_count <= 0) return;

	// バッジ位置: 家アイコンの右上
	const float bx = m_uiPos.x + kHouseSize * 0.4f;
	const float by = m_uiPos.y + kHouseSize * 0.4f;
	const float size = kBadgeSize * m_badgeScale;

	sprite.DrawTex(m_texBadge,
		(int)bx, (int)by,
		(int)size, (int)size);

	sprite.DrawFont(
		Math::Vector2{ bx + 3, by - 0.5f },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },	//pivot
		0.0f,
		FontID::Future24,
		std::to_string(m_count).c_str());
}