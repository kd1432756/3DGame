#pragma once

#include "../UIBase/UIBase.h"

class TimeUI : public UIBase
{
public:
	void Init() override;
	void DrawSprite() override;

private:
	void DrawTimeGauge();
	void DrawTimeIcons();
	void DrawIndicator();

	std::shared_ptr<KdTexture> m_gaugeTex = nullptr;
	std::shared_ptr<KdTexture> m_indicatorTex = nullptr;

	std::shared_ptr<KdTexture> m_sunTex = nullptr;
	std::shared_ptr<KdTexture> m_moonTex = nullptr;

	static constexpr int kStartX = -410;
	static constexpr int kStartY = 300;

	static constexpr int kGaugeWidth = 384;
	static constexpr int kGaugeHeight = 64;

	static constexpr int kGaugeLeft = 32;
	static constexpr int kGaugeRight = 352;

	static constexpr int kIndicatorSize = 32;

	static constexpr int kTimeIconSize = 48;
};
