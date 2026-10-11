#include "CharacterBase.h"

#include "../../Scene/SceneManager.h"

#include "../../System/Score/ScoreManager.h"

#include "../../System/FootStep/FootStep.h"

#include "../UI/EmoteIcon/EmoteIcon.h"

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

	// 一番高い面を採用 + そのオブジェクトも記録 
	bool hit = false;
	float best = 0.0f;
	Math::Vector3 groundPos;
	std::shared_ptr<KdGameObject> groundObj;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj || obj.get() == this) continue;

		std::list<KdCollider::CollisionResult> objHits;
		if (!obj->Intersects(ray, &objHits)) continue;

		for (auto& r : objHits)
		{
			if (!hit || r.m_overlapDistance > best)
			{
				best = r.m_overlapDistance;
				groundPos = r.m_hitPos;
				groundObj = obj;
				hit = true;
			}
		}
	}
	if (!hit)
	{
		char buf[256];
		sprintf_s(buf, "[CheckGround] no hit pos=(%.2f, %.2f, %.2f) rayY=%.2f range=%.2f\n",
			m_pos.x, m_pos.y, m_pos.z, ray.m_pos.y, ray.m_range);
		OutputDebugStringA(buf);
		m_groundType = Ground::GroundType::None;
		return;
	}

	// 地面素材を更新
	m_groundType = Ground::GroundType::None;
	if (auto ground = std::dynamic_pointer_cast<Ground>(groundObj))
	{
		m_groundType = ground->GetGroundType();
	}

	// 上昇中（ジャンプ中）は下り吸着しない
	const bool rising = m_gravity > 0.0f;   // ※重力の符号はプロジェクトに合わせる
	if (rising && groundPos.y < footY) return;

	const float newY = groundPos.y + footOffset;
	const float diff = newY - m_pos.y;      // + なら段差を上った

	m_pos.y = newY;
	m_isGround = true;
	m_gravity = 0.0f;

	// 上り下りの瞬間移動を見た目だけ滑らかにする
	/*/if (wasGround && std::abs(diff) > 0.001f)
	 		m_stepVisualOffset -= diff;*/
}

void CharacterBase::CollectWallHits(float radius, std::vector<WallHit>& out)
{
	out.clear();

	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Radius = radius;
	sphere.m_type = KdCollider::TypeBump;

	std::list<KdCollider::CollisionResult> collisionList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj || obj.get() == this) { continue; }

		if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (!mesh->IsCollisionEnabled()) { continue; }
		}
		obj->Intersects(sphere, &collisionList);
	}

	for (auto& c : collisionList)
	{
		if (c.m_overlapDistance <= 0.0f) { continue; }

		Math::Vector3 n = c.m_hitNDir;
		if (n.LengthSquared() <= 0.000001f) { continue; }
		n.Normalize();

		//if (n.y > 0.7f || n.y < -0.7f) { continue; }   // 床・天井は除外

		n.y = 0.0f;
		if (n.LengthSquared() <= 0.000001f) { continue; }
		n.Normalize();

		// 近い法線は統合
		bool merged = false;
		for (auto& w : out)
		{
			if (w.normal.Dot(n) > 0.98f)
			{
				w.overlap = std::max(w.overlap, c.m_overlapDistance);
				merged = true;
				break;
			}
		}
		if (!merged) { out.push_back({ n, c.m_overlapDistance }); }
	}
}

void CharacterBase::ResolveCollision()
{
	const float radius = m_collisionRadius * m_sceneScale;
	const float skin = 0.001f;

	std::vector<WallHit> walls;

	for (int i = 0; i < 5; ++i)
	{
		CollectWallHits(radius, walls);
		if (walls.empty()) { break; }

		Math::Vector3 push = Math::Vector3::Zero;
		for (auto& w : walls)
		{
			push += w.normal * (w.overlap + skin);
		}

		const float len = push.Length();
		if (len < 0.00001f) { break; }
		if (len > radius) { push *= radius / len; }

		m_pos += push;
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
	// 1フレーム5.0f度まで
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
	UpdateFootstepByAnim(*m_spAnimator);
	m_spModel->CalcNodeMatrices();
}

const std::vector<float>* CharacterBase::GetStepPhases(const KdAnimationData* data) const
{
	if (!data) return nullptr;

	if (data->m_name.find("Sprint") != std::string::npos) return &FootstepPhase::Sprint();
	if (data->m_name.find("Walk") != std::string::npos) return &FootstepPhase::Walk();

	return nullptr;   // 足音を鳴らさないアニメ
}

void CharacterBase::UpdateFootstepByAnim(const KdAnimator& animator)
{
	const KdAnimationData* data = animator.GetAnimationData();
	const float phase = animator.GetPhase();

	// アニメ切り替え直後は記録だけ
	if (data != m_prevAnimData)
	{
		m_prevAnimData = data;
		m_prevPhase = phase;
		return;
	}

	const std::vector<float>* phases = GetStepPhases(data);
	if (!phases || !m_isGround || !m_isMoving)
	{
		m_prevPhase = phase;
		return;
	}

	const bool looped = (phase < m_prevPhase);

	for (float p : *phases)
	{
		bool crossed;
		if (!looped)
		{
			crossed = (m_prevPhase < p && p <= phase);
		}
		else
		{
			crossed = (p > m_prevPhase) || (p <= phase);
		}

		if (crossed)
		{
			m_stepCount++;
			const bool withSmoke = (m_stepCount % 2 == 0);   // 煙は2回に1回
			FootstepSystem::Instance().Play(m_groundType, m_pos, m_sceneScale, withSmoke);
		}
	}

	m_prevPhase = phase;
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
// NPC用：修復用途を持つアイテムがある
bool CharacterBase::HasRepairItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;
		ItemData data;
		if (data.Load(item.itemPath) && data.HasRepairUse()) return true;
	}
	return false;
}

// NPC用：いたずら用途を持つアイテムがある（危険物かは問わない）
bool CharacterBase::HasPrankItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;
		ItemData data;
		if (data.Load(item.itemPath) && data.HasPrankUse()) return true;
	}
	return false;
}

// 警察用：追跡・没収の対象（危険物）を持っている
bool CharacterBase::HasDangerousItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;
		ItemData data;
		if (data.Load(item.itemPath) && data.IsDangerous()) return true;
	}
	return false;
}

void CharacterBase::ConfiscateDangerousItems()
{
	const auto& items = m_inventory.GetItems();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].IsEmpty()) continue;
		ItemData data;
		if (data.Load(items[i].itemPath) && data.IsDangerous())
		{
			m_inventory.RemoveItem(i);
		}
	}

	OnItemsConfiscated();
}

void CharacterBase::OnArrested(const std::shared_ptr<Police>& police)
{
	m_isArrested = true;

	ShowEmote(EmoteIcon::IconType::Alert, 2.0f);
}

void CharacterBase::OnReleased()
{
	m_isArrested = false;
}

void CharacterBase::InitEmoteIcon()
{
	m_spEmoteIcon = std::make_shared<EmoteIcon>();
	m_spEmoteIcon->Init();
	m_spEmoteIcon->SetOwner(
		std::dynamic_pointer_cast<CharacterBase>(shared_from_this()));

	SceneManager::Instance().AddObject(m_spEmoteIcon);
}

void CharacterBase::ShowEmote(EmoteIcon::IconType type, float duration, bool force)
{
	if (m_spEmoteIcon) m_spEmoteIcon->SetIcon(type, duration, force);
}

void CharacterBase::HideEmote()
{
	if (m_spEmoteIcon) m_spEmoteIcon->SetIcon(EmoteIcon::IconType::None);
}