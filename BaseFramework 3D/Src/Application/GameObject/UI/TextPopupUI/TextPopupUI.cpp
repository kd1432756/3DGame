#include "TextPopupUI.h"

void TextPopupUI::Update()
{
	const float dt = DeltaTime();

	m_elapsedTime += dt;

	const float life = (std::max)(m_lifeTime, 0.0001f);
	const float t = std::clamp(m_elapsedTime / life, 0.0f, 1.0f);

	// 演出固有の更新
	OnUpdate(dt, t);

	// 寿命が来たら消える
	if (m_elapsedTime >= m_lifeTime)
	{
		m_isExpired = true;
	}
}

void TextPopupUI::DrawSprite()
{
	Math::Color color = m_color;
	color.w *= m_alpha;

	DrawLabel(m_pos, color, m_fontId, m_text.c_str(), m_pivot, m_angle);
}
