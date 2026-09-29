#pragma once

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
	static bool  IsKeyPressed(int vk);	// 押したフレームだけ true
	static bool  IsKeyDown(int vk);		// 押している間 true
	static void  DrainInput();			// 溜まっている入力を読み捨てる

	// 画面中央原点・ピクセル指定の矩形描画(halfW/halfH は半分のサイズ)
	static void  DrawRect(float x, float y, float halfW, float halfH, const Math::Color& color);

protected:
	SkillResult m_result = SkillResult::None;
	float       m_difficulty = 1.0f;
	float       m_elapsed = 0.0f;	// Start からの経過時間
	float       m_readyTime = 0.4f;	// この間は入力を受け付けない(開始時の誤入力防止＆予告)
	float       m_timeLimit = 3.0f;	// 入力受付開始から数える。超えたらMiss(OnStartで変更可)
};