#include "Item.h"
#include "../../../../Scene/SceneManager.h"
#include "../../../Character/Player/Player.h"
#include "../../../../System/Inventory/Inventory.h"
#include "ItemSpawner.h"
#include "ItemManager.h"

void Item::Init()
{
	if (!m_modelPath.empty())
	{
		StaticMeshObject::ReloadModel();
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Item::Update()
{
	if (m_mode != ItemMode::Dropped)
	{
		return;
	}

	UpdateGravity();
	UpdateFriction();
	CheckGround();
	ResolveCollision();
	UpdateWorldMatrix();

	UpdateDropTimer();
}

void Item::DrawLit()
{
	if (!m_spModel) return;
	StaticMeshObject::DrawLit();

	// アウトライン描画
	KdShaderManager::Instance().m_StandardShader.SetEnableOutLineDraw(true);
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Item::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;
	if (m_mode == ItemMode::InInventory)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

bool Item::LoadItem(const std::string& path)
{
	if (!m_itemData.Load(path))
	{
		return false;
	}

	SetModelPath(m_itemData.GetModelPath());

	return true;
}

void Item::Interact(std::shared_ptr<Player> player)
{
	if (!player)
	{
		return;
	}

	Inventory& inventory = player->GetInventory();

	// 空きスロットがあれば通常通り拾う
	if (inventory.HasEmptySlot())
	{
		Pickup(inventory);

		return;
	}

	// 満杯 → 選択中スロットと交換
	int selectIndex = inventory.GetSelectIndex();

	if (selectIndex < 0)
	{
		// 何も選択していない場合は交換不可
		return;
	}

	player->SwapItem(std::dynamic_pointer_cast<Item>(shared_from_this()), selectIndex);
}

bool Item::Pickup(Inventory& inventory, int replaceIndex)
{
	if (IsExpired()) return false;

	if (m_mode == ItemMode::InInventory) return false;

	const std::string& path = GetItemPath();
	if (path.empty()) return false;

	if (replaceIndex < 0)
	{
		if (!inventory.AddItem(path)) return false;
	}
	else
	{
		if (replaceIndex >= Inventory::MaxItem) return false;
		inventory.ReplaceItem(replaceIndex, path);
	}

	ItemManager::Instance().OnPickedUp(*this);

	KdEffekseerManager::GetInstance().Play("itemPickup.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 2.0f);

	if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/SE/PickUp_Item.wav", m_pos))
	{
		se->SetVolume(0.2f);
		se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
	}

	Destroy();
	return true;
}

void Item::Drop(const Math::Vector3& pos, const Math::Vector3& dir)
{
	SetMode(ItemMode::Dropped);

	m_dropTimer = m_dropTime;

	SetPos(pos);

	m_rot.y = DirectX::XMConvertToDegrees(std::atan2f(dir.x, dir.z));
	SetRot(m_rot);

	Math::Vector3 _dir = dir;

	_dir.y -= 0.1f;
	_dir.Normalize();

	m_velocity = _dir * 0.05f * SceneManager::Instance().GetSceneScale();
	m_velocity.y += 0.05f * SceneManager::Instance().GetSceneScale();
	m_pos += m_velocity * 0.5f * SceneManager::Instance().GetSceneScale();
}

void Item::RegisterCollider()
{
	if (m_mode == ItemMode::InInventory) return;

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Item",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeInteract);
}

void Item::UpdateGravity()
{
	if (!m_isGround)
	{
		m_gravity -= m_gravityPower * SceneManager::Instance().GetSceneScale();
	}
}

void Item::UpdateFriction()
{
	if (!m_isGround)
	{
		return;
	}

	m_velocity *= 0.85f;

	if (m_velocity.LengthSquared() < 0.00001f)
	{
		m_velocity = Math::Vector3::Zero;
	}
}

void Item::UpdateWorldMatrix()
{
	m_pos += m_velocity;
	m_pos.y += m_gravity;

	const Math::Matrix rotMat =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_rot.y),
			DirectX::XMConvertToRadians(m_rot.x),
			DirectX::XMConvertToRadians(m_rot.z));

	m_mWorld =
		Math::Matrix::CreateScale(m_scale) *
		rotMat *
		Math::Matrix::CreateTranslation(m_pos);
}

void Item::UpdateDropTimer()
{
	m_dropTimer -= 1.0f / 60.0f;

	if (m_dropTimer <= 0.0f)
	{
		Destroy();
		return;
	}
}

void Item::CheckGround()
{
	m_isGround = false;

	// レイ
	KdCollider::RayInfo ray;
	ray.m_pos = m_pos;
	float enableStepHigh = 0.1f * SceneManager::Instance().GetSceneScale();
	ray.m_pos.y += enableStepHigh;
	ray.m_dir = { 0,-1,0 };
	const float raySafetyMargin = 0.05f;    // 誤差
	ray.m_range = -m_gravity + -m_velocity.y + enableStepHigh + raySafetyMargin;
	ray.m_type = KdCollider::TypeGround;

	std::list<KdCollider::CollisionResult>retRayList;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		obj->Intersects(ray, &retRayList);
	}

	float maxOverlap = 0;
	Math::Vector3 hitPos;
	bool hit = false;
	for (auto& ret : retRayList)
	{
		if (maxOverlap < ret.m_overlapDistance)
		{
			// 更新
			maxOverlap = ret.m_overlapDistance;
			hitPos = ret.m_hitPos;
			hit = true;
		}
	}
	if (hit)
	{
		// 地面から少しだけ浮かせる
		const float groundOffset = 0.02f;
		m_pos = hitPos + Math::Vector3{ 0, groundOffset, 0 };
		m_gravity = 0.0f;
		m_velocity.y *= 0.5f;
		m_isGround = true;
	}
}

void Item::ResolveCollision()
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Radius = 0.2f * SceneManager::Instance().GetSceneScale();
	sphere.m_type = KdCollider::TypeBump;

	std::list<KdCollider::CollisionResult> retSphereList;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		obj->Intersects(sphere, &retSphereList);
	}

	float maxOverlap = 0.0f;
	bool hit = false;
	Math::Vector3 hitDir;	

	for (auto& ret : retSphereList)
	{
		if (maxOverlap < ret.m_overlapDistance)
		{
			// 更新
			Math::Vector3 dir = ret.m_hitDir;
			dir.Normalize();
			m_pos += dir * ret.m_overlapDistance;
		}
	}
}
