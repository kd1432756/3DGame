#include "DamageIndicatorUI.h"
#include "../../Character/Player/Player.h"
#include "../../../System/Damage/DamageDirector.h"

void DamageIndicatorUI::Init()
{
	m_texHouse = LoadTexture("Asset/Textures/UI/House.png");
	m_texBadge = LoadTexture("Asset/Textures/UI/Badge.png");
}

void DamageIndicatorUI::Update()
{
	if (!LockPlayer()) return;

	m_count = DamageDirector::CountDamaged();

	if (m_count > m_prevCount)
	{
		m_bounceTimer = kBounceTime;
	}
	m_prevCount = m_count;

	// バウンド
	if (m_bounceTimer > 0.0f)
	{
		m_bounceTimer -= DeltaTime();
		if (m_bounceTimer < 0.0f) m_bounceTimer = 0.0f;

		const float t = 1.0f - (m_bounceTimer / kBounceTime);
		m_badgeScale = 1.0f + 0.5f * sinf(t * DirectX::XM_PI);
	}
	else
	{
		m_badgeScale = 1.0f;
	}

	m_houseColor = (m_count > 0)
		? Math::Color{ 1.0f, 0.6f, 0.6f, 1.0f }
	: Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f };
}

void DamageIndicatorUI::DrawSprite()
{
	auto player = LockPlayer();

	if (!player) return;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// 家アイコン
	sprite.DrawTex(m_texHouse,
		(int)m_uiPos.x, (int)m_uiPos.y,
		(int)kHouseSize, (int)kHouseSize,
		nullptr, &m_houseColor);

	if (m_count <= 0) return;

	// バッジ
	const float bx = m_uiPos.x + kHouseSize * 0.4f;
	const float by = m_uiPos.y + kHouseSize * 0.4f;
	const float size = kBadgeSize * m_badgeScale;

	sprite.DrawTex(m_texBadge,
		(int)bx, (int)by,
		(int)size, (int)size);

	DrawLabel(
		Math::Vector2{ bx + 3, by - 0.5f },
		kWhiteColor,
		FontID::Future24,
		std::to_string(m_count).c_str());
}
