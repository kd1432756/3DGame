#pragma once

class DayCycle;

class TimeUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

private:
	void DrawGauge();
	void DrawTimeIcons();
	void DrawIndicator();

	std::shared_ptr<KdTexture> m_gaugeTex = nullptr;
	std::shared_ptr<KdTexture> m_indicatorTex = nullptr;

	std::shared_ptr<KdTexture> m_sunTex = nullptr;
	std::shared_ptr<KdTexture> m_moonTex = nullptr;

	// UIの位置
	static constexpr int StartX = -410;
	static constexpr int StartY = 300;

	// ゲージのサイズ
	static constexpr int GaugeWidth = 384;
	static constexpr int GaugeHeight = 64;

	// ゲージ上の○の位置
	static constexpr int GaugeLeft = 32;
	static constexpr int GaugeRight = 352;

	// △のサイズ
	static constexpr int IndicatorSize = 32;

	// 太陽・月のサイズ
	static constexpr int TimeIconSize = 48;
};