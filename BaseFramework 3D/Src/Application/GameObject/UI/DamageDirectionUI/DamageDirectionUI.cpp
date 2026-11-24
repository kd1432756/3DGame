#include "DamageDirectionUI.h"
#include "../../Character/Player/Player.h"
#include "../../../Scene/CameraManager/CameraManager.h"
#include "../../../GameObject/Camera/CameraBase.h"
#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

void DamageDirectionUI::Init()
{
	m_tex = LoadTexture("Asset/Textures/UI/House.png");
}

std::shared_ptr<KdTexture> DamageDirectionUI::GetIcon(const std::string& path)
{
	if (path.empty()) return m_tex;	// 指定なしは既定アイコン

	auto it = m_iconCache.find(path);
	if (it != m_iconCache.end()) return it->second;

	auto tex = LoadTexture(path);
	m_iconCache[path] = tex;
	return tex;
}

void DamageDirectionUI::AddTarget(const std::shared_ptr<InteractiveObject>& obj)
{
	Marker m;
	m.obj = obj;
	m.pos = obj->GetPos();
	m.icon = GetIcon(obj->GetIconPath());
	m.alertTimer = kAlertTime;
	m_markers.push_back(m);
}

void DamageDirectionUI::RemoveTarget(const std::shared_ptr<InteractiveObject>& obj)
{
	m_markers.erase(
		std::remove_if(m_markers.begin(), m_markers.end(),
			[&](const Marker& m) { return m.obj.lock() == obj; }),
		m_markers.end());
}

void DamageDirectionUI::PostUpdate()
{
	// 削除済みを掃除
	m_markers.erase(
		std::remove_if(m_markers.begin(), m_markers.end(),
			[](const Marker& m) { return m.obj.expired(); }),
		m_markers.end());

	auto owner = LockPlayer();
	auto camera = CameraManager::Instance().GetCamera();
	if (!owner || !camera) return;

	const float dt = DeltaTime();

	// 画面端の範囲
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);
	const float limitX = vp.width * 0.5f * (1.0f - kEdgeMargin);
	const float limitY = vp.height * 0.5f * (1.0f - kEdgeMargin);

	const Math::Matrix view = camera->GetMatrix().Invert();

	for (auto& m : m_markers)
	{
		const Math::Vector3 v = Math::Vector3::Transform(m.pos, view);

		// 正面度合い
		m.facing = v.Length() > 0.001f ? v.z / v.Length() : 1.0f;

		// 画面内にいるか
		Math::Vector3 result;
		camera->ConvertWorldToScreenDetail(m.pos, result);
		m.onScreen = (v.z > 0.001f) && fabsf(result.x) < limitX && fabsf(result.y) < limitY;

		if (m.onScreen)
		{
			m.uiPos = { result.x, result.y };
		}
		else
		{
			// 画面外
			Math::Vector2 dir = { v.x, v.y };
			if (dir.LengthSquared() < 0.0001f) dir = { 0.0f, -1.0f };	// 真後ろは下
			dir.Normalize();

			const float sx = fabsf(dir.x) > 0.0001f ? limitX / fabsf(dir.x) : FLT_MAX;
			const float sy = fabsf(dir.y) > 0.0001f ? limitY / fabsf(dir.y) : FLT_MAX;

			m.dir = dir;
			m.uiPos = dir * (std::min)(sx, sy);
		}

		// 透明度を変える
		const bool bright = (m.facing >= kFacingLimit);
		const float target = bright ? kBrightAlpha : kDimAlpha;
		m.alpha += (target - m.alpha) * (std::min)(dt * 10.0f, 1.0f);

		if (m.alertTimer > 0.0f) m.alertTimer -= dt;
	}
}

void DamageDirectionUI::DrawSprite()
{
	auto player = LockPlayer();

	if (!player) return;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	for (const auto& m : m_markers)
	{
		if (m.alpha <= 0)return;

		// 壊された直後は点滅
		Math::Color color = { 1.0f, 1.0f, 1.0f, m.alpha };
		if (m.alertTimer > 0.0f)
		{
			const float blink = 0.5f + 0.5f * sinf(m.alertTimer * 12.0f);
			color = { 1.0f, 0.2f, 0.2f, 0.6f + 0.4f * blink };
		}

		Math::Vector2 iconPos = m.uiPos;

		if (!m.onScreen)
		{
			// 三角形
			const Math::Vector2 f = m.dir;
			const Math::Vector2 r = { f.y, -f.x };
			const float L = kArrowSize;
			const Math::Vector2 tip = m.uiPos + f * L * 0.5f;
			const Math::Vector2 left = m.uiPos - f * L * 0.5f - r * L * 0.5f;
			const Math::Vector2 right = m.uiPos - f * L * 0.5f + r * L * 0.5f;

			sprite.DrawTriangle((int)tip.x, (int)tip.y, (int)left.x, (int)left.y,
				(int)right.x, (int)right.y, &color, true);

			iconPos = m.uiPos - f * L * 0.9f;
		}

		// アイコン
		sprite.DrawTex(m.icon, (int)iconPos.x, (int)iconPos.y,
			(int)kIconSize, (int)kIconSize, nullptr, &color);
	}
}