#include "Player.h"

#include "../../../Scene/CameraManager/CameraManager.h"
#include "../../Camera/FPSCamera/FPSCamera.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/Item/ItemManager.h"

#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../../System/Interact/Interact.h"
#include "../../../System/Usable/Usable.h"

#include "../../../System/Location/LocationPointManager.h"

#include "../../../System/Skillcheck/SkillCheckFactory.h"

#include "../../../System/DayCycle/DayCycle.h"

#include "../../UI/FloatingTextUI/FloatingTextUI.h"
#include "../../UI/SlideTextUI/SlideTextUI.h"
#include "../../UI/ShakeTextUI/ShakeTextUI.h"
#include "../../UI/DamageDirectionUI/DamageDirectionUI.h"

#include "../../../System/FootStep/FootStep.h"

#include "../Enemy/Police/Police.h"

#include "../../UI/NotificationUI/NotificationManager.h"

void Player::Init()
{
	CharacterBase::Init();

	SetName("Player");

	SetDisplayName("You");

	m_spModel->SetModelData("Asset/Models/Character/Player/Player.gltf");

	m_spAnimator = std::make_shared<KdAnimator>();
	m_currentAnimName = "Armature|Idle";
	m_spAnimator->SetAnimation(m_spModel->GetAnimation(m_currentAnimName));

	UpdateAnimation();

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"Editor",
		m_spModel,
		KdCollider::TypeEditor);

	InitEmoteIcon();

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Player::Update()
{
	FootstepSystem::Instance().SetListenerPos(m_pos);

	UpdateRepairBonus();
	UpdateNeglect();

	if (m_isArrested)
	{
		UpdateArrested();
		return;
	}

	if (m_isHinderCamReturning)
	{
		UpdateHinderCamera();
		UpdateLocked();
		return;
	}

	if (!m_isHindering)
	{
		UpdateRotationByCamera();
	}

	UpdateHinder();

	if (m_isHinderCamReturning)
	{
		UpdateLocked();
		return;
	}

	Move();
	ApplyGravity();

	if (!m_isRestrained)
	{
		UpdateInventorySelect();
		UpdateFocusObject();
		TryInteract();
		StartUse();
		UpdateUse();
		TryDropItem();
	}

	UpdateReport();

	UpdateWorldMatrix();
}

void Player::PostUpdate()
{
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

void Player::UpdateAnimation()
{
	if (m_isMoving)
	{
		PlayAnimation("Armature|Sprint");
	}
	else
	{
		PlayAnimation("Armature|Idle");
	}

	CharacterBase::UpdateAnimation();
}

void Player::UpdateLocked()
{
	m_isMoving = false;
	ApplyGravity();
	UpdateWorldMatrix();
}

void Player::UpdateArrested()
{
	m_isMoving = false;

	const auto arrestCam = std::dynamic_pointer_cast<ArrestCamera>(m_camera.lock());

	if (arrestCam)
	{
		if (m_isReleasing)
		{
			arrestCam->StartPushIn();
			m_isReleasing = false;
		}

		if (arrestCam->IsTransitionFinished())
		{
			RestoreFpsCamera(arrestCam);

			m_isArrested = false;
		}
	}
	else
	{
		RestoreFpsCamera(arrestCam);

		m_isArrested = false;
	}

	ApplyGravity();
	UpdateWorldMatrix();
}

void Player::SwitchToArrestCamera()
{
	if (!m_fpsCamera) return;

	const auto cam = std::make_shared<ArrestCamera>();
	cam->Init();
	cam->SetTarget(std::dynamic_pointer_cast<Player>(shared_from_this()));
	cam->SetEyeHeight(0.8f * m_sceneScale);
	cam->SetDegAng(m_fpsCamera->GetDegAng());

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		if (obj.get() == this) continue;

		// 地面登録
		cam->RegistHitObject(obj);
	}

	CameraManager::Instance().SetCamera(cam);
	SetCamera(cam);

	m_isDrawEnabled = true;
}

