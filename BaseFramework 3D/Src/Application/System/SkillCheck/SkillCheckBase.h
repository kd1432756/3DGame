#pragma once

#include "../../GameObject/UI/SkillKeyUI/SkillKeyUI.h"

// スキルチェックの結果
enum class SkillResult
{
	None,		// 未確定(進行中)
	Miss,
	Good,
	Perfect,
};

// 種類(Factoryで「直前と同じものを除く」判定に使う)
enum class SkillCheckType
{
	GaugeStop,		// 動くバーを緑ゾーンで止める
	Mash,			// 連打してゲージを満タンにする
	HoldRelease,	// 長押しして、満タン直前で離す
	Arrow,			// 表示された方向を順番に入力する
	Reaction,		// 合図が出た瞬間に押す
	ShrinkRing,		// 縮む枠が中心に重なる瞬間に押す
	AltMash,		// AとDを交互に連打する
	Count,
};

//==================================================
// スキルチェック基底クラス
// 流れ: Start() → (準備時間) → 入力受付 → Finish() → IsFinished()==true
// 派生クラスは OnStart / OnUpdate / OnDraw の3つを実装するだけ
//==================================================
class SkillCheckBase
{
public:
	virtual ~SkillCheckBase() = default;

	// difficulty: 1.0が標準。大きいほど難しい(ゾーンが狭い・速い)
	void Start(float difficulty = 1.0f);
	void Update();
	void Draw();

	bool        IsFinished() const { return m_result != SkillResult::None; }
	SkillResult GetResult()  const { return m_result; }

	virtual SkillCheckType GetType() const = 0;

protected:
	virtual void OnStart() = 0;
	virtual void OnUpdate(float dt) = 0;	// 入力受付中のみ呼ばれる
	virtual void OnDraw() = 0;

	void Finish(SkillResult result);

	// 決定キー(スペース)が押されたフレームか
	bool IsActionPressed() const { return IsKeyPressed(VK_SPACE); }

	// 準備時間中か(操作アイコンを見せる猶予)
	bool IsReady() const { return m_elapsed >= m_readyTime; }

	// 準備時間中は薄く表示するための透明度
	float GetAlpha() const { return IsReady() ? 1.0f : 0.4f; }

	//---------------- 共通ヘルパー ----------------
	bool  IsKeyPressed(int vk) const;	// 押したフレームだけ true
	bool  IsKeyDown(int vk) const;		// 押している間 true

	// 画面中央原点・ピクセル指定の矩形描画(halfW/halfH は半分のサイズ)
	static void  DrawRect(float x, float y, float halfW, float halfH, const Math::Color& color);

	// 操作キーの表示(実際に押している間は「押された」画像になる)
	//  anim    : Static=静止 / Timing=タイミング / Mash=連打 / Hold=長押し
	//  alphaMul: 追加の透明度(押さないキーを薄くする用)
	void DrawKey(SkillKey key, float x, float y, SkillKeyAnim anim, float alphaMul = 1.0f) const;
	
protected:
	SkillResult m_result = SkillResult::None;
	float       m_difficulty = 1.0f;
	float       m_elapsed = 0.0f;	// Start からの経過時間
	float       m_readyTime = 0.4f;	// この間は入力を受け付けない(開始時の誤入力防止＆予告)
	float       m_timeLimit = 3.0f;	// 入力受付開始から数える。超えたらMiss(OnStartで変更可)

private:
	void PollKeys();
	void ClearPressed();	// 保持している「押された」を捨てる(準備時間・開始時用)
	static int KeyIndex(int vk);

	void PlayKeyDownSound() const;

	static constexpr int kKeyCount = static_cast<int>(SkillKey::Count);
	bool m_keyDown[kKeyCount] = {};
	bool m_keyPressed[kKeyCount] = {};
	bool m_keyPrevDown[kKeyCount] = {};

	static constexpr float kLayoutOffsetY = 60.0f;	// 全スキルチェックUIを上へずらす量
};