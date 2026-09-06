#include "AlertIcon.h"

#include "../../../Scene/CameraManager/CameraManager.h"

#include "../../Camera/FPSCamera/FPSCamera.h"

void AlertIcon::Init()
{
	m_spPoly = std::make_shared<KdSquarePolygon>();
	m_spPoly->SetMaterial("Asset/Textures/Effect/EmoteIcon/EmoteIcon.png");

	m_spPoly->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_spPoly->SetSplit(10, 3);

	m_spPoly->SetEnable(false);
}

void AlertIcon::Update()
{
	auto owner = m_owner.lock();

	if (!owner)
	{
		m_spPoly->SetEnable(false);
		return;
	}

	m_pos = owner->GetMatrix().Translation();
	m_pos.y += 1.4f * m_iconScale;
	
	UpdateAnimation();

	UpdateWorldMatrix();
}

void AlertIcon::DrawUnLit()
{
	if (m_spPoly && m_spPoly->IsEnable())
	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	}
}

void AlertIcon::SetIcon(IconType iconType)
{
	if (m_iconType == iconType)return;

	m_iconType = iconType;

	switch (iconType)
	{
	case IconType::None:

		m_spPoly->SetEnable(false);
		break;

	case IconType::Exclamation:

		m_spPoly->SetUVRect(8);
		m_spPoly->SetEnable(true);
		StartAnimation();
		break;
	case IconType::Question:

		m_spPoly->SetUVRect(9);
		m_spPoly->SetEnable(true);
		StartAnimation();
		break;
	}
}

void AlertIcon::UpdateAnimation()
{
	if (!m_isAnimating)
	{
		SetScale(m_iconScale);
		return;
	}

	m_animTime += 1.0f / 60.0f;

	float t = m_animTime / m_animDuration;

	if (t >= 1.0f)
	{
		t = 1.0f;
		m_isAnimating = false;
	}

	// --------------------------------
	// Scale
	// 0 → 1.3 → 0.9 → 1.0
	// --------------------------------

	if (t < 0.4f)
	{
		float p = t / 0.4f;

		SetScale(std::lerp(0.0f, m_iconScale * 1.3f, p));
	}
	else
	{
		float p = (t - 0.4f) / 0.6f;

		SetScale(std::lerp(m_iconScale * 1.3f, m_iconScale, p));
	}

	// --------------------------------
	// 上に跳ねる
	// --------------------------------

	float jump = std::sin(t * DirectX::XM_PI);

	float jumpHeight = 0.20f * m_iconScale;

	m_pos.y += jump * jumpHeight;
}

void AlertIcon::UpdateWorldMatrix()
{
	auto camera = CameraManager::Instance().GetCamera();
	if (!camera)
		return;

	Math::Vector3 cameraPos = camera->GetMatrix().Translation();

	Math::Vector3 dir = cameraPos - m_pos;
	dir.y = 0.0f;

	if (dir.LengthSquared() < 0.0001f)
		return;

	dir.Normalize();

	const float yaw = std::atan2(dir.x, dir.z) + DirectX::XM_PI;

	m_mWorld =
		Math::Matrix::CreateScale(m_scale)
		* Math::Matrix::CreateRotationY(yaw)
		* Math::Matrix::CreateTranslation(m_pos);
}

void AlertIcon::StartAnimation()
{
	m_animTime = 0.0f;
	m_isAnimating = true;
	SetScale(0.0f);
}

