#include "Reticle.h"

#include "../../../Scene/SceneManager.h"
#include "../../Character/Player/Player.h"

void Reticle::Init()
{
	m_tex = std::make_shared<KdTexture>();
	m_tex->Load("Asset/Textures/UI/Reticle.png");
}

void Reticle::DrawSprite()
{
	if (!m_tex)return;

	Math::Color color = { 1,1,1,1 };

	if (m_canInteract)
	{
		color = { 0,1,0,1 };
	}

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_tex,
		0,
		0,
		32,
		32,
		nullptr,
		&color);
}

void Reticle::Update()
{
	std::shared_ptr<Player> player = m_wpPlayer.lock();

	if (!player)return;

	m_canInteract = player->CanHighlight();
}
