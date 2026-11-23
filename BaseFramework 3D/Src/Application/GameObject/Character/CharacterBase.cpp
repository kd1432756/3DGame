#include "CharacterBase.h"

#include "../../Scene/SceneManager.h"

#include "../../System/Score/ScoreManager.h"

#include "../../System/FootStep/FootStep.h"

#include "../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

#include "../UI/EmoteIcon/EmoteIcon.h"

void CharacterBase::Init()
{
	m_updateGroup = UpdateGroup::World;

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

	const float snapDown = wasGround ? stepHeight : std::abs(m_gravity);
	ray.m_range = stepHeight + snapDown;

	bool hit = false;
	float best = 0.0f;
	Math::Vector3 groundPos;
	std::shared_ptr<KdGameObject> groundObj;
	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj || obj.get() == this) continue;

		std::list<KdCollider::CollisionResult> objHits;
		if (!obj->Intersects(ray, &objHits)) continue;

		for (const auto& r : objHits)
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
		m_groundType = Ground::GroundType::None;
		return;
	}

	// 地面素材を更新
	m_groundType = Ground::GroundType::None;
	if (const auto ground = std::dynamic_pointer_cast<Ground>(groundObj))
	{
		m_groundType = ground->GetGroundType();
	}

	const float newY = groundPos.y + footOffset;

	m_pos.y = newY;
	m_isGround = true;
	m_lastGroundPos = m_pos;
	m_hasLastGround = true;
	m_gravity = 0.0f;
}

void CharacterBase::SlideAlongWalls(Math::Vector3& move, float radius) const
{
	std::vector<WallHit> walls;
	CollectWallHits(radius, walls);

	for (int pass = 0; pass < 2; ++pass)
	{
		for (const auto& w : walls)
		{
			const float into = move.Dot(w.normal);
			if (into < 0.0f)
			{
				move -= w.normal * into;
			}
		}
	}
}

void CharacterBase::CollectWallHits(float radius, std::vector<WallHit>& out) const
{
	out.clear();

	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Radius = radius;
	sphere.m_type = KdCollider::TypeBump;

	std::list<KdCollider::CollisionResult> collisionList;

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj || obj.get() == this) { continue; }
		if (const auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (!mesh->IsCollisionEnabled()) { continue; }
			if (mesh->IsGround()) { continue; }
		}
		obj->Intersects(sphere, &collisionList);
	}

	for (const auto& c : collisionList)
	{
		if (c.m_overlapDistance <= 0.0f) { continue; }

		Math::Vector3 n = c.m_hitNDir;
		if (n.LengthSquared() <= 0.000001f) { continue; }
		n.Normalize();

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
	const float radius = (WallRadius()) - 0.002f * m_sceneScale;
	const float skin = 0.001f * m_sceneScale;
	const float maxPush = radius * 0.5f;   // 1回の最大押し出し

	std::vector<WallHit> walls;

	for (int i = 0; i < 8; ++i)
	{
		CollectWallHits(radius, walls);
		if (walls.empty()) { break; }

		// 最も深くめり込んでいる壁を1つだけ解消
		const WallHit* deepest = &walls[0];
		for (const auto& w : walls)
		{
			if (w.overlap > deepest->overlap) { deepest = &w; }
		}

		float d = deepest->overlap + skin;
		if (d > maxPush) { d = maxPush; }

		m_pos += deepest->normal * d;
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

	if (m_hasLastGround &&
		m_pos.y < m_lastGroundPos.y - m_fallLimit * m_sceneScale)
	{
		m_pos = m_lastGroundPos;
		m_gravity = 0.0f;
	}
}

void CharacterBase::RotateToMoveDirection(const Math::Vector3& dir)
{
	Math::Vector3 toDir = dir;
	toDir.y = 0.0f;

	if (toDir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	toDir.Normalize();

	// =========================================
	// 現在の向き
	// =========================================

	const Math::Vector3 nowDir = GetFacingDir();

	// =========================================
	// 内積から角度を求める
	// =========================================

	float dot = nowDir.Dot(toDir);

	dot = std::clamp(dot, -1.0f, 1.0f);

	float angle = DirectX::XMConvertToDegrees(acosf(dot));

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

	const Math::Vector3 cross = nowDir.Cross(toDir);

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

	for (const float p : *phases)
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

// 警察用：追跡・没収の対象（危険物）を持っている
bool CharacterBase::HasDangerousItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;
		const ItemData* d = ItemData::Get(item.itemPath);
		if (d && d->IsDangerous()) return true;
	}
	return false;
}

int CharacterBase::CountDangerousItems() const
{
	int n = 0;
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty()) continue;
		const ItemData* data = ItemData::Get(item.itemPath);
		if (data && data->IsDangerous()) ++n;
	}
	return n;
}

int CharacterBase::ConfiscateDangerousItems()
{
	int removed = 0;
	const auto& items = m_inventory.GetItems();
	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].IsEmpty()) continue;
		const ItemData* data = ItemData::Get(items[i].itemPath);
		if (data && data->IsDangerous())
		{
			m_inventory.RemoveItem(i);
			++removed;
		}
	}
	OnItemsConfiscated();
	return removed;
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
	m_spEmoteIcon->SetOwner(std::dynamic_pointer_cast<CharacterBase>(shared_from_this()));

	SceneManager::Instance().AddObject(m_spEmoteIcon);
}

void CharacterBase::ShowEmote(EmoteIcon::IconType type, float duration, bool force) const
{
	if (!m_spEmoteIcon) return;
	if (!m_spEmoteIcon->SetIcon(type, duration, force)) return;

	if (const auto se = KdAudioManager::Instance().Play3D("Asset/Sound/SE/Icon_Pop.wav", m_pos))
	{
		se->SetVolume(0.2f);
		se->SetPitch(0.3f);
		se->SetCurveDistanceScaler(m_sceneScale * 1.2f);
	}
}

void CharacterBase::HideEmote() const
{
	if (m_spEmoteIcon) m_spEmoteIcon->SetIcon(EmoteIcon::IconType::None);
}