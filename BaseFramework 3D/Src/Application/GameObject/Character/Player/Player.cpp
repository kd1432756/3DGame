#include "Player.h"

#include "../../../Scene/CameraManager/CameraManager.h"
#include "../../Camera/CameraBase.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../../System/Interact/Interact.h"
#include "../../../System/Usable/Usable.h"

#include "../../UI/FloatingTextUI/FloatingTextUI.h"

void Player::Init()
{
	CharacterBase::Init();

	SetName("Player");

	m_spModel->SetModelData(
		"Asset/Models/Character/Player/Player.gltf"
	);

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"Editor",
		m_spModel,
		KdCollider::TypeEditor);

	ScoreManager::Instance().RegisterCharacter(
		std::dynamic_pointer_cast<CharacterBase>(
			shared_from_this()));

	auto camera = CameraManager::Instance().GetCamera();

	if (camera)
	{
		camera->SetTarget(std::dynamic_pointer_cast<Player>(shared_from_this()));
		SetCamera(camera);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Player::Update()
{
	if (m_isArrested)
	{
		ApplyGravity();
		UpdateWorldMatrix();
		return;
	}

	Move();

	//Jump();

	ApplyGravity();

	char num = '1';
	for (int i = 0; i < 3; ++i)
	{
		if (GetAsyncKeyState(num) & 0x0001)
		{
			SelectInventory(num - '1');
		}
		num++;
	}

	//CheckBush();

	UpdateFocusObject();

	TryInteract();

	StartUse();

	UpdateUse();

	if (GetAsyncKeyState('G') & 0x0001)
	{
		DropItem();
	}

	UpdateWorldMatrix();
}

void Player::PostUpdate()
{
	//ResolveCollision();

	UpdateAnimation();

	UpdateHandItem();
}

void Player::DrawLit()
{
	if (!m_isDrawEnabled) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Player::OnPrankSuccess(int score, const std::string& message)
{
	AddScore(score);

	auto floatingText = std::make_shared<FloatingTextUI>();

	floatingText->SetText(
		message + "  + " + std::to_string(score)
	);

	floatingText->SetPosition({ 0, 100 });

	SceneManager::Instance().AddObject(floatingText);
}

void Player::OnPranked(const std::shared_ptr<InteractiveObject>& object, const InteractiveAction& action)
{
	auto floatingText = std::make_shared<FloatingTextUI>();

	floatingText->SetText(
		action.ownerMessage.c_str()
	);

	floatingText->SetPosition({ 0, 100 });

	SceneManager::Instance().AddObject(floatingText);
}

void Player::EquipItem(const std::string& itemPath)
{
	if (m_handItem)
	{
		m_handItem->Destroy();
		m_handItem.reset();
	}

	if (itemPath.empty())
	{
		return;
	}

	// 新しい手持ちアイテム生成
	m_handItem = std::make_shared<Item>();
	m_handItem->Init();
	m_handItem->SetMode(ItemMode::InInventory);
	m_handItem->LoadItem(itemPath);
	m_handItem->SetScale(m_handItem->GetItemData().GetHandScale() * m_sceneScale);
	SceneManager::Instance().AddObject(m_handItem);
}

void Player::SelectInventory(int index)
{
	// 同じ番号を押したら解除
	if (m_inventory.GetSelectIndex() == index && m_handItem)
	{
		m_inventory.SelectIndex(-1);
		EquipItem("");
		return;
	}

	m_inventory.SelectIndex(index);

	auto item = m_inventory.GetCurrentItem();

	if (item)
	{
		EquipItem(item->itemPath);
	}
	else
	{
		EquipItem("");
	}
}

void Player::Move()
{
	if (m_isUsing)
	{
		return;
	}

	Math::Vector3 _moveDir = {};
	float _moveSpeed = 0.15f * m_sceneScale;

	Math::Matrix camRotYMat = Math::Matrix::Identity;
	if (m_camera.expired() == false)
	{
		camRotYMat = m_camera.lock()->GetRotationYMatrix();
	}

	if (GetAsyncKeyState('W') & 0x8000) { _moveDir.z += 1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { _moveDir.z += -1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { _moveDir.x += -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { _moveDir.x += 1.0f; }

	if (_moveDir.LengthSquared() <= 0.0f)
	{
		return;
	}

	_moveDir = _moveDir.TransformNormal(_moveDir, camRotYMat);
	_moveDir.Normalize();

	RotateToMoveDirection(_moveDir);

	m_pos += _moveDir * _moveSpeed;
	
	CheckStep(_moveDir);

	ResolveCollision(_moveDir);
}

/*
void Player::Jump()
{
	if (m_isGround &&
		(GetAsyncKeyState(VK_SPACE) & 0x0001))
	{
		m_gravity = m_jumpPower;
		m_isGround = false;
	}

	m_gravity -= m_gravityPower;

	m_pos.y += m_gravity;

	CheckGround();
}*/

void Player::UpdateFocusObject()
{  
	auto oldObj = m_focusObject.lock();

	if (oldObj)
	{
		auto usable = std::dynamic_pointer_cast<Usable>(oldObj);

		if (usable)
		{
			usable->SetFocused(false);
		}
	}

	m_focusObject.reset();

	auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	KdCollider::RayInfo ray;

	ray.m_type = KdCollider::TypeInteract;
	ray.m_range = 3.0f * m_sceneScale;

	ray.m_pos = camera->GetMatrix().Translation();
	ray.m_dir = camera->GetMatrix().Backward();

	std::list<KdCollider::CollisionResult> retRayList;

	float maxOverlap = 0.0f;

	std::shared_ptr<KdGameObject> hitObj;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		if (obj.get() == this) continue;

		std::list<KdCollider::CollisionResult> retList;

		obj->Intersects(ray, &retList);

		for (auto& ret : retList)
		{
			if (ret.m_overlapDistance > maxOverlap)
			{
				maxOverlap = ret.m_overlapDistance;
				hitObj = obj;
			}
		}
	}

	if (hitObj)
	{
		m_focusObject = hitObj;

		auto usable = std::dynamic_pointer_cast<Usable>(hitObj);

		if (usable)
		{
			usable->SetFocused(true);
		}
	}
}

void Player::TryInteract()
{
	if (!(GetAsyncKeyState('E') & 0x0001))
	{
		return;
	}

	auto obj = m_focusObject.lock();

	if (!obj)
	{
		return;
	}

	auto interact =
		std::dynamic_pointer_cast<Interactable>(obj);

	if (!interact)
	{
		return;
	}

	interact->Interact(
		std::dynamic_pointer_cast<Player>(
			shared_from_this()));
}

void Player::StartUse()
{
	if (m_isUsing)
	{
		return;
	}

	if (!(GetAsyncKeyState(VK_LBUTTON) & 0x0001))
	{
		return;
	}

	auto obj = m_focusObject.lock();

	if (!obj)
	{
		return;
	}

	auto usable =
		std::dynamic_pointer_cast<Usable>(obj);

	if (!usable)
	{
		return;
	}

	if (!usable->CanUse(std::dynamic_pointer_cast<Player>(shared_from_this())))
	{
		return;
	}

	m_isUsing = true;
	m_useObject = usable;
	m_useTime = usable->GetUseTime(std::dynamic_pointer_cast<Player>(shared_from_this()));
	m_useTimer = m_useTime;

	//usable->Use(std::dynamic_pointer_cast<Player>(shared_from_this()));
}

void Player::UpdateUse()
{
	if (!m_isUsing)
	{
		return;
	}

	m_useTimer -= 1 / 60.0f;

	if (m_useTimer <= 0)
	{
		auto obj = m_useObject.lock();

		if (obj)
		{
			obj->Use(std::dynamic_pointer_cast<Player>(shared_from_this()));
		}

		m_isUsing = false;
	}
}

void Player::DropItem()
{
	if (!m_handItem)
	{
		return;
	}

	auto camera = m_camera.lock();

	auto dropItem = std::make_shared<Item>();
	dropItem->Init();
	dropItem->LoadItem(m_handItem->GetItemPath());
	dropItem->Drop(
		camera->GetMatrix().Translation(),
		camera->GetMatrix().Backward());
	dropItem->SetScale(m_handItem->GetItemData().GetWorldScale() * m_sceneScale);

	SceneManager::Instance().AddObject(dropItem);

	m_inventory.RemoveItem(m_inventory.GetSelectIndex());

	m_handItem->Destroy();
	m_handItem.reset();

/*	// 次のアイテムを装備
	auto next = m_inventory.GetCurrentItem();

	if (next)
	{
		EquipItem(next->itemPath);
	}*/

	m_inventory.SelectIndex(-1);
	EquipItem("");
}

void Player::UpdateHandItem()
{
	if (!m_handItem)
	{
		return;
	}

	auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	// カメラ基準の行列
	Math::Matrix camMat = camera->GetMatrix();

	// ItemDataを取得
	const ItemData& data = m_handItem->GetItemData();

	Math::Matrix scale =
		Math::Matrix::CreateScale(data.GetHandScale() * m_sceneScale);

	Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(data.GetHandRotation().y),
			DirectX::XMConvertToRadians(data.GetHandRotation().x),
			DirectX::XMConvertToRadians(data.GetHandRotation().z));

	Math::Matrix trans =
		Math::Matrix::CreateTranslation(data.GetHandOffset() * SceneManager::Instance().GetSceneScale());

	Math::Matrix world = scale * rot * trans * camMat;

	m_handItem->SetMatrix(world);
}

bool Player::CanHighlight()
{
	auto obj = m_focusObject.lock();

	if (!obj)
	{
		return false;
	}

	if (std::dynamic_pointer_cast<Interactable>(obj))
	{
		return true;
	}

	auto usable =
		std::dynamic_pointer_cast<Usable>(obj);

	if (usable)
	{
		return usable->CanUse(
			std::dynamic_pointer_cast<Player>(
				shared_from_this()));
	}

	return false;
}

void Player::CheckBush()
{
	m_isInBush = false;

	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Center.y += 1.0f;   // 胴体付近
	sphere.m_sphere.Radius = 1.0f;
	sphere.m_type = KdCollider::TypeBush;

	std::list<KdCollider::CollisionResult> retList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;
		if (obj.get() == this) continue;

		obj->Intersects(sphere, &retList);
	}

	m_isInBush = !retList.empty();
}

void Player::OnArrested(const std::shared_ptr<Police>& police)
{
	CharacterBase::OnArrested(police);

	// いたずら/インタラクト中の状態をキャンセル
	m_isUsing = false;
	m_useTimer = 0.0f;
	m_useObject.reset();

	// フォーカス解除(ハイライトが残らないように)
	auto oldObj = m_focusObject.lock();

	if (oldObj)
	{
		auto usable = std::dynamic_pointer_cast<Usable>(oldObj);

		if (usable)
		{
			usable->SetFocused(false);
		}
	}

	m_focusObject.reset();
}

bool Player::HasPrankItem() const
{
	for (const auto& item : m_inventory.GetItems())
	{
		if (item.IsEmpty())
		{
			continue;
		}

		ItemData itemData;

		if (!itemData.Load(item.itemPath))
		{
			continue;
		}

		// Repair以外(いたずら用)を1つでも持っていたら該当
		if (itemData.GetAction() != ItemAction::Repair &&
			itemData.GetAction() != ItemAction::None)
		{
			return true;
		}
	}

	return false;
}

void Player::ConfiscatePrankItems()
{
	const auto& items = m_inventory.GetItems();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].IsEmpty())
		{
			continue;
		}

		ItemData itemData;
		itemData.Load(items[i].itemPath);

		// いたずらアイテムのみ没収(Repairアイテムは残す)
		if (itemData.GetAction() != ItemAction::Repair &&
			itemData.GetAction() != ItemAction::None)
		{
			m_inventory.RemoveItem(i);
		}
	}

	if (m_handItem)
	{
		const ItemData& handData = m_handItem->GetItemData();

		if (handData.GetAction() != ItemAction::Repair &&
			handData.GetAction() != ItemAction::None)
		{
			m_handItem->Destroy();
			m_handItem.reset();
			m_inventory.SelectIndex(-1);
		}
	}
}