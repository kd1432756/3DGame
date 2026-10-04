#include "EmoteIcon.h"

#include "../../../Scene/SceneManager.h"

#include "../../Character/CharacterBase.h"

#include "../../Camera/CameraBase.h"
#include "../../../Scene/CameraManager/CameraManager.h"

void EmoteIcon::Init()
{
	if (m_iconPolygon)
	{
		return;
	}

	m_iconPolygon = std::make_shared<KdSquarePolygon>();

	m_iconPolygon->SetMaterial("Asset/Textures/Effect/EmoteIcon/EmoteIcon.png");

	m_iconPolygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_iconPolygon->SetSplit(10, 3);
	m_iconPolygon->SetEnable(false);

	m_baseScale = 1.0f * SceneManager::Instance().GetSceneScale();
}

void EmoteIcon::Update()
{
	auto ownerCharacter = m_ownerCharacter.lock();

	if (!ownerCharacter)
	{
		m_iconPolygon->SetEnable(false);
		return;
	}

	//========================================
	// ライフ処理
	//========================================

	if (m_hasLife)
	{
		m_lifeTime -= 1.0f / 60.0f;
		if (m_lifeTime <= 0.0f)
		{
			SetIcon(IconType::None);
		}
	}

	//========================================
	// キャラクターの頭上に配置
	//========================================

	m_pos = ownerCharacter->GetMatrix().Translation();
	m_pos.y += m_heightOffset * m_baseScale;

	//========================================
	// アイコンアニメーション
	//========================================

	UpdateIconAnimation();

	//========================================
	// ワールド行列更新
	//========================================

	UpdateIconWorldMatrix();
}

void EmoteIcon::DrawUnLit()
{
	if (!m_iconPolygon || !m_iconPolygon->IsEnable())
	{
		return;
	}

	KdShaderManager::Instance()
		.m_StandardShader
		.DrawPolygon(*m_iconPolygon, m_mWorld);
}

void EmoteIcon::SetIcon(IconType iconType, float duration, bool force)
{
	const bool same = (m_currentIcon == iconType);

	m_hasLife = (duration > 0.0f);
	m_lifeTime = duration;

	if (same && !force) return;

	m_currentIcon = iconType;

	switch (m_currentIcon)
	{
	case IconType::None:

		m_iconPolygon->SetEnable(false);
		break;

	case IconType::Exclamation:

		m_iconPolygon->SetUVRect(8);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Question:

		m_iconPolygon->SetUVRect(9);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Sad:

		m_iconPolygon->SetUVRect(14);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Angry:

		m_iconPolygon->SetUVRect(15);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Alert:

		m_iconPolygon->SetUVRect(16);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Stars:

		m_iconPolygon->SetUVRect(18);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Music:

		m_iconPolygon->SetUVRect(19);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Anger:

		m_iconPolygon->SetUVRect(22);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Idea:

		m_iconPolygon->SetUVRect(27);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	case IconType::Laugh:

		m_iconPolygon->SetUVRect(28);
		m_iconPolygon->SetEnable(true);

		StartIconAnimation();
		break;

	default:

		break;
	}
}

void EmoteIcon::UpdateIconAnimation()
{
	if (!m_isAnimationPlaying)
	{
		SetScale(m_baseScale);
		return;
	}

	//========================================
	// アニメーション時間更新
	//========================================

	m_animationElapsed += 1.0f / 60.0f;

	float progress =
		m_animationElapsed / m_animationDuration;

	if (progress >= 1.0f)
	{
		progress = 1.0f;
		m_isAnimationPlaying = false;
	}

	//========================================
	// スケールアニメーション
	//
	// 0 → 1.3 → 1.0
	//========================================

	if (progress < 0.4f)
	{
		const float scaleProgress = progress / 0.4f;

		SetScale(
			std::lerp(
				0.0f,
				m_baseScale * 1.3f,
				scaleProgress
			)
		);
	}
	else
	{
		const float scaleProgress =
			(progress - 0.4f) / 0.6f;

		SetScale(
			std::lerp(
				m_baseScale * 1.3f,
				m_baseScale,
				scaleProgress
			)
		);
	}

	//========================================
	// 上方向への跳ね上げ
	//========================================

	const float jumpProgress =
		std::sin(progress * DirectX::XM_PI);

	const float jumpHeight =
		0.20f * m_baseScale;

	m_pos.y += jumpProgress * jumpHeight;
}

void EmoteIcon::UpdateIconWorldMatrix()
{
	auto camera = CameraManager::Instance().GetCamera();

	if (!camera)
	{
		return;
	}

	const Math::Vector3 cameraPosition =
		camera->GetMatrix().Translation();

	// アイコンからカメラへの方向
	Math::Vector3 direction =
		cameraPosition - m_pos;

	// Y方向は無視して水平回転だけ求める
	direction.y = 0.0f;

	if (direction.LengthSquared() < 0.0001f)
	{
		return;
	}

	direction.Normalize();

	const float yaw =
		std::atan2(direction.x, direction.z)
		+ DirectX::XM_PI;

	m_mWorld =
		Math::Matrix::CreateScale(m_scale)
		* Math::Matrix::CreateRotationY(yaw)
		* Math::Matrix::CreateTranslation(m_pos);
}

void EmoteIcon::StartIconAnimation()
{
	m_animationElapsed = 0.0f;
	m_isAnimationPlaying = true;

	SetScale(0.0f);
}
