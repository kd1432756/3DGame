#include "MenuUI.h"

void MenuUI::Init()
{
	m_pointerTex = LoadTexture("Asset/Textures/Cursor/Pointer.png");

	m_prevClick = false;
	m_clickTrigger = false;

	m_fadingOut = false;
	m_fadeFinished = false;
	m_fadeAlpha = 0.0f;
}

void MenuUI::UpdateMouse()
{
	m_mousePos = GetMousePos();

	// クリックした瞬間だけ true にする
	const bool click = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	m_clickTrigger = click && !m_prevClick;
	m_prevClick = click;
}

void MenuUI::UpdateHover(MenuButton* buttons, int count)
{
	for (int i = 0; i < count; ++i)
	{
		buttons[i].hover = HitTestRect(m_mousePos, buttons[i].pos, buttons[i].size);
	}
}

void MenuUI::ClearHover(MenuButton* buttons, int count)
{
	for (int i = 0; i < count; ++i)
	{
		buttons[i].hover = false;
	}
}

void MenuUI::DrawButtons(const MenuButton* buttons, int count,
	const Math::Color& normalColor, const Math::Color& hoverColor,
	float offsetY) const
{
	for (int i = 0; i < count; ++i)
	{
		const MenuButton& btn = buttons[i];

		Math::Vector2 drawPos = btn.pos;
		drawPos.y += offsetY;

		DrawLabel(drawPos, btn.hover ? hoverColor : normalColor,
			btn.font, btn.text);
	}
}

void MenuUI::DrawPointer() const
{
	if (!m_pointerTex) return;

	const Math::Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_pointerTex,
		m_mousePos.x,
		m_mousePos.y,
		nullptr,
		&white,
		Math::Vector2{ 0.3f, 0.8f });
}

void MenuUI::UpdateFade()
{
	if (!m_fadingOut) return;

	m_fadeAlpha = (std::min)(m_fadeAlpha + kFadeSpeed * DeltaTime(), 1.0f);

	// フェードに合わせて BGM を小さくする
	if (m_fadeBgm)
	{
		m_fadeBgm->SetVolume((std::max)(m_fadeBgmVolume * (1.0f - m_fadeAlpha), 0.0f));
	}

	if (m_fadeAlpha >= 1.0f && !m_fadeFinished)
	{
		m_fadeFinished = true;
		OnFadeOutFinished();
	}
}

void MenuUI::DrawFadeOverlay() const
{
	if (m_fadeAlpha <= 0.0f) return;

	DrawFullScreenBox(Math::Color{ 0.0f, 0.0f, 0.0f, m_fadeAlpha });
}
