#pragma once

// 表示するキーの種類
enum class SkillKey
{
	Space,
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
	Timing,		// タイミングよく押す … 通常 → ちょい押し → 押された → 通常 を1回ずつ繰り返す
	Mash,		// 連打する           … 通常 ⇔ 押された を素早く繰り返す
	Hold,		// 長押しする         … 通常 → ちょい押し → 押された(しばらく維持) → 通常
};

//==================================================
// スキルチェック用のキー表示
//  ・テクスチャは横に3コマ並び: 通常 / 押された / ちょい押し
//  ・Spaceは1コマ 41x21、W/A/S/Dは1コマ 19x21
//  ・テクスチャは初回の描画時にだけ読み込む
//==================================================
class SkillKeyUI
{
public:
	static SkillKeyUI& Instance();

	// x, y    : 画面中央原点・ピクセル(キー画像の中心)
	// time    : アニメーション用の経過時間(秒)
	// isDown  : 実際にそのキーを押している間 true (アニメより優先して「押された」画像にする)
	// alpha   : 透明度
	// scale   : 整数倍率(ドット絵がにじまないよう整数推奨)
	void Draw(SkillKey key, float x, float y,
		SkillKeyAnim anim, float time, bool isDown,
		float alpha = 1.0f, int scale = kDefaultScale);

	// SkillKey → 仮想キーコード
	static int ToVirtualKey(SkillKey key);

private:
	struct KeyData
	{
		std::shared_ptr<KdTexture> tex;
		int frameW = 0;
		int frameH = 0;
	};

	void LoadIfNeeded();

	KeyData m_keys[static_cast<int>(SkillKey::Count)];
	bool   m_loaded = false;

	static constexpr int kDefaultScale = 2;
};