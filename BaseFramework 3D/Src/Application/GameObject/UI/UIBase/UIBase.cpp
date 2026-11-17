#include "UIBase.h"

UIBase::UIBase(bool alwaysUpdate)
{
	if (alwaysUpdate)
	{
		m_updateGroup = UpdateGroup::Always;
	}
}

Math::Vector2 UIBase::GetMousePos()
{
	POINT p;

	GetCursorPos(&p);
	ScreenToClient(Application::Instance().GetWindowHandle(), &p);

	return Math::Vector2{
		static_cast<float>(p.x - kHalfW),
		static_cast<float>(-(p.y - kHalfH)) };
}

bool UIBase::HitTestRect(const Math::Vector2& point,
	const Math::Vector2& center, const Math::Vector2& size)
{
	return  point.x >= center.x - size.x * 0.5f &&
		point.x <= center.x + size.x * 0.5f &&
		point.y >= center.y - size.y * 0.5f &&
		point.y <= center.y + size.y * 0.5f;
}

void UIBase::DrawFullScreenBox(const Math::Color& color)
{
	KdShaderManager::Instance().m_spriteShader.DrawBox(0, 0, kHalfW, kHalfH, &color);
}

void UIBase::DrawGauge(int x, int y, int halfW, int halfH, float ratio,
	const Math::Color& bgColor, const Math::Color& fgColor)
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// 背景
	sprite.DrawBox(x, y, halfW, halfH, &bgColor);

	// ゲージ
	ratio = std::clamp(ratio, 0.0f, 1.0f);
	const int currentW = static_cast<int>(halfW * ratio);
	if (currentW <= 0) return;

	sprite.DrawBox(x - halfW + currentW, y, currentW, halfH, &fgColor);
}

void UIBase::DrawLabel(const Math::Vector2& pos, const Math::Color& color,
	FontID font, const char* text,
	const Math::Vector2& pivot, float angle)
{
	KdShaderManager::Instance().m_spriteShader.DrawFont(
		pos, &color, pivot, angle, font, text);
}

void UIBase::DrawKeyAndText(const KdTexture* keyTex, const Math::Vector2& pos,
	const char* text, const Math::Color& color,
	FontID font, float iconH, float gap)
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// アイコンサイズ
	const float iconW = iconH *
		static_cast<float>(keyTex->GetInfo().Width) /
		static_cast<float>(keyTex->GetInfo().Height);

	// 文字サイズ
	auto fontSprite = KdFontManager::Instance().CreateFontTexture(
		static_cast<int>(font), text, false);
	const Math::Vector2 textSize = sprite.CalcFontSize(fontSprite);

	// アイコン & 文字
	const float totalW = iconW + gap + textSize.x;
	const float left = pos.x - totalW * 0.5f;

	sprite.DrawTex(keyTex,
		static_cast<int>(left), static_cast<int>(pos.y),
		static_cast<int>(iconW), static_cast<int>(iconH),
		nullptr, &color, Math::Vector2{ 0.0f, 0.5f });

	sprite.DrawFont(
		Math::Vector2{ left + iconW + gap, pos.y },
		&color,
		Math::Vector2{ 0.0f, 0.5f },
		0.0f,
		font,
		text);
}

float UIBase::CalcFadeAlpha(float t, float fadeInEnd, float fadeOutStart)
{
	if (t < fadeInEnd)
	{
		return t / fadeInEnd;
	}
	if (t > fadeOutStart)
	{
		return 1.0f - ((t - fadeOutStart) / (1.0f - fadeOutStart));
	}
	return 1.0f;
}

std::shared_ptr<KdTexture> UIBase::LoadTexture(const std::string& path)
{
	auto tex = std::make_shared<KdTexture>();
	tex->Load(path);
	return tex;
}
