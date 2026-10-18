#pragma once

// 表示するキーの種類
enum class SkillKey
{
	MouseLeft,
	W,
	A,
	S,
	D,
	Count,
};

// キーのアニメーション(操作の種類を見た目で伝える)
enum class SkillKeyAnim
{
	Static,		// 静止(通常の画像のまま)
	Timing,		// タイミングよく押す … 通常 → 押された → 通常 を1回だけ押して戻る動きの繰り返し
	Mash,		// 連打する           … 通常 ⇔ 押された を素早く繰り返す
	Hold,		// 長押しする         … 通常 → 押された(しばらく維持) → 通常
};

//==================================================
// スキルチェック用のキー表示(Kenney Input Prompts 用)
//  ・1キーにつき画像は2枚: 通常 / 押された(2段階のみ)
//  ・「押された」は2枚目の画像 + 少し縮小 + 少し下げ + 少し暗く(kStyle で調整)
//  ・表示サイズは画像の縦横比から自動計算(画像サイズに依存しない)
//  ・テクスチャは初回の描画時にだけ読み込む
//==================================================
class SkillKeyUI
{
public:
	static SkillKeyUI& Instance();

	// x, y    : 画面中央原点・ピクセル(キー画像の中心)
	// time    : アニメーション用の経過時間(秒)
	// isDown  : 実際にそのキーを押している間 true (アニメより優先して「押された」表現にする)
	// alpha   : 透明度
	void Draw(SkillKey key, float x, float y,
		SkillKeyAnim anim, float time, bool isDown,
		float alpha = 1.0f);

	// SkillKey → 仮想キーコード
	static int ToVirtualKey(SkillKey key);

private:
	struct KeyData
	{
		std::shared_ptr<KdTexture> tex[2];	// 0=通常 / 1=押された
		float aspect[2] = { 1.0f, 1.0f };	// 横 / 縦
	};

	void LoadIfNeeded();

	KeyData m_keys[static_cast<int>(SkillKey::Count)];
	bool   m_loaded = false;

	static constexpr float kKeyHeight = 44.0f;	// 画面上でのキーの高さ(px)
};