void Player::RestoreFpsCamera(const std::shared_ptr<ArrestCamera>& arrestCam)
{
	if (!m_fpsCamera || !arrestCam) return;

	m_fpsCamera->SetDegAng(arrestCam->GetDegAng());
	CameraManager::Instance().SetCamera(m_fpsCamera);
	SetCamera(m_fpsCamera);

	m_isDrawEnabled = false;
}

void Player::UpdateNeglect()
{
	for (auto it = m_neglected.begin(); it != m_neglected.end();)
	{
		const auto obj = it->obj.lock();

		// 修復されたら消す
		if (!obj || !obj->IsDamaged())
		{
			if (obj)
			{
				if (const auto ui = m_damageUI.lock()) ui->RemoveTarget(obj);
			}
			it = m_neglected.erase(it);
			continue;
		}

		it->elapsed += 1 / 60.0f;

		if (it->elapsed >= kNeglectGrace)
		{
			it->tickTimer += 1 / 60.0f;

			if (it->tickTimer >= kNeglectInterval)
			{
				it->tickTimer = 0.0f;

				// 0未満にはしない
				const int penalty = std::min(kNeglectPenalty, GetScore());
				if (penalty > 0)
				{
					AddScore(-penalty);

					SpawnFloatingText(
						"-" + std::to_string(penalty),
						{ 500, 270 },
						{ 0.9f, 0.3f, 0.3f, 1.0f }
					);
				}
			}
		}

		++it;
	}
}

void Player::UpdateReport()
{
	if (m_reportTimer > 0.0f)
	{
		m_reportTimer -= 1 / 60.0f;
		return;
	}

	bool pressed = GetAsyncKeyState('R') & 0x8000;

	if (!pressed)
	{
		m_isReportKeyPressed = false;
	}
	else if (!m_isReportKeyPressed)
	{
		m_isReportKeyPressed = true;
		if (TryReport())
		{
			m_reportTimer = kReportCooldown;
		}
	}
}

bool Player::IsVisibleFromCamera(
	const Math::Vector3& targetPos,
	const KdGameObject* ignore) const
{
	const auto camera = m_camera.lock();

	if (!camera)
	{
		return false;
	}

	//============================================================
	// ① 画面内か
	//============================================================

	Math::Vector3 screen;

	// カメラの画面内か
	if (!camera->ConvertWorldToScreenDetail(targetPos, screen))
	{
		return false;	// カメラの後ろ
	}

	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// 画面の少し内側で判定
	constexpr float kScreenMargin = 0.7f;

	if (std::abs(screen.x) > vp.width * 0.5f * kScreenMargin ||
		std::abs(screen.y) > vp.height * 0.5f * kScreenMargin)
	{
		return false;
	}

	//============================================================
	// ② Rayで遮られていないか
	//============================================================

	const Math::Vector3 camPos = camera->GetMatrix().Translation();

	Math::Vector3 dir = targetPos - camPos;

	const float distance = dir.Length();

	if (distance <= 0.0001f)
	{
		return true;
	}

	dir /= distance;

	KdCollider::RayInfo ray;

	ray.m_type = KdCollider::TypeGround | KdCollider::TypeBump;
	ray.m_pos = camPos;
	ray.m_dir = dir;

	ray.m_range = std::max(distance - 0.05f * m_sceneScale, 0.001f);

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		if (obj.get() == this) continue;
		// 判定対象はのぞく
		if (obj.get() == ignore) continue;

		std::list<KdCollider::CollisionResult> hits;

		if (obj->Intersects(ray, &hits))
		{
			return false;	// 遮蔽物あり
		}
	}

	return true;
}

