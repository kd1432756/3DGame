#include "ReticleUI.h"

#include "../../Character/Player/Player.h"

void Reticle::Init()
{
	m_tex = LoadTexture("Asset/Textures/UI/Reticle.png");
}

void Reticle::Update()
{
	auto player = LockPlayer();

	if (!player) return;

	m_canInteract = player->CanHighlight();

	m_isVisible = !player->IsArrested();
}

void Reticle::DrawSprite()
{
	if (!m_tex) return;

	if (!m_isVisible) return;

	// 触れるものがあるときは緑
	const Math::Color color = m_canInteract
		? Math::Color{ 0, 1, 0, 1 }
	: Math::Color{ 1, 1, 1, 1 };

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_tex,
		0,
		0,
		32,
		32,
		nullptr,
		&color);
}
