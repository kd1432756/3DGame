#include "HomeDirectionUI.h"
#include "../../Character/Player/Player.h"
#include "../../../Scene/CameraManager/CameraManager.h"
#include "../../../GameObject/Camera/CameraBase.h"
#include "../../../System/Location/LocationPointManager.h"
#include "../../../Scene/SceneManager.h"

void HomeDirectionUI::Init()
{
	m_texHouse = LoadTexture("Asset/Textures/UI/House.png");

	m_sceneScale = SceneManager::Instance().GetSceneScale();
}

void HomeDirectionUI::Update()
{
	auto owner = LockPlayer();
	if (!owner) return;

	const float dt = DeltaTime();

	// --- 自宅位置の取得（未取得の間だけ試みる） ---
	if (!m_hasHomePos)
	{
		auto point = LocationPointManager::Instance().FindByTag(LocationTag::PlayerHouse);
		if (point)
		{
			m_homePos = point->GetPos();
			m_hasHomePos = true;
		}
		else
		{
			return;
		}
	}

	// --- 距離判定（ヒステリシス） ---
	const Math::Vector3 playerPos = owner->GetPos();   // ★Playerの座標取得に合わせる
	m_distance = (m_homePos - playerPos).Length();

	if (m_isVisible) { if (m_distance <= kHideDistance * m_sceneScale) m_isVisible = false; }
	else { if (m_distance >= kShowDistance * m_sceneScale) m_isVisible = true; }

	// --- 強調表示 ---
	bool alert = false;
	if (m_alertTimer > 0.0f)
	{
		m_alertTimer -= dt;
		if (m_alertTimer < 0.0f) m_alertTimer = 0.0f;
		alert = true;
	}

	// --- 被害表示 ---
	m_count = owner->GetDamagedCount();
	const bool damaged = (m_count > 0);

	// --- フェード（強調中は距離に関係なく表示） ---
	const bool show = m_isVisible || alert;
	const float targetAlpha = show ? 1.0f : 0.0f;
	if (m_alpha < targetAlpha) m_alpha = (std::min)(m_alpha + kFadeSpeed * dt, targetAlpha);
	else                       m_alpha = (std::max)(m_alpha - kFadeSpeed * dt, targetAlpha);

	// --- 色（通常：半透明の白、強調：赤く点滅） ---
	if (alert)
	{
		const float blink = 0.5f + 0.5f * sinf(m_alertTimer * 12.0f);
		m_color = { 1.0f, 0.2f, 0.2f, m_alpha * (0.6f + 0.4f * blink) };
	}
	else if (damaged)
	{
		// DamageIndicatorUIと同じ赤み。通常より少し濃くして目立たせる
		m_color = { 1.0f, 0.6f, 0.6f, m_alpha * (kBaseAlpha + 0.2f) };
	}
	else
	{
		m_color = { 1.0f, 1.0f, 1.0f, m_alpha * kBaseAlpha };
	}

	// --- スクリーン位置の計算 ---
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);
	const float halfW = vp.width * 0.5f;
	const float halfH = vp.height * 0.5f;
	const float limitX = halfW * (1.0f - kEdgeMargin);
	const float limitY = halfH * (1.0f - kEdgeMargin);

	auto camera = CameraManager::Instance().GetCamera();
	if (!camera) return;

	// ワールド → カメラ空間（x:右, y:上, z:前）
	// ※カメラのワールド行列の取得関数名は環境に合わせて変更
	const Math::Matrix view = camera->GetMatrix().Invert();
	const Math::Vector3 viewPos = Math::Vector3::Transform(m_homePos, view);

	const bool inFront = viewPos.z > 0.001f;

	Math::Vector2 screen;
	m_onScreen = false;
	if (inFront)
	{
		// 前方の場合のみ、従来の射影で画面座標を求める
		WorldToScreen(m_homePos, screen);
		m_onScreen = fabsf(screen.x) < limitX && fabsf(screen.y) < limitY;
	}

	if (m_onScreen)
	{
		m_uiPos = screen;
	}
	else
	{
		// 画面外・後方：カメラ空間の x,y をそのまま矢印の方向にする
		Math::Vector2 dir = { viewPos.x, viewPos.y };

		// 真後ろ（x,yがほぼ0）のときは下向きに固定（「後ろ」の意味になる）
		if (dir.LengthSquared() < 0.0001f) dir = { 0.0f, -1.0f };
		dir.Normalize();

		const float sx = (fabsf(dir.x) > 0.0001f) ? limitX / fabsf(dir.x) : FLT_MAX;
		const float sy = (fabsf(dir.y) > 0.0001f) ? limitY / fabsf(dir.y) : FLT_MAX;
		const float s = (std::min)(sx, sy);

		m_uiPos = dir * s;
		m_arrowDir = dir;
	}
}

void HomeDirectionUI::DrawSprite()
{
	if (!m_hasHomePos || m_alpha <= 0.0f) return;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	if (m_onScreen)
	{
		// 家の位置に家アイコンのみ
		sprite.DrawTex(m_texHouse,
			(int)m_uiPos.x, (int)m_uiPos.y,
			(int)kHouseSize, (int)kHouseSize,
			nullptr, &m_color);
	}
	else
	{
		// 三角形
		DrawArrow(m_uiPos, m_arrowDir, m_color);

		// 三角形の内側寄りに家アイコン
		const Math::Vector2 housePos = m_uiPos - m_arrowDir * kArrowSize * 0.9f;
		sprite.DrawTex(m_texHouse,
			(int)housePos.x, (int)housePos.y,
			(int)kHouseSize, (int)kHouseSize,
			nullptr, &m_color);
	}
}

bool HomeDirectionUI::WorldToScreen(const Math::Vector3& worldPos, Math::Vector2& outScreen) const
{
	auto camera = CameraManager::Instance().GetCamera();
	if (!camera) return false;

	Math::Vector3 result;
	camera->ConvertWorldToScreenDetail(worldPos, result);

	outScreen = { result.x, result.y };
	return result.z > 0.001f;   // w > 0 が前方
}

void HomeDirectionUI::DrawArrow(const Math::Vector2& center, const Math::Vector2& dir, const Math::Color& color)
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	const Math::Vector2 f = dir;
	const Math::Vector2 r = { f.y, -f.x };

	const float L = kArrowSize;
	const Math::Vector2 tip = center + f * L * 0.55f;
	const Math::Vector2 left = center - f * L * 0.45f - r * L * 0.5f;
	const Math::Vector2 right = center - f * L * 0.45f + r * L * 0.5f;
	const Math::Vector2 notch = center - f * L * 0.1f;   // 後ろの切れ込み

	// 左半分
	sprite.DrawTriangle((int)tip.x, (int)tip.y, (int)left.x, (int)left.y,
		(int)notch.x, (int)notch.y, &color, true);
	// 右半分
	sprite.DrawTriangle((int)tip.x, (int)tip.y, (int)notch.x, (int)notch.y,
		(int)right.x, (int)right.y, &color, true);
}
