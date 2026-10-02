#include "CharacterBase.h"

#include "../../Scene/SceneManager.h"

#include "../../System/Score/ScoreManager.h"

void CharacterBase::Init()
{
	m_sceneScale = SceneManager::Instance().GetSceneScale();

	m_spModel = std::make_shared<KdModelWork>();

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void CharacterBase::Update()
{}

void CharacterBase::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void CharacterBase::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void CharacterBase::CheckStep(const Math::Vector3& moveDir)
{
	Math::Vector3 dir = moveDir;
	dir.y = 0.0f;

	if (dir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	dir.Normalize();

	const float scale = m_sceneScale;

	const float footOffset =
		m_footOffset * scale;

	const float stepHeight =
		m_stepHeight * scale;

	const float checkDistance =
		m_collisionRadius * scale;


	// 移動先
	Math::Vector3 checkPos =
		m_pos + dir * checkDistance;


	// 段差の上からRay
	KdCollider::RayInfo rayInfo;

	rayInfo.m_pos = checkPos;

	rayInfo.m_pos.y =
		m_pos.y - footOffset + stepHeight;

	rayInfo.m_dir = { 0.0f, -1.0f, 0.0f };

	rayInfo.m_range =
		stepHeight + 0.1f * scale;

	rayInfo.m_type =
		KdCollider::TypeGround;


	std::list<KdCollider::CollisionResult> retRayList;

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

		obj->Intersects(
			rayInfo,
			&retRayList
		);
	}


	bool hit = false;

	float maxOverlap = 0.0f;

	Math::Vector3 groundPos = {};

	for (auto& ret : retRayList)
	{
		if (maxOverlap < ret.m_overlapDistance)
		{
			maxOverlap = ret.m_overlapDistance;
			groundPos = ret.m_hitPos;
			hit = true;
		}
	}

	if (!hit)
	{
		return;
	}


	// 現在の足元
	const float currentFootY =
		m_pos.y - footOffset;

	// 段差の高さ
	const float heightDiff =
		groundPos.y - currentFootY;


	// 上り坂・段差ではない
	if (heightDiff <= 0.0f)
	{
		return;
	}

	// 登れる高さを超えている
	if (heightDiff > stepHeight)
	{
		return;
	}


	// 段差を上る
	m_pos.y += heightDiff;
}

void CharacterBase::CheckGround()
{
	const float scale = m_sceneScale;
	const float footOffset = m_footOffset * scale;
	const float stepHeight = m_stepHeight * scale;

	const bool wasGround = m_isGround;
	m_isGround = false;

	const float footY = m_pos.y - footOffset;

	KdCollider::RayInfo ray;
	ray.m_pos = { m_pos.x, footY + stepHeight, m_pos.z };
	ray.m_dir = { 0.0f, -1.0f, 0.0f };
	ray.m_type = KdCollider::TypeGround;

	// 接地中は下り段差にも吸着、空中なら重力ぶんだけ
	const float snapDown = wasGround ? stepHeight : std::abs(m_gravity);
	ray.m_range = stepHeight + snapDown;

	std::list<KdCollider::CollisionResult> hits;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj || obj.get() == this) continue;
		obj->Intersects(ray, &hits);
	}

	// 一番高い面（＝Rayの始点に最も近い面）を採用
	bool hit = false;
	float best = 0.0f;
	Math::Vector3 groundPos;
	for (auto& r : hits)
	{
		if (!hit || r.m_overlapDistance > best)
		{
			best = r.m_overlapDistance;
			groundPos = r.m_hitPos;
			hit = true;
		}
	}
	if (!hit) return;

	// 上昇中（ジャンプ中）は下り吸着しない
	const bool rising = m_gravity > 0.0f;   // ※重力の符号はプロジェクトに合わせる
	if (rising && groundPos.y < footY) return;

	const float newY = groundPos.y + footOffset;
	const float diff = newY - m_pos.y;      // + なら段差を上った

	m_pos.y = newY;
	m_isGround = true;
	m_gravity = 0.0f;

	// 上り下りの瞬間移動を見た目だけ滑らかにする
	/*if (wasGround && std::abs(diff) > 0.001f)
		m_stepVisualOffset -= diff;*/
}


void CharacterBase::ResolveCollision(const Math::Vector3& moveDir)
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
		sphere.m_sphere.Radius = m_collisionRadius * m_sceneScale;

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

			if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
			{
				if (mesh->IsCollisionEnabled() == false)
				{
					continue;
				}
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

void CharacterBase::ApplyGravity()
{
	// 重力を加える
	m_gravity -= m_gravityPower;

	// 落下
	m_pos.y += m_gravity;

	// 地面を確認
	CheckGround();
}

void CharacterBase::RotateToMoveDirection(
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

void CharacterBase::UpdateAnimation()
{
	if (!m_spAnimator)return;
	if (!m_spModel)return;

	m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
	m_spModel->CalcNodeMatrices();
}

bool CharacterBase::AddInventoryItem(const std::string& itemPath)
{
	if (itemPath.empty()) return false;
	return m_inventory.AddItem(itemPath);
}

bool CharacterBase::HasInventoryItem(const std::string& itemPath) const
{
	if (itemPath.empty()) return false;

	for (const auto& item : m_inventory.GetItems())
	{
		if (!item.IsEmpty() && item.itemPath == itemPath) return true;
	}
	return false;
}

bool CharacterBase::HasRepairItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;

		ItemData data;
		if (data.Load(item.itemPath) && data.IsRepairItem()) return true;
	}
	return false;
}

bool CharacterBase::HasPrankItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;

		ItemData data;
		if (data.Load(item.itemPath) && data.IsPrankItem()) return true;
	}
	return false;
}

void CharacterBase::ConfiscatePrankItems()
{
	const auto& items = m_inventory.GetItems();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].IsEmpty()) continue;

		ItemData data;
		if (data.Load(items[i].itemPath) && data.IsPrankItem())
		{
			m_inventory.RemoveItem(i);
		}
	}

	OnItemsConfiscated();
}

void CharacterBase::OnArrested(const std::shared_ptr<Police>& police)
{
	m_isArrested = true;
}

void CharacterBase::OnReleased()
{
	m_isArrested = false;
}