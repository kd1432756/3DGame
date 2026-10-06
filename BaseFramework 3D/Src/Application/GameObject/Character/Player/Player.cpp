#include "Player.h"

#include "../../../Scene/CameraManager/CameraManager.h"
#include "../../Camera/FPSCamera/FPSCamera.h"
#include "../../Camera/ArrestCamera/ArrestCamera.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/Item/ItemManager.h"

#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../../System/Interact/Interact.h"
#include "../../../System/Usable/Usable.h"

#include "../../../System/Skillcheck/SkillCheckFactory.h"

#include "../../../System/DayCycle/DayCycle.h"

#include "../../UI/FloatingTextUI/FloatingTextUI.h"
#include "../../UI/SlideTextUI/SlideTextUI.h"
#include "../../UI/ShakeTextUI/ShakeTextUI.h"

// いたずら、修復時にアクションを追加（スキルチェックかなにか）。
// それが終わってからカウントダウンを開始する。

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
		m_fpsCamera = std::dynamic_pointer_cast<FPSCamera>(camera);
	}

	InitEmoteIcon();

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Player::Update()
{
	if (m_isArrested)
	{
		UpdateArrested();
		return;
	}

	UpdateRotationByCamera();

	Move();

	//Jump();

	ApplyGravity();

	UpdateInventorySelect();

	UpdateFocusObject();

	TryInteract();

	StartUse();

	UpdateUse();

	TryDropItem();

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

void Player::DrawSprite()
{
	if (m_usePhase == UsePhase::SkillCheck && m_skillCheck)
	{
		m_skillCheck->Draw();
	}
}

void Player::UpdateArrested()
{
	auto arrestCam = std::dynamic_pointer_cast<ArrestCamera>(m_camera.lock());

	if (arrestCam)
	{
		if (m_isReleasing)
		{
			arrestCam->StartPushIn();
			m_isReleasing = false;
		}

		if (arrestCam->IsTransitionFinished())
		{
			m_fpsCamera->SetDegAng(arrestCam->GetDegAng());
			CameraManager::Instance().SetCamera(m_fpsCamera);
			SetCamera(m_fpsCamera);

			m_isArrested = false;
			m_isDrawEnabled = false;
		}
	}

	ApplyGravity();
	UpdateWorldMatrix();
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
	auto slideText = std::make_shared<SlideTextUI>();

	slideText->SetText(action.ownerMessage);

	slideText->SetPosition({ -500, 130 });

	SceneManager::Instance().AddObject(slideText);
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

void Player::OnItemsConfiscated()
{
	// 手持ちが没収されて選択が外れていたら、手持ちモデルを消す
	if (m_handItem && !m_inventory.GetCurrentItem())
	{
		m_handItem->Destroy();
		m_handItem.reset();
		m_inventory.SelectIndex(-1);
	}
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
	EquipItem(item ? item->itemPath : "");
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

	m_pos += _moveDir * _moveSpeed;
	
	ResolveCollision(_moveDir);
}

void Player::UpdateRotationByCamera()
{
	auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	m_rot.y = camera->GetDegAng().y + DirectX::XMConvertToDegrees(DirectX::XM_PI);
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

	if (m_isUsing)
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

void Player::UpdateInventorySelect()
{
	if (m_isUsing)
	{
		return;
	}

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (GetAsyncKeyState('1' + i) & 0x0001)
		{
			SelectInventory(i);
		}
	}
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
	m_useTimer = 0.0f;
	m_useTotal = 0.0f;

	const bool isRepair = usable->IsRepairAction(
		std::dynamic_pointer_cast<Player>(shared_from_this()));

	// スキルチェック開始(種類は抽選)
	m_skillResult = SkillResult::None;

	m_skillCheck = SkillCheckFactory::Create(
		isRepair,
		m_lastSkillType,
		DayCycle::Instance().GetTimeProgress());

	m_lastSkillType = static_cast<int>(m_skillCheck->GetType());

	// 難易度: 修復は易しめ、いたずらは標準
	m_skillCheck->Start(isRepair ? 0.8f : 1.0f);

	m_usePhase = UsePhase::SkillCheck;
}

void Player::UpdateUse()
{
	if (!m_isUsing)
	{
		return;
	}

	auto self = std::dynamic_pointer_cast<Player>(shared_from_this());

	// 対象が消えた / 他キャラに先に修復された等で使用不能になったら中断
	auto obj = m_useObject.lock();

	if (!obj || !obj->CanUse(self))
	{
		CancelUse();
		return;
	}

	// キャンセル操作(右クリック)。警察が来たら逃げる、の駆け引き用
	if (GetAsyncKeyState(VK_RBUTTON) & 0x0001)
	{
		CancelUse();
		return;
	}

	switch (m_usePhase)
	{
	case UsePhase::SkillCheck:
	{
		m_skillCheck->Update();

		if (m_skillCheck->IsFinished())
		{
			m_skillResult = m_skillCheck->GetResult();
			m_skillCheck.reset();

			std::string text;
			Math::Color color;
			switch (m_skillResult)
			{
			case SkillResult::Perfect:
				text = "Perfect!";
				color = { 1.0f, 0.9f, 0.2f, 1.0f };		// 黄
				break;

			case SkillResult::Good:
				text = "Good";
				color = { 0.4f, 0.9f, 0.4f, 1.0f };		// 緑
				break;

			case SkillResult::Miss:
				text = "Miss";
				color = { 0.9f, 0.3f, 0.3f, 1.0f };		// 赤
				break;

			default:
				return;
			}

			auto floatingText = std::make_shared<FloatingTextUI>();
			floatingText->SetText(text);
			floatingText->SetColor(color);
			floatingText->SetFont(FontID::Future16);
			floatingText->SetPosition({ 0, -25 });
			SceneManager::Instance().AddObject(floatingText);

			// 結果でカウント時間を補正(Perfect短縮 / Miss延長)
			m_useTotal = m_useTime * TimeRate(m_skillResult);
			m_useTimer = m_useTotal;
			m_usePhase = UsePhase::Counting;
		}
		break;
	}

	case UsePhase::Counting:
	{
		m_useTimer -= 1 / 60.0f;

		if (m_useTimer <= 0)
		{
			// Use() の中で GetSkillResult() を見てスコア倍率などを変える
			obj->Use(self);

			CancelUse();
		}
		break;
	}

	default:
		CancelUse();
		break;
	}
}

void Player::CancelUse()
{
	m_isUsing = false;
	m_usePhase = UsePhase::None;
	m_useTimer = 0.0f;
	m_useTotal = 0.0f;
	m_useObject.reset();
	m_skillCheck.reset();
}

void Player::SwapItem(
	const std::shared_ptr<Item>& newItem,
	int index)
{
	if (!newItem)
	{
		return;
	}

	if (index < 0 || index >= Inventory::MaxItem)
	{
		return;
	}

	auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	//============================================================
	// 交換対象（今そのスロットにあるアイテム）をワールドへ落とす
	//============================================================

	const std::string oldPath = m_inventory.GetItems()[index].itemPath;

	if (!oldPath.empty())
	{
		Math::Vector3 pos = camera->GetMatrix().Translation();
		Math::Vector3 dir = camera->GetMatrix().Backward();

		ItemManager::Instance().SpawnDropped(oldPath, pos, dir);
	}

	//============================================================
	// 手持ちモデルだった場合は先に破棄
	//============================================================

	if (m_handItem && m_inventory.GetSelectIndex() == index)
	{
		m_handItem->Destroy();
		m_handItem.reset();
	}

	//============================================================
	// 新アイテムをそのスロットに上書きして拾う
	//============================================================

	if (!newItem->Pickup(m_inventory, index))
	{
		return;
	}

	//============================================================
	// 選択中スロットなら手持ちを更新
	//============================================================

	if (m_inventory.GetSelectIndex() == index)
	{
		EquipItem(m_inventory.GetItems()[index].itemPath);
	}
}

void Player::TryDropItem()
{
	if (!(GetAsyncKeyState('G') & 0x0001))
	{
		return;
	}

	if (m_isUsing)
	{
		return;
	}

	if (!m_handItem)
	{
		return;
	}

	auto camera = m_camera.lock();

	std::string path = m_handItem->GetItemPath();
	Math::Vector3 pos = camera->GetMatrix().Translation();
	Math::Vector3 dir = camera->GetMatrix().Backward();

	ItemManager::Instance().SpawnDropped(path, pos, dir);

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

void Player::OnArrested(const std::shared_ptr<Police>& police)
{
	CharacterBase::OnArrested(police);

	// いたずら/インタラクト/スキルチェック中の状態をキャンセル
	CancelUse();

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

	{
		auto shakeText = std::make_shared<ShakeTextUI>();

		shakeText->SetText(
			"ARRESTED!!"
		);

		shakeText->SetPosition({ 0, 120 });

		SceneManager::Instance().AddObject(shakeText);
	}

	m_isDrawEnabled = true;

	SelectInventory(-1);

	auto arrestCamera = std::make_shared<ArrestCamera>(); // 先ほど作った専用カメラ
	arrestCamera->Init();
	arrestCamera->SetTarget(std::dynamic_pointer_cast<Player>(shared_from_this()));
	float height = 0.8f * m_sceneScale;
	arrestCamera->SetEyeHeight(height);
	arrestCamera->SetDegAng(m_fpsCamera->GetDegAng());

	// カメラマネージャーやPlayerのカメラを差し替え
	CameraManager::Instance().SetCamera(arrestCamera); // マネージャーの仕様に合わせる
	SetCamera(arrestCamera);
}

void Player::OnReleased()
{
	m_isReleasing = true;
}