bool Player::FindReportPos(Math::Vector3& outPos) const
{
	const auto camera = m_camera.lock();

	if (!camera)
	{
		return false;
	}

	const Math::Vector3 camPos = camera->GetMatrix().Translation();

	// ① 見えている「いたずら中のNPC」のうち、一番近いもの
	float nearestSq = std::numeric_limits<float>::max();
	bool found = false;

	for (const auto& ch : SceneManager::Instance().FindObjects<CharacterBase>())
	{
		if (!ch || ch.get() == this) continue;
		if (ch->IsArrested() || !ch->IsPranking()) continue;

		// レイで確認
		if (!IsVisibleFromCamera(ch->GetPos(), ch.get())) continue;

		// 向かっている場合、プレイヤーからの距離
		if (!ch->IsDamaging())
		{
			Math::Vector3 diff = ch->GetPos() - m_pos;
			diff.y = 0.0f;

			const float range = kReportPrankRange * m_sceneScale;
			if (diff.LengthSquared() > range * range) continue;
		}

		const float sq = (ch->GetPos() - camPos).LengthSquared();

		if (sq < nearestSq)
		{
			nearestSq = sq;
			outPos = ch->GetDamagePos();		// 通報地点はいたずらしてる現場
			found = true;
		}
	}

	if (found)
	{
		return true;
	}

	// ② 見えている「最近NPCにいたずらされた物」のうち、一番新しいもの
	float best = kReportWindowSeconds;

	for (const auto& obj : SceneManager::Instance().FindObjects<InteractiveObject>())
	{
		if (obj->GetDamageSource() != DamageSource::NPC) continue;

		const float seconds = obj->GetSecondsSinceDamaged();

		if (seconds > best) continue;

		// 足元ではなく少し上
		const Math::Vector3 target =
			obj->GetPos() + Math::Vector3{ 0.0f, 0.3f * m_sceneScale, 0.0f };

		if (!IsVisibleFromCamera(target, obj.get())) continue;

		best = seconds;
		outPos = obj->GetPos();
		found = true;
	}

	return found;
}

bool Player::TryReport() const
{
	Math::Vector3 reportPos;

	if (!FindReportPos(reportPos))
	{
		SpawnFloatingText(
			"No target in sight",
			{ 530, -210 },
			{ 0.8f, 0.8f, 0.8f, 1.0f },
			FontID::Future16
		);
		return false;
	}

	// 通報地点から最も近く、出動できる警察官を探す
	std::shared_ptr<Police> nearest;
	float nearestSq = std::numeric_limits<float>::max();

	for (const auto& police : SceneManager::Instance().FindObjects<Police>())
	{
		if (!police->CanDispatch()) continue;

		Math::Vector3 diff = police->GetPos() - reportPos;
		diff.y = 0.0f;

		const float dSq = diff.LengthSquared();

		if (dSq < nearestSq)
		{
			nearestSq = dSq;
			nearest = police;
		}
	}

	if (!nearest)
	{
		SpawnFloatingText(
			"No police available",
			{ 530, -210 },
			{ 0.8f, 0.8f, 0.8f, 1.0f },
			FontID::Future16
		);
		return false;
	}

	nearest->Dispatch(reportPos);

	SpawnFloatingText(
		"Reported!",
		{ 530, -210 },
		{ 0.9f, 0.35f, 0.0f, 1.0f },
		FontID::Future24
	);

	if (const auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Call.wav"))
	{
		se->SetVolume(0.1f);
	}

	return true;
}

void Player::UpdateHinder()
{
	if (m_hinderCooldownTimer > 0.0f)
	{
		m_hinderCooldownTimer = std::max(0.0f, m_hinderCooldownTimer - 1 / 60.0f);
	}

	if (!m_isHindering)
	{
		if (m_hinderCooldownTimer > 0.0f)
		{
			return;
		}

		if ((GetAsyncKeyState('F') & 0x0001) && !m_isArrested)
		{
			TryBeginHinder();
		}

		return;
	}

	const auto target = m_wpHinderTarget.lock();

	// 対象が消えた／逮捕された／いたずらをやめた時終了
	if (!target || target->IsArrested() || !target->IsDamaging())
	{
		EndHinder(true, false);
		return;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x0001)
	{
		// 満タンのときは回復も減衰カウントも進めない
		if (m_hinderRemaining < kHinderBaseTime)
		{
			// 連打回数に応じて回復量を減衰させる
			const float extend = kHinderMashExtend * std::pow(kHinderMashDecay, static_cast<float>(m_hinderMashCount));

			m_hinderRemaining = std::min(m_hinderRemaining + extend, kHinderBaseTime);
			m_hinderMashCount++;
		}
	}

	m_hinderRemaining -= 1 / 60.0f;

	if (m_hinderRemaining <= 0.0f)
	{
		EndHinder(true, true);
	}
}

bool Player::TryBeginHinder()
{
	if (m_isHinderCamReturning) return false;

	const auto nearest = FindHinderTarget();
	if (!nearest) return false;

	// 自分のいたずら／修復中ならキャンセル
	CancelUse();

	m_wpHinderTarget = nearest;
	m_hinderRemaining = kHinderBaseTime;
	m_hinderMashCount = 0;

	m_isHindering = true;
	m_isRestrained = true;
	nearest->SetRestrained(true);

	BeginHinderCamera();

	return true;
}

std::shared_ptr<CharacterBase> Player::FindHinderTarget() const
{
	const float range = kHinderRange * m_sceneScale;

	std::shared_ptr<CharacterBase> nearest;
	float nearestSq = range * range;

	// 一定範囲内にいる最も近いいたずら中のNPC
	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		const auto ch = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!ch || ch.get() == this) continue;
		if (std::dynamic_pointer_cast<Police>(ch)) continue;
		if (ch->IsArrested() || !ch->IsDamaging()) continue;

		Math::Vector3 diff = ch->GetPos() - m_pos;
		diff.y = 0.0f;
		const float dSq = diff.LengthSquared();

		if (dSq <= nearestSq)
		{
			nearestSq = dSq;
			nearest = ch;
		}
	}
	return nearest;
}

