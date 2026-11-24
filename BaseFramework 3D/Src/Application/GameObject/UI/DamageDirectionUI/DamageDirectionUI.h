#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class InteractiveObject;
		
class DamageDirectionUI : public PlayerUIBase
{
public:
	void Init() override;
	void PostUpdate() override;
	void DrawSprite() override;

	void AddTarget(const std::shared_ptr<InteractiveObject>& obj);
	void RemoveTarget(const std::shared_ptr<InteractiveObject>& obj);

	std::shared_ptr<KdTexture> GetIcon(const std::string& path);
private:
	struct Marker
	{
		std::weak_ptr<InteractiveObject> obj;
		Math::Vector3 pos = Math::Vector3::Zero;

		float alpha = 0.0f;
		float alertTimer = 0.0f;		// 生成時の赤点滅

		float distance = 0.0f;
		float facing = -1.0f;	// 1が中心で0まで正面
		bool  onScreen = false;
		Math::Vector2 uiPos = Math::Vector2::Zero;
		Math::Vector2 dir = { 0.0f, 1.0f };		// 矢印の向き
		std::shared_ptr<KdTexture> icon;
	};

	std::vector<Marker> m_markers;

	std::shared_ptr<KdTexture> m_tex;	// アイコン
	std::unordered_map<std::string, std::shared_ptr<KdTexture>> m_iconCache;

	// ---- 調整用パラメータ ----
	static constexpr float kArrowSize = 40.0f;
	static constexpr float kIconSize = 32.0f;
	static constexpr float kEdgeMargin = 0.08f;		// マージン
	static constexpr float kFacingLimit = 0.87f;
	static constexpr float kDimAlpha = 0.45f;	// 薄い時
	static constexpr float kBrightAlpha = 0.9f;	// 濃い時
	static constexpr float kAlertTime = 3.0f;		// 点滅の長さ
};