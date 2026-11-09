#pragma once

//==================================================
// 全UI共通の基底クラス
//  ・カテゴリを UI に固定
//  ・「ポーズ中も動くUI」の指定をコンストラクタで行う
//  ・画面サイズ定数 / deltaTime / マウス座標 / 当たり判定
//  ・よく使う描画ヘルパー(全画面ボックス・ゲージ・キー付き文字)
//  ・イージング関数
//  ・テクスチャ読み込みヘルパー
//
// 座標はすべて「画面中央が原点・Yが上」(既存UIと同じ)
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
	//----------------------------------------------
	// 画面サイズ
	//----------------------------------------------
	static constexpr int kScreenW = 1280;
	static constexpr int kScreenH = 720;
	// DrawBox は「中心 + 半サイズ」で指定するので、全画面はこの値
	static constexpr int kHalfW = kScreenW / 2;	// 640
	static constexpr int kHalfH = kScreenH / 2;	// 360

	//----------------------------------------------
	// 時間
	//  各UIにバラバラに書かれていた 1/60 をここに集約。
	//  将来 Application の deltaTime に切り替えるならここだけ直す
	//----------------------------------------------
	static float DeltaTime() { return 1.0f / 60.0f; }

	//----------------------------------------------
	// 入力
	//----------------------------------------------
	// マウス座標(画面中央原点・Y上向き)
	static Math::Vector2 GetMousePos();

	// 矩形(中心 + 全サイズ)と点の当たり判定
	static bool HitTestRect(const Math::Vector2& point,
		const Math::Vector2& center, const Math::Vector2& size);

	//----------------------------------------------
	// 描画ヘルパー
	//----------------------------------------------
	// 画面全体を塗りつぶす(暗転・フェード・背景用)
	static void DrawFullScreenBox(const Math::Color& color);

	// 左詰めで満ちていくゲージ
	//  x, y      : ゲージ中心
	//  halfW/H   : 半サイズ(DrawBoxと同じ流儀)
	//  ratio     : 0〜1
	static void DrawGauge(int x, int y, int halfW, int halfH, float ratio,
		const Math::Color& bgColor, const Math::Color& fgColor);

	// 文字(ピボット中央がデフォルト)
	static void DrawLabel(const Math::Vector2& pos, const Math::Color& color,
		FontID font, const char* text,
		const Math::Vector2& pivot = { 0.5f, 0.5f }, float angle = 0.0f);

	// 「キーアイコン + 隙間 + 文字」を pos を中心に揃えて描く
	static void DrawKeyAndText(const KdTexture* keyTex, const Math::Vector2& pos,
		const char* text, const Math::Color& color,
		FontID font = FontID::Future24,
		float iconH = 28.0f, float gap = 12.0f);

	//----------------------------------------------
	// イージング (t は 0〜1)
	//----------------------------------------------
	static float EaseOutCubic(float t)
	{
		return 1.0f - std::pow(1.0f - t, 3.0f);
	}
	static float EaseInCubic(float t)
	{
		return t * t * t;
	}

	// 0→1でフェードイン、fadeOutStart〜1でフェードアウトするα値
	// (FloatingTextUI / ShakeTextUI で同じ計算をしていた部分)
	static float CalcFadeAlpha(float t, float fadeInEnd = 0.1f, float fadeOutStart = 0.8f);

	//----------------------------------------------
	// リソース
	//----------------------------------------------
	static std::shared_ptr<KdTexture> LoadTexture(const std::string& path);
};
