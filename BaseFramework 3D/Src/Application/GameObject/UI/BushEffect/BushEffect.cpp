#include "BushEffect.h"

#include "../../../Scene/SceneManager.h"
#include "../../Character/Player/Player.h"

void BushEffect::Init()
{
	m_tex = std::make_shared<KdTexture>();
	m_tex->Load("Asset/Textures/UI/Bush.png");
}

void BushEffect::DrawSprite()
{
	if (!m_tex)return;

	if (!m_visible)return;

	Math::Color color = { 1,1,1,m_bushAlpha };

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_tex,
		0,
		0,
		1280,
		720,
		nullptr,
		&color);
}

void BushEffect::Update()
{
	auto player =
		SceneManager::Instance().FindObject<Player>();

	if (player)
	{
		m_visible = player->IsInBush();
	}

	if (m_visible)
	{
		m_bushAlpha = std::min(m_bushAlpha + 0.08f, 1.0f);
	}
	else
	{
		m_bushAlpha = std::max(m_bushAlpha - 0.08f, 0.0f);
	}
}
