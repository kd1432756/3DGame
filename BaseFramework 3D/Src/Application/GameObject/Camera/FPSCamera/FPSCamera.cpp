#include "FPSCamera.h"

void FPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 基準点(ターゲット)の目線
	//m_mLocalPos = Math::Matrix::CreateTranslation(0.0f, 1.5f, 0.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_DegAng = { 0.0f,180.0f,0.0f };
}

void FPSCamera::Update()
{
	Math::Matrix _targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject> _spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		Math::Vector3 targetPos = _spTarget->GetPos();

		// 初回、または大きく離れた場合（テレポート等）は即座に合わせる
		if (!m_smoothYInit || std::abs(targetPos.y - m_smoothY) > 5.0f)
		{
			m_smoothY = targetPos.y;
			m_smoothYInit = true;
		}
		else
		{
			const float t = 1.0f - std::exp(-m_ySmoothSpeed * 1.0f / 60.0f);
			m_smoothY += (targetPos.y - m_smoothY) * t;
		}

		targetPos.y = m_smoothY;
		_targetMat = Math::Matrix::CreateTranslation(targetPos);
	}

	// カメラの回転
	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;
}

void FPSCamera::SetEyeHeight(float height)
{
	m_mLocalPos = Math::Matrix::CreateTranslation(0.0f, height, 0.0f);
}
