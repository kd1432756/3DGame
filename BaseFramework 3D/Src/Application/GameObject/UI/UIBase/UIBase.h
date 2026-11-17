#pragma once

//==================================================
// 全UI共通の基底クラス
//==================================================
class UIBase : public KdGameObject
{
public:
	UIBase(bool alwaysUpdate = false);
	~UIBase() override {}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

protected:

	// 画面サイズ
	static constexpr int kScreenW = 1280;
	static constexpr int kScreenH = 720;
	// 半分サイズ
	static constexpr int kHalfW = kScreenW / 2;	// 640
	static constexpr int kHalfH = kScreenH / 2;	// 360

	// 時間
	static float DeltaTime() { return 1.0f / 60.0f; }


	// マウス座標
	static Math::Vector2 GetMousePos();

	// ボタン当たり判定
	static bool HitTestRect(const Math::Vector2& point, const Math::Vector2& center, const Math::Vector2& size);

	// 画面全体を塗りつぶす
	static void DrawFullScreenBox(const Math::Color& color);

	// 左詰めで満ちていくゲージ
	static void DrawGauge(int x, int y, int halfW, int halfH, float ratio,
		const Math::Color& bgColor, const Math::Color& fgColor);

	// 文字
	static void DrawLabel(const Math::Vector2& pos, const Math::Color& color,
		FontID font, const char* text,
		const Math::Vector2& pivot = { 0.5f, 0.5f }, float angle = 0.0f);

	// キーアイコンと文字
	static void DrawKeyAndText(const KdTexture* keyTex, const Math::Vector2& pos,
		const char* text, const Math::Color& color,
		FontID font = FontID::Future24,
		float iconH = 28.0f, float gap = 12.0f);

	// イージング
	static float EaseOutCubic(float t)
	{
		return 1.0f - std::pow(1.0f - t, 3.0f);
	}
	static float EaseInCubic(float t)
	{
		return t * t * t;
	}

	// フェードするα
	static float CalcFadeAlpha(float t, float fadeInEnd = 0.1f, float fadeOutStart = 0.8f);

	static std::shared_ptr<KdTexture> LoadTexture(const std::string& path);
};
