#include "Item.h"
#include "../../../../Scene/SceneManager.h"
#include "../../../Character/Player/Player.h"
#include "../../../../System/Inventory/Inventory.h"
#include "ItemSpawner.h"

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
	switch (m_mode)
	{
	case ItemMode::Spawned:

	
		break;
	case ItemMode::InInventory:

		break;
	case ItemMode::Dropped:
		UpdateGravity();
		UpdateFriction();
		CheckGround();
		ResolveCollision();
		UpdateWorldMatrix();

		UpdateDropTimer();
		break;
	default:
		break;
	}

}

void Item::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld);
}

void Item::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;

	if (m_mode == ItemMode::InInventory)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld);
}

bool Item::LoadItem(const std::string& path)
{
	if (!m_itemData.Load(path))
	{
		return false;
	}

	// StaticMeshObjectへモデルを反映
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

	//============================================================
	// 空きスロットがあれば通常通り拾う
	//============================================================

	if (inventory.HasEmptySlot())
	{
		Pickup(inventory);

		return;
	}

	//============================================================
	// 満杯 → 選択中スロットと交換
	//============================================================

	int selectIndex = inventory.GetSelectIndex();

	if (selectIndex < 0)
	{
		// 何も選択していない場合は交換不可（拾えない）
		return;
	}

	player->SwapItem(
		std::dynamic_pointer_cast<Item>(shared_from_this()),
		selectIndex
	);
}

bool Item::Pickup(Inventory& inventory)
{
	//============================================================
	// すでに取得済み
	//============================================================

	if (m_mode == ItemMode::InInventory)
	{
		return false;
	}

	//============================================================
	// ItemPathが無効
	//============================================================

	const std::string& itemPath = GetItemPath();

	if (itemPath.empty())
	{
		return false;
	}

	//============================================================
	// Inventoryに追加
	//============================================================

	if (!inventory.AddItem(itemPath))
	{
		return false;
	}

	//============================================================
	// SpawnedアイテムならSpawnerに再出現を要求
	//============================================================

	if (m_mode == ItemMode::Spawned)
	{
		auto spawner = m_spawner.lock();

		if (spawner)
		{
			spawner->StartRespawn();
		}
	}

	//============================================================
	// Itemをワールドから削除
	//============================================================

	Destroy();

	return true;
}

bool Item::PickupReplace(Inventory& inventory, int index)
{
	if (m_mode == ItemMode::InInventory)
	{
		return false;
	}

	const std::string& itemPath = GetItemPath();

	if (itemPath.empty())
	{
		return false;
	}

	if (index < 0 || index >= Inventory::MaxItem)
	{
		return false;
	}

	inventory.ReplaceItem(index, itemPath);

	if (m_mode == ItemMode::Spawned)
	{
		auto spawner = m_spawner.lock();

		if (spawner)
		{
			spawner->StartRespawn();
		}
	}

	Destroy();

	return true;
}

void Item::Drop(const Math::Vector3& pos, const Math::Vector3& dir)
{
	SetMode(ItemMode::Dropped);

	m_dropTimer = m_dropTime;

	SetPos(pos);

	Math::Vector3 _dir = dir;

	_dir.y -= 0.1f;
	_dir.Normalize();

	m_velocity = _dir * 0.2f * SceneManager::Instance().GetSceneScale();
	m_velocity.y += 0.05f * SceneManager::Instance().GetSceneScale();
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

void Item::CheckGround()
{
	m_isGround = false;
	// 当たる側
	// ====================
	// レイ（光線）判定
	// ====================
	KdCollider::RayInfo ray;
	// レイの発射位置を設定
	ray.m_pos = m_pos;
	// 段差の許容範囲を設定
	float enableStepHigh = 0.1f * SceneManager::Instance().GetSceneScale();
	ray.m_pos.y += enableStepHigh;
	// レイの発射方向を設定
	ray.m_dir = { 0,-1,0 };
	const float raySafetyMargin = 0.05f;    // 誤差吸収の余裕
	// レイの長さを設定
	ray.m_range = -m_gravity + -m_velocity.y + enableStepHigh + raySafetyMargin;
	// 当たり判定を行いたいタイプを設定
	ray.m_type = KdCollider::TypeGround;

	// デバッグ処理
	//m_pDebugWire->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range);

	// レイに当たったオブジェクト情報を格納するリスト
	std::list<KdCollider::CollisionResult>retRayList;
	// 全オブジェクトと当たり判定をする
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 全オブジェクトに対してレイ判定をする関数を呼び出す
		obj->Intersects(ray, &retRayList);
	}

	// レイリストから一番近いオブジェクトを探す
	float maxOverlap = 0;
	Math::Vector3 hitPos;
	bool hit = false;
	for (auto& ret : retRayList)
	{
		// レイを遮断し、オーバーした長さが一番長いものを探す
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
		// 当たっていたらその座標をプレイヤー座標にセットする
		// 地面から少しだけ浮かせて安定させる（見た目の誤差を吸収）
		const float groundOffset = 0.02f;
		m_pos = hitPos + Math::Vector3{ 0, groundOffset, 0 };
		m_gravity = 0.0f;
		m_velocity.y *= 0.5f;
		m_isGround = true;
	}
}

void Item::UpdateWorldMatrix()
{
	m_pos += m_velocity;
	m_pos.y += m_gravity;

	Math::Matrix rotMat =
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

void Item::ResolveCollision()
{
	KdCollider::SphereInfo sphere;
	// 球の中心座標を設定
	sphere.m_sphere.Center = m_pos;
	//sphere.m_sphere.Center.y += 0.1f * SceneManager::Instance().GetSceneScale();
	// 球の半径を設定
	sphere.m_sphere.Radius = 0.2f * SceneManager::Instance().GetSceneScale();
	// 当たり判定をしたいタイプを設定
	sphere.m_type = KdCollider::TypeBump;

	// デバッグ処理
	//m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius);

	// 球に当たったオブジェクトを格納するリスト
	std::list<KdCollider::CollisionResult> retSphereList;
	// 全オブジェクトと当たり判定をする
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 全オブジェクトに対して球判定をする関数を呼び出す
		obj->Intersects(sphere, &retSphereList);
	}

	// 球に当たったリストから一番近いオブジェクトを探す
	// ↓こいつ　レイ判定で宣言済みのものを使いまわす
	float maxOverlap = 0.0f;
	// ↓こいつ　レイ判定で宣言済みのものを使いまわす
	bool hit = false;
	Math::Vector3 hitDir;	// 当たった方向

	for (auto& ret : retSphereList)
	{
		// 球にめり込んだ長さが一番長いものを探す
		if (maxOverlap < ret.m_overlapDistance)
		{
			// 更新
			//maxOverlap = ret.m_overlapDistance;
			Math::Vector3 dir = ret.m_hitDir;
			dir.Normalize();
			m_pos += dir * ret.m_overlapDistance;
		}
	}
}
