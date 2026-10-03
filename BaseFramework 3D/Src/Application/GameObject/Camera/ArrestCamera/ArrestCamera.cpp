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

	// ↓めり込み防止の為の座標補正計算↓
	// ①当たり判定(レイ判定)用の情報作成
	KdCollider::RayInfo rayInfo;
	// レイの発射位置を設定
	rayInfo.m_pos = GetPos();

	// レイの発射方向を設定
	rayInfo.m_dir = Math::Vector3::Down;
	// レイの長さを設定
	rayInfo.m_range = 1000.f;
	if (_spTarget)
	{
		Math::Vector3 _targetPos = _spTarget->GetPos();
		_targetPos.y += 0.1f;
		rayInfo.m_dir = _targetPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

	// ②HIT判定対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③ 結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// 何かしらの障害物に当たっている
				Math::Vector3 _hitPos = hitPos;
				_hitPos += rayInfo.m_dir * 0.4f;
				SetPos(_hitPos);
			}
		}
	}
}

void ArrestCamera::SetEyeHeight(float height)
{
	m_height = height;
	m_mLocalPos = Math::Matrix::CreateTranslation(0.0f, m_height, 0.0f);
}
