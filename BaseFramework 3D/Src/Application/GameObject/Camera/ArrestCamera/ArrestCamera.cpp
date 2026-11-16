#include "ArrestCamera.h"

#include "../../../Scene/SceneManager.h"

void ArrestCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 1.5f, -1.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_state = State::PullingOut;
	m_transitionTimer = 0.0f;
	m_isFinished = false;
}

void ArrestCamera::PostUpdate()
{
	// 目標とするTPSの通常距離（例: -10.0f）と、FPSの近い距離（例: -1.0f または 0.0f）
	float startZ = -0.0f * SceneManager::Instance().GetSceneScale();
	float targetZ = -10.0f * SceneManager::Instance().GetSceneScale();
	float currentZ = -10.0f * SceneManager::Instance().GetSceneScale();

	// 状態に応じたズーム処理
	if (m_state == State::PullingOut)
	{
		m_transitionTimer += 1.0f / 60.0f;;
		float t = m_transitionTimer / m_transitionDuration;
		if (t >= 1.0f) { t = 1.0f; m_state = State::Normal; } // 引き終わり

		// 補間（イージングを入れたい場合はここに数式を挟むと滑らかになります）
		currentZ = std::lerp(startZ, targetZ, t);
	}
	else if (m_state == State::Normal)
	{
		currentZ = targetZ;
	}
	else if (m_state == State::PushingIn)
	{
		m_transitionTimer += 1.0f / 60.0f;
		float t = m_transitionTimer / m_transitionDuration;
		if (t >= 1.0f) { t = 1.0f; m_isFinished = true; } // ズームイン完了

		// 逆に遠くから近く（FPSの目線）へ戻る
		currentZ = std::lerp(targetZ, startZ, t);
	}

	// ローカル位置のZを更新（高さ1.5f、ZをcurrentZにする）
	m_mLocalPos = Math::Matrix::CreateTranslation(0.0f, m_height, currentZ);

	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	// カメラの回転
	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

	Math::Vector3 idealPos = m_mWorld.Translation();

	//========================================
	// めり込み防止処理
	//========================================

	KdCollider::RayInfo rayInfo;

	// Rayの開始位置：ターゲット
	rayInfo.m_pos = _spTarget->GetPos();

	// 少し高さを加える
	rayInfo.m_pos.y += m_height;

	// ターゲットからカメラへの方向
	rayInfo.m_dir = idealPos - rayInfo.m_pos;

	// Rayの長さ
	rayInfo.m_range = rayInfo.m_dir.Length();

	if (rayInfo.m_range > 0.001f)
	{
		rayInfo.m_dir.Normalize();
	}

	// 判定対象
	rayInfo.m_type = KdCollider::TypeGround || KdCollider::TypeBump;

	// 最も近い衝突位置を探す
	float minDistance = rayInfo.m_range;
	bool hit = false;

	Math::Vector3 hitPos = {};

	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();

		if (!spGameObj) continue;

		std::list<KdCollider::CollisionResult> retRayList;

		spGameObj->Intersects(rayInfo, &retRayList);

		for (auto& ret : retRayList)
		{
			float distance = (ret.m_hitPos - rayInfo.m_pos).Length();

			if (distance < minDistance)
			{
				minDistance = distance;
				hitPos = ret.m_hitPos;
				hit = true;
			}
		}
	}

	//========================================
	// カメラ位置の補正
	//========================================

	if (hit)
	{
		// 衝突位置から少し手前に移動
		const float offset = 0.4f;

		Math::Vector3 cameraPos = hitPos - rayInfo.m_dir * offset;

		m_mWorld.Translation(cameraPos);
	}
}

void ArrestCamera::SetEyeHeight(float height)
{
	m_height = height;
	m_mLocalPos = Math::Matrix::CreateTranslation(0.0f, m_height, 0.0f);
}
