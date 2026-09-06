#include "BaseCharacter.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

void BaseCharacter::Init()
{
	m_sceneScale = SceneManager::Instance().GetSceneScale();

	m_spModel = std::make_shared<KdModelWork>();
}

void BaseCharacter::Update()
{}

void BaseCharacter::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void BaseCharacter::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void BaseCharacter::CheckGround()
{
	m_isGround = false;

	float enableStepHigh = 0.18f * SceneManager::Instance().GetSceneScale();
	KdCollider::RayInfo ray;
	ray.m_pos = m_pos;
	ray.m_pos.y += enableStepHigh;
	ray.m_dir = { 0,-1,0 };
	ray.m_range = enableStepHigh - m_gravity + m_sceneScale;
	ray.m_type = KdCollider::TypeGround;

	std::list<KdCollider::CollisionResult>retRayList;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj)
		{
			continue;
		}

		if (obj.get() == this)
		{
			continue;
		}

		obj->Intersects(ray, &retRayList);
	}

	float maxOverlap = 0;
	Math::Vector3 hitPos;
	bool hit = false;
	for (auto& ret : retRayList)
	{
		if (ret.m_overlapDistance > maxOverlap)
		{
			maxOverlap = ret.m_overlapDistance;
			hitPos = ret.m_hitPos;
			hit = true;
		}
	}
	if (hit)
	{
		m_isGround = true;
		m_gravity = 0.0f;
		m_pos.y = hitPos.y + m_sceneScale;
	}
	if (m_pos.y < 0)
	{
		m_isGround = true;
		m_gravity = 0.0f;
		m_pos.y = 0;
	}
}

void BaseCharacter::ResolveCollision(const Math::Vector3& moveDir)
{
	// =========================================================
	// 移動していないなら何もしない
	// =========================================================
	if (moveDir.LengthSquared() <= 0.000001f)
	{
		return;
	}


	// =========================================================
	// プレイヤーの移動方向
	// =========================================================
	Math::Vector3 direction = moveDir;
	direction.Normalize();


	// =========================================================
	// 最大反復回数
	//
	// 壁＋壁、壁＋角などに対応するため、
	// 押し戻す → 再判定を数回行う
	// =========================================================
	const int maxIterations = 3;


	for (int iteration = 0; iteration < maxIterations; ++iteration)
	{
		// =====================================================
		// プレイヤーの衝突Sphere
		// =====================================================
		KdCollider::SphereInfo sphere;

		sphere.m_sphere.Center = m_pos;
		sphere.m_sphere.Center.y += m_sceneScale;
		sphere.m_sphere.Radius = m_sceneScale;

		sphere.m_type = KdCollider::TypeBump;


		// =====================================================
		// 衝突結果
		// =====================================================
		std::list<KdCollider::CollisionResult> collisionList;


		// =====================================================
		// シーン内のオブジェクトと衝突判定
		// =====================================================
		for (auto& obj : SceneManager::Instance().GetObjList())
		{
			if (!obj)
			{
				continue;
			}

			// 自分自身は除外
			if (obj.get() == this)
			{
				continue;
			}

			obj->Intersects(
				sphere,
				&collisionList
			);
		}


		// =====================================================
		// 衝突していない
		// =====================================================
		if (collisionList.empty())
		{
			break;
		}


		// =====================================================
		// 今回一番深くめり込んでいる壁を探す
		// =====================================================
		bool foundCollision = false;

		Math::Vector3 bestNormal = Math::Vector3::Zero;
		float bestOverlap = 0.0f;


		for (auto& collision : collisionList)
		{
			Math::Vector3 normal = collision.m_hitNDir;


			// -------------------------------------------------
			// 法線が取得できない場合
			// -------------------------------------------------
			if (normal.LengthSquared() <= 0.000001f)
			{
				continue;
			}

			normal.Normalize();


			// -------------------------------------------------
			// 地面・坂は無視
			// -------------------------------------------------
			if (normal.y > 0.7f)
			{
				continue;
			}


			// -------------------------------------------------
			// 天井も壁として扱わない
			// -------------------------------------------------
			if (normal.y < -0.7f)
			{
				continue;
			}


			// -------------------------------------------------
			// 水平方向だけを使用
			// -------------------------------------------------
			normal.y = 0.0f;

			if (normal.LengthSquared() <= 0.000001f)
			{
				continue;
			}

			normal.Normalize();


			// -------------------------------------------------
			// 壁に向かって移動している場合だけ処理
			// -------------------------------------------------
			float dot = direction.Dot(normal);

			if (dot >= 0.0f)
			{
				continue;
			}


			// -------------------------------------------------
			// めり込み量
			// -------------------------------------------------
			float overlap = collision.m_overlapDistance;

			if (overlap <= 0.0f)
			{
				continue;
			}


			// -------------------------------------------------
			// 一番深い衝突を採用
			// -------------------------------------------------
			if (!foundCollision || overlap > bestOverlap)
			{
				foundCollision = true;
				bestOverlap = overlap;
				bestNormal = normal;
			}
		}


		// =====================================================
		// 壁との衝突がなければ終了
		// =====================================================
		if (!foundCollision)
		{
			break;
		}


		// =====================================================
		// めり込みを完全に解消
		// =====================================================
		const float skin = 0.001f;

		m_pos += bestNormal * (bestOverlap + skin);
	}
}

void BaseCharacter::ApplyGravity()
{
	if (m_isGround)
	{
		m_gravity = 0.0f;
		return;
	}

	// 重力を加える
	m_gravity -= m_gravityPower;

	// 落下
	m_pos.y += m_gravity;

	// 地面を確認
	CheckGround();
}

void BaseCharacter::RotateToMoveDirection(
	const Math::Vector3& dir)
{
	if (dir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	Math::Vector3 toDir = dir;
	toDir.y = 0.0f;
	toDir.Normalize();


	// =========================================
	// 現在の向き
	// =========================================

	Math::Matrix nowRotMat =
		Math::Matrix::CreateRotationY(
			DirectX::XMConvertToRadians(m_rot.y));

	Math::Vector3 nowDir =
		Math::Vector3::TransformNormal(
			Math::Vector3(0, 0, -1),
			nowRotMat);

	nowDir.y = 0.0f;
	nowDir.Normalize();


	// =========================================
	// 内積から角度を求める
	// =========================================

	float dot = nowDir.Dot(toDir);

	dot = std::clamp(dot, -1.0f, 1.0f);

	float angle =
		DirectX::XMConvertToDegrees(
			acosf(dot));


	// =========================================
	// ほぼ向いている
	// =========================================

	if (angle < 0.1f)
	{
		return;
	}


	// =========================================
	// 1フレーム最大5度
	// =========================================

	angle = std::min(angle, 5.0f);


	// =========================================
	// 左右判定
	// =========================================

	Math::Vector3 cross =
		nowDir.Cross(toDir);

	if (cross.y >= 0.0f)
	{
		m_rot.y += angle;
	}
	else
	{
		m_rot.y -= angle;
	}


	// =========================================
	// 角度を0～360度に整理
	// =========================================

	if (m_rot.y >= 360.0f)
	{
		m_rot.y -= 360.0f;
	}
	else if (m_rot.y < 0.0f)
	{
		m_rot.y += 360.0f;
	}

	UpdateWorldMatrix();
}

void BaseCharacter::UpdateAnimation()
{
	if (!m_spAnimator)return;
	if (!m_spModel)return;

	m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
	m_spModel->CalcNodeMatrices();
}