void Player::EndHinder(bool restoreCamera, bool applyCooldown)
{
	if (const auto target = m_wpHinderTarget.lock())
	{
		target->SetRestrained(false);
	}

	m_wpHinderTarget.reset();
	m_hinderRemaining = 0.0f;
	m_hinderMashCount = 0;

	if (applyCooldown)
	{
		m_hinderCooldownTimer = kHinderCooldown;
	}

	m_isHindering = false;
	m_isRestrained = false;

	if (restoreCamera)
	{
		if (const auto cam = std::dynamic_pointer_cast<ArrestCamera>(m_camera.lock()))
		{
			cam->StartPushIn();
			m_isHinderCamReturning = true;
		}
	}
}

void Player::BeginHinderCamera()
{
	SelectInventory(-1);

	ClearFocus();

	SwitchToArrestCamera();
}

void Player::UpdateHinderCamera()
{
	const auto arrestCam = std::dynamic_pointer_cast<ArrestCamera>(m_camera.lock());

	if (!arrestCam)
	{
		m_isHinderCamReturning = false;
		return;
	}

	if (!arrestCam->IsTransitionFinished()) return;

	RestoreFpsCamera(arrestCam);

	m_isHinderCamReturning = false;
}

void Player::OnRepairSuccess(int score, const std::string& message)
{
	AddScore(score);

	SpawnFloatingText(
		message + "  + " + std::to_string(score),
		{ 0, 100 },
		{ 1.0f, 0.78f, 0.10f, 1.0f }
	);
}

