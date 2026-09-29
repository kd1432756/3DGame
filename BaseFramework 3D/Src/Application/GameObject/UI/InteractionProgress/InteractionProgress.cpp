#include "InteractionProgress.h"

#include "../../../Scene/SceneManager.h"
#include "../../Character/Player/Player.h"

void InteractionProgress::Init()
{

}

void InteractionProgress::DrawSprite()
{
	if (!m_visible)	return;

	constexpr int width = 90;
	constexpr int height = 6;

	constexpr int x = 0;
	constexpr int y = -40;

	Math::Color color;

	// 背景
	color = { 0, 0, 0, 0.5f };
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		x,
		y,
		width,
		height,
		&color);

	// 前景
	float progress = std::clamp(m_progress, 0.0f, 1.0f);

	int currentWidth = static_cast<int>(width * progress);

	if (currentWidth <= 0)
		return;

	color = { 0, 1, 0, 1 };
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		x - width + currentWidth,
		y,
		currentWidth,
		height,
		&color);
}

void InteractionProgress::Update()
{
	std::shared_ptr<Player> player = m_wpPlayer.lock();

	if (player)
	{
		m_visible = player->IsCounting();
		m_progress = player->GetUseProgress();
	}
}
