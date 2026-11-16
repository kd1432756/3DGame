#pragma once

#include "../../GameObject/UI/SkillKeyUI/SkillKeyUI.h"

enum class SkillResult
{
	None,		// 未確定
	Miss,
	Good,
	Perfect,
};

enum class SkillCheckType
{
	GaugeStop,	// 動くバーを緑ゾーンで止める
	Mash,			// 連打してゲージを満タンにする
	HoldRelease,	// 長押しして、満タン直前で離す
	Arrow,			// 表示された方向を順番に入力する
	Reaction,		// 合図が出た瞬間に押す
	ShrinkRing,	// 縮む枠が中心に重なる瞬間に押す
	AltMash,		// AとDを交互に連打する
	Count,
};

class SkillCheckBase
{
public:
	virtual ~SkillCheckBase() = default;

	void Start(float difficulty = 1.0f);
	void Update();
	void Draw();

	bool IsFinished() const { return m_result != SkillResult::None; }
	SkillResult GetResult()  const { return m_result; }

	virtual SkillCheckType GetType() const = 0;

protected:
	virtual void OnStart() = 0;
	virtual void OnUpdate(float dt) = 0;
	virtual void OnDraw() = 0;

	void Finish(SkillResult result);

	static constexpr SkillKey kActionKey = SkillKey::MouseLeft;
	static constexpr int      kActionVK = VK_LBUTTON;
	bool IsActionPressed() const { return IsKeyPressed(kActionVK); }	// 押したフレーム
	bool IsActionDown()    const { return IsKeyDown(kActionVK); }		// 押している間

	// 準備時間中か
	bool IsReady() const { return m_elapsed >= m_readyTime; }

	float GetAlpha() const { return IsReady() ? 1.0f : 0.4f; }

	//---------------- 共通ヘルパー ----------------
	bool  IsKeyPressed(int vk) const;	// 押したフレーム
	bool  IsKeyDown(int vk) const;		// 押している間

	static void  DrawRect(float x, float y, float halfW, float halfH, const Math::Color& color);

	static void DrawCircle(float x, float y, float radius, const Math::Color& color, bool fill = true);

	static void DrawRing(float x, float y, float radius, float thickness, const Math::Color& color);

	static void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
		const Math::Color& color);

	// UIの下地
	static void DrawPanel(float x, float y, float halfW, float halfH, float alpha);

	// 矢印の図形
	static void DrawArrowShape(float x, float y, int fx, int fy, float size, const Math::Color& color);

	// 残り時間バー
	void DrawTimeBar(float x, float y, float halfW, float perfectTime = 0.0f) const;

	// 操作キー
	void DrawKey(SkillKey key, float x, float y, SkillKeyAnim anim, float alphaMul = 1.0f) const;
	
protected:
	SkillResult m_result = SkillResult::None;
	float	m_difficulty = 1.0f;
	float	m_elapsed = 0.0f;		// Start からの経過時間
	float	m_readyTime = 0.4f;
	float	m_timeLimit = 3.0f;	

private:
	void PollKeys();
	void ClearPressed();
	static int KeyIndex(int vk);

	void PlayKeyDownSound() const;

	static constexpr int kKeyCount = static_cast<int>(SkillKey::Count);
	bool m_keyDown[kKeyCount] = {};
	bool m_keyPressed[kKeyCount] = {};
	bool m_keyPrevDown[kKeyCount] = {};

	static constexpr float kLayoutOffsetY = 60.0f;
};