void Player::OnObjectDamaged(const std::shared_ptr<InteractiveObject>& obj, const InteractiveAction& action)
{
	NotificationManager::Instance().Push(action.ownerMessage);

	if (const auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Blip_Down.wav"))
	{
		se->SetVolume(0.1f);
	}

	AddDamageTarget(obj);
}

void Player::AddDamageTarget(const std::shared_ptr<InteractiveObject>& obj)
{
	for (const auto& e : m_neglected)
	{
		if (e.obj.lock() == obj) return;
	}

	m_neglected.push_back({ obj, 0.0f, 0.0f });

	if (const auto ui = m_damageUI.lock())
	{
		ui->AddTarget(obj);
	}
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
	if (m_inventory.GetSelectIndex() == index && m_handItem)
	{
		ClearHandItem();
		return;
	}

	m_inventory.SelectIndex(index);

	const auto item = m_inventory.GetCurrentItem();
	EquipItem(item ? item->itemPath : "");
}

void Player::Move()
{
	m_isMoving = false;

	if (m_isUsing || m_isRestrained)
	{
		return;
	}

	Math::Vector3 moveDir = {};
	const float moveSpeed = kMoveSpeed * m_sceneScale;

	Math::Matrix camRotYMat = Math::Matrix::Identity;
	if (m_camera.expired() == false)
	{
		camRotYMat = m_camera.lock()->GetRotationYMatrix();
	}

	if (GetAsyncKeyState('W') & 0x8000) { moveDir.z += 1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { moveDir.z += -1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { moveDir.x += -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { moveDir.x += 1.0f; }

	if (moveDir.LengthSquared() <= 0.0f)
	{
		return;
	}

	moveDir = moveDir.TransformNormal(moveDir, camRotYMat);
	moveDir.Normalize();

	Math::Vector3 move = moveDir * moveSpeed;

	const float radius = WallRadius();

	std::vector<WallHit> walls;
	CollectWallHits(radius, walls);

	// 壁に向かう成分を除去
	SlideAlongWalls(move, radius);

	const Math::Vector3 before = m_pos;
	m_pos += move;
	ResolveCollision();   // 念のための貫通解消
	m_isMoving = (m_pos - before).LengthSquared() > 0.00000001f;
}

void Player::UpdateRotationByCamera()
{
	const auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	m_rot.y = camera->GetDegAng().y + DirectX::XMConvertToDegrees(DirectX::XM_PI);
}

void Player::UpdateFocusObject()
{
	ClearFocus();

	const auto camera = m_camera.lock();

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

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		if (obj.get() == this) continue;

		std::list<KdCollider::CollisionResult> retList;

		obj->Intersects(ray, &retList);

		for (const auto& ret : retList)
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

		const auto usable = std::dynamic_pointer_cast<Usable>(hitObj);

		if (usable)
		{
			usable->SetFocused(true);
		}
	}
}

void Player::ClearFocus()
{
	if (const auto old = m_focusObject.lock())
	{
		if (const auto usable = std::dynamic_pointer_cast<Usable>(old))
		{
			usable->SetFocused(false);
		}
	}
	m_focusObject.reset();
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

	if (TryDeliver()) return;

	const auto obj = m_focusObject.lock();

	if (!obj)
	{
		return;
	}

	const auto interact = std::dynamic_pointer_cast<Interactable>(obj);

	if (!interact)
	{
		return;
	}

	interact->Interact(std::dynamic_pointer_cast<Player>(shared_from_this()));
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

	const auto obj = m_focusObject.lock();

	if (!obj)
	{
		return;
	}

	const auto usable = std::dynamic_pointer_cast<Usable>(obj);

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

	// スキルチェック開始
	m_skillResult = SkillResult::None;

	m_skillCheck = SkillCheckFactory::Create(m_lastSkillType, DayCycle::Instance().GetTimeProgress());

	m_lastSkillType = static_cast<int>(m_skillCheck->GetType());

	// 難易度
	m_skillCheck->Start(1.0f + DayCycle::Instance().GetTimeProgress() * 0.5f);

	m_usePhase = UsePhase::SkillCheck;
}

void Player::UpdateUse()
{
	if (!m_isUsing)
	{
		return;
	}

	const auto obj = m_useObject.lock();

	if (!obj || !obj->CanUse(std::dynamic_pointer_cast<Player>(shared_from_this())))
	{
		CancelUse();
		return;
	}

	// キャンセル
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

			SpawnFloatingText(
				text,
				{ 0, -25 },
				color,
				FontID::Future16
			);

			// スキルチェックの結果に応じて使用時間を決定
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
			const int usedIndex = m_inventory.GetSelectIndex();

			obj->Use(std::dynamic_pointer_cast<Player>(shared_from_this()));

			if (usedIndex >= 0)
			{
				ConsumeItem(usedIndex);
			}

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
	GetAsyncKeyState(VK_LBUTTON);
}

void Player::SwapItem(const std::shared_ptr<Item>& newItem, int index)
{
	if (!newItem)
	{
		return;
	}

	if (index < 0 || index >= Inventory::MaxItem)
	{
		return;
	}

	const auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	const std::string oldPath = m_inventory.GetItems()[index].itemPath;

	// 拾う
	if (!newItem->Pickup(m_inventory, index)) return;

	// 古い物を落とす
	if (!oldPath.empty())
	{
			ItemManager::Instance().SpawnDropped(
			oldPath,
			camera->GetMatrix().Translation(),
			camera->GetMatrix().Backward());
	}

	// 選択中スロットなら手持ちを作る
	if (m_inventory.GetSelectIndex() == index)
	{
		EquipItem(m_inventory.GetItems()[index].itemPath);
	}
}

void Player::ClearHandItem()
{
	m_inventory.SelectIndex(-1);
	EquipItem("");
}

void Player::ConsumeItem(int index)
{
	m_inventory.RemoveItem(index);

	if (m_handItem)
	{
		m_handItem->Destroy();
		m_handItem.reset();
	}

	ClearHandItem();
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

	const auto camera = m_camera.lock();

	const std::string path = m_handItem->GetItemPath();
	const Math::Vector3 pos = camera->GetMatrix().Translation() + Math::Vector3{ 0, -GetScale().y * 0.35f, 0 };
	const Math::Vector3 dir = camera->GetMatrix().Backward();

	ItemManager::Instance().SpawnDropped(path, pos, dir);

	m_inventory.RemoveItem(m_inventory.GetSelectIndex());

	m_handItem->Destroy();
	m_handItem.reset();

	ClearHandItem();
}

void Player::UpdateHandItem() const
{
	if (!m_handItem)
	{
		return;
	}

	const auto camera = m_camera.lock();

	if (!camera)
	{
		return;
	}

	// カメラ基準の行列
	const Math::Matrix camMat = camera->GetMatrix();

	const ItemData& data = m_handItem->GetItemData();

	const Math::Matrix scale = Math::Matrix::CreateScale(data.GetHandScale() * m_sceneScale);

	const Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(data.GetHandRotation().y),
			DirectX::XMConvertToRadians(data.GetHandRotation().x),
			DirectX::XMConvertToRadians(data.GetHandRotation().z));

	const Math::Matrix trans = Math::Matrix::CreateTranslation(data.GetHandOffset() * SceneManager::Instance().GetSceneScale());

	const Math::Matrix world = scale * rot * trans * camMat;

	m_handItem->SetMatrix(world);
}

void Player::PlayAnimation(const std::string& animName)
{
	if (m_currentAnimName == animName)
	{
		return;
	}

	m_currentAnimName = animName;

	m_spAnimator->SetAnimation(m_spModel->GetAnimation(animName));
}

bool Player::CanHighlight()
{
	const auto obj = m_focusObject.lock();

	if (!obj)
	{
		return false;
	}

	if (std::dynamic_pointer_cast<Interactable>(obj))
	{
		return true;
	}

	const auto usable = std::dynamic_pointer_cast<Usable>(obj);

	if (usable)
	{
		return usable->CanUse(std::dynamic_pointer_cast<Player>(shared_from_this()));
	}

	return false;
}

void Player::OnArrested(const std::shared_ptr<Police>& police)
{
	CharacterBase::OnArrested(police);

	const bool wasSuspicious = IsSuspicious();

	m_isReleasing = false;
	m_isHinderCamReturning = false;

	CancelUse();

	// 妨害中なら解除
	EndHinder(false, true);

	// 逮捕ペナルティ
	const float penaltyRate = 0.2f;

	const int calculatedPenalty = static_cast<int>(GetScore() * penaltyRate);

	// 0未満にしない
	const int penalty = std::min(calculatedPenalty, GetScore());
	if (penalty > 0)
	{
		AddScore(-penalty);

		SpawnFloatingText(
			"Arrest Penalty  -" + std::to_string(penalty),
			{ 0, 160 },
			{ 0.9f, 0.3f, 0.3f, 1.0f }
		);
	}

	if (const auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Arrested.wav"))
	{
		se->SetVolume(0.2f);
	}

	ClearFocus();

	{
		const auto shakeText = std::make_shared<ShakeTextUI>();
		shakeText->Init();
		shakeText->SetText("ARRESTED!!");
		shakeText->SetPosition({ 0, 120 });
		SceneManager::Instance().AddObject(shakeText);
	}

	if (wasSuspicious)
	{
		const auto shakeText = std::make_shared<ShakeTextUI>();
		shakeText->Init();
		shakeText->SetText("Mistaken for the culprit!");
		shakeText->SetPosition({ 0, 40 });
		shakeText->SetFont(FontID::Future48);
		SceneManager::Instance().AddObject(shakeText);
	}

	SelectInventory(-1);

	SwitchToArrestCamera();
}

void Player::OnReleased()
{
	m_isReleasing = true;
}

bool Player::IsSuspicious() const
{
	if (!m_isUsing)
	{
		return false;
	}

	const auto obj = std::dynamic_pointer_cast<InteractiveObject>(m_useObject.lock());

	if (!obj)
	{
		return false;
	}

	// 初期損壊は疑われない
	if (obj->GetDamageSource() != DamageSource::NPC)
	{
		return false;
	}

	return obj->GetSecondsSinceDamaged() <= kSuspicionSeconds;
}

bool Player::CanDeliver() const
{
	if (CountDangerousItems() <= 0) return false;

	const auto point = LocationPointManager::Instance().FindByTag(LocationTag::DeliveryPoint);
	if (!point) return false;

	Math::Vector3 diff = point->GetPos() - m_pos;
	diff.y = 0.0f;

	const float r = kDeliverRadius * m_sceneScale;
	return diff.LengthSquared() <= r * r;
}

int Player::DeliverDangerousItems()
{
	const int n = ConfiscateDangerousItems();
	if (n <= 0) return 0;

	// 初回ボーナス獲得時は時間を最大にする
	if (m_bonusCount <= 0)
	{
		m_bonusTime = kBonusMaxTime;
	}
	else
	{
		m_bonusTime = std::min(m_bonusTime + kBonusExtend * n, kBonusMaxTime);
	}

	m_bonusCount = std::min(m_bonusCount + n, kBonusMaxCount);

	SpawnFloatingText(
		"Turn in!  x" + std::to_string(n),
		{ 0, 160 },
		{ 0.3f, 1.0f, 0.5f, 1.0f },
		FontID::Future48
	);

	if (const auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Delivery.wav"))
	{
		se->SetVolume(0.2f);
	}

	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 2.0f);

	return n;
}

bool Player::TryDeliver()
{
	if (!CanDeliver()) return false;

	DeliverDangerousItems();
	return true;
}

void Player::UpdateRepairBonus()
{
	if (m_bonusTime <= 0.0f) return;
	m_bonusTime -= 1 / 60.0f;
	if (m_bonusTime <= 0.0f) { m_bonusTime = 0.0f; m_bonusCount = 0; }
}

void Player::SpawnFloatingText(const std::string& text, const Math::Vector2& pos, const Math::Color& color, FontID font) const
{
	const auto floatingText = std::make_shared<FloatingTextUI>();
	floatingText->Init();
	floatingText->SetText(text);
	floatingText->SetPosition(pos);
	floatingText->SetColor(color);
	floatingText->SetFont(font);
	SceneManager::Instance().AddObject(floatingText);
}
