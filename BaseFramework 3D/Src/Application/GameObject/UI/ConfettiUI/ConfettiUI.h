#pragma once

class ConfettiUI : public KdGameObject
{
public:
	ConfettiUI() {}
	~ConfettiUI() override {}

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	// 発射開始（クラッカー + 上から降らせる）
	void Start();
	// 上から降らせるのを止める（残りの紙吹雪は落ち切るまで描画）
	void Stop();

private:
	struct Particle
	{
		Math::Vector2 pos;
		Math::Vector2 vel;
		float rot = 0.0f;		// 回転角(rad)
		float rotSpeed = 0.0f;
		float flip = 0.0f;		// 裏返り用の位相
		float flipSpeed = 0.0f;
		float swayAmp = 0.0f;	// 横揺れの強さ
		float w = 0.0f;
		float h = 0.0f;
		Math::Color color;
	};

	void Burst(bool isLeft);	// クラッカー発射
	void SpawnRain();			// 上から1個生成
	Particle MakeBase();		// 共通パラメータ生成

	std::vector<Particle> m_particles;

	bool  m_active = false;
	float m_rainTimer = 0.0f;

	// ---- 調整用パラメータ ----
	static constexpr int   kBurstCount = 80;		// 片側の発射数
	static constexpr int   kMaxParticles = 400;
	static constexpr float kGravity = 300.0f;	// 下向き加速
	static constexpr float kDrag = 2.5f;	// 空気抵抗（大きいほど早く減速）
	static constexpr float kRainInterval = 0.04f;	// 上から降らせる間隔(秒)
	static constexpr float kBottom = -400.0f;	// これより下で消す
};