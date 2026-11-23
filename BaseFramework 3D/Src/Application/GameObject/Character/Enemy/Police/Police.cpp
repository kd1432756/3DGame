#include "Police.h"

#include "../../../../System/Node/AStarNode/AStarNodeManager.h"
#include "../../../../System/Navigation/NavMesh/NavMeshManager.h"
#include "../../../../System/Navigation/NavMesh/NavMeshPathfinder.h"
#include "../../../../System/Location/LocationPointManager.h"

#include "../../../../Scene/SceneManager.h"

#include "../Neighbor/Neighbor.h"

void Police::Init()
{
	EnemyBase::Init();

	m_spModel->SetModelData("Asset/Models/Character/Police/Police2.gltf");

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	UpdateAnimation();

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape(
		"Police",
		m_spModel,
		KdCollider::TypeEditor
	);

	//============================================================
	// Emote Icon
	//============================================================

	InitEmoteIcon();

	//============================================================
	// State
	//============================================================

	ApplyStateSettings();

	ChangeState(State::Return);
}

void Police::Release() const
{
	KdShaderManager::Instance().WorkAmbientController().AddConeLight(
		{},
		0.0f,
		{},
		0.0f,
		{},
		false
	);
}
	
void Police::UpdateBehavior()
{
	UpdateTimers();

	switch (m_state)
	{
	case State::Patrol:
		UpdatePatrol();
		break;

	case State::Chase:
		UpdateChase();
		break;

	case State::Arrest:
		UpdateArrest();
		break;

	case State::Escort:
		UpdateEscort();
		break;

	case State::LastSeen:
		UpdateLastSeen();
		break;

	case State::Search:
		UpdateSearch();
		break;

	case State::Return:
		UpdateReturn();
		break;

	case State::Investigate:
		UpdateInvestigate();
		break;
	}
}

void Police::UpdateExtra() const
{
	const bool enable = (m_state != State::Arrest && m_state != State::Escort);

	const Math::Vector3 forward = GetFacingDir();

	KdShaderManager::Instance().WorkAmbientController().AddConeLight(
		Math::Vector3{ 3,0,0 },
		kSightDistance* m_sceneScale,
		m_pos,
		DirectX::XMConvertToRadians(kSightAngleDeg / 2.0f),
		forward,
		enable
	);
}

void Police::UpdateTimers()
{
	if (m_investigateCooldown > 0.0f)
	{
		m_investigateCooldown -= 1.0f / 60.0f;
	}
}

void Police::UpdatePatrol()
{
	// Playerを発見したらChase
	if (SearchTarget())
	{
		ChangeState(State::Chase);
		return;
	}

	if (m_investigateCooldown <= 0.0f && DetectDamageSound())
	{
		ChangeState(State::Investigate);
		return;
	}

	MoveToNextPathNode();
}

void Police::UpdateChase()
{
	const auto target = m_wpTarget.lock();

	if (!target)
	{
		ChangeState(State::LastSeen);

		m_loseSightTimer = 0.0f;

		return;
	}

	//============================================================
	// すでに連行中なら追跡を終了
	//============================================================

	if (target->IsArrested())
	{
		m_wpTarget.reset();

		m_chasingForCrime = false;

		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 逮捕対象でないなら追跡終了
	//============================================================

	if (!m_chasingForCrime && !IsWanted(target))
	{
		m_wpTarget.reset();

		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 確保判定
	//============================================================

	if (CanArrestTarget(target))
	{
		m_wpArrested = target;

		ChangeState(State::Arrest);
		return;
	}

	//============================================================
	// 追跡判定
	//============================================================

	const bool visible = CanSeeTarget(target);

	if (visible)
	{
		m_loseSightTimer = 0.0f;

		m_lastTargetPos = target->GetPos();
	}
	else
	{
		m_loseSightTimer += 1.0f / 60.0f;

		if (m_loseSightTimer >= kLoseSightTime)
		{
			ChangeState(State::LastSeen);

			m_loseSightTimer = 0.0f;

			return;
		}
	}

	//============================================================
	// Path更新
	//============================================================

	m_pathUpdateTimer += 1.0f / 60.0f;

	if (m_navPath.empty() || m_pathUpdateTimer >= kPathUpdateInterval)
	{
		m_pathUpdateTimer = 0.0f;

		FindChasePath(visible);
	}

	//============================================================
	// NavMesh Pathを進む
	//============================================================

	if (FollowNavPath() != MoveResult::Arrived)
	{
		if (visible)
		{
			// 経路は無いとき直進で近づく
			Math::Vector3 dir = target->GetPos() - m_pos;
			dir.y = 0.0f;
			SetMoveDirection(dir);
		}
	}
}

void Police::UpdateArrest()
{
	const auto target = m_wpArrested.lock();

	if (!target)
	{
		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 相手の方を向く
	//============================================================

	Math::Vector3 dir = target->GetPos() - m_pos;
	dir.y = 0.0f;

	if (dir.LengthSquared() > 0.0001f)
	{
		dir.Normalize();
		RotateToMoveDirection(dir);
	}

	//============================================================
	// 確保時間
	//============================================================

	m_arrestTimer += 1.0f / 60.0f;

	if (m_arrestTimer < kArrestTime)
	{
		return;
	}

	m_arrestTimer = 0.0f;

	//============================================================
	// 確保確定 → 連行開始
	//============================================================

	const Math::Vector3 pos = m_pos + Math::Vector3{ 0,GetScale().y * -0.7f,0 };
	KdEffekseerManager::GetInstance().Play("arrest.efkefc", pos, SceneManager::Instance().GetSceneScale() * 3.0f);

	BeginEscort(target);
}

void Police::UpdateEscort()
{
	const auto arrested = m_wpArrested.lock();

	if (!arrested)
	{
		EndEscort(false);
		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 連行相手を警察の背後に追従させる
	//============================================================

	const Math::Vector3 backward = -GetFacingDir();

	const float followDist = 1.2f * m_sceneScale;

	Math::Vector3 pos = m_pos + backward * followDist;
	pos.y = arrested->GetPos().y;

	arrested->SetPos(pos);

	//============================================================
	// Pathがなければ作る
	//============================================================

	if (m_navPath.empty())
	{
		FindPolicePostPath();

		if (m_navPath.empty())
		{
			EndEscort(true);
			ChangeState(State::Return);
			return;
		}
	}

	//============================================================
	// 交番へ移動
	//============================================================

	if (FollowNavPath() != MoveResult::Arrived)
	{
		return;
	}

	//============================================================
	// 交番に到着 → 没収して解放
	//============================================================

	EndEscort(true);

	ChangeState(State::Return);
}

void Police::UpdateLastSeen()
{
	//============================================================
	// Playerを再発見したらChase
	//============================================================

	if (SearchTarget())
	{
		ChangeState(State::Chase);

		m_searchTimer = 0.0f;

		return;
	}

	//============================================================
	// 音が聞こえたら調査
	//============================================================

	if (m_investigateCooldown <= 0.0f  && DetectDamageSound())
	{
		ChangeState(State::Investigate);
		return;
	}

	//============================================================
	// LastSeen Pathがない場合
	//============================================================

	if (m_navPath.empty())
	{
		m_navPath = FindNavPath(m_pos, m_lastTargetPos);

		m_navPathIndex = 0;
				
		//========================================================
		// Pathが作れない場合
		//========================================================

		if (m_navPath.empty())
		{
			m_searchTimer = 0.0f;

			ChangeState(State::Search);

			return;
		}
	}

	//============================================================
	// 最後に見た場所へ移動
	//============================================================

	if (FollowNavPath() == MoveResult::Arrived)
	{
		return;
	}

	//============================================================
	// 到着したのでSearchへ
	//============================================================

	ChangeState(State::Search);
}

void Police::UpdateSearch()
{
	//============================================================
	// Playerを再発見したらChase
	//============================================================

	if (SearchTarget())
	{
		m_searchTimer = 0.0f;
		
		ChangeState(State::Chase);

		return;
	}

	//============================================================
	// 音が聞こえたら調査
	//============================================================

	if (m_investigateCooldown <= 0.0f && DetectDamageSound())
	{
		ChangeState(State::Investigate);
		return;
	}

	//============================================================
	// Timer
	//============================================================

	m_searchTimer += 1.0f / 60.0f;

	//============================================================
	// Search終了
	//============================================================

	if (m_searchTimer >= kSearchTime)
	{
		m_searchTimer = 0.0f;

		m_wpTarget.reset();
		
		m_chasingForCrime = false;

		ChangeState(State::Return);

		ShowEmote(EmoteIcon::IconType::Sad, 2.0f);

		return;
	}

	//============================================================
	// 見回す
	//============================================================

	const float swingPeriod = 5.0f;   // 1往復の秒数
	const float swingAngle = 90.0f;

	const float offsetDeg = sinf(m_searchTimer * DirectX::XM_2PI / swingPeriod) * swingAngle;

	const float rad = DirectX::XMConvertToRadians(m_searchStartRotY + offsetDeg);

	Math::Vector3 lookDir(-sinf(rad), 0.0f, -cosf(rad));

	lookDir.y = 0.0f;
	lookDir.Normalize();

	//============================================================
	// 回転
	//============================================================

	RotateToMoveDirection(lookDir);
}

void Police::UpdateReturn()
{
	//============================================================
	// Playerを再発見したらChase
	//============================================================

	if (SearchTarget())
	{
		m_searchTimer = 0.0f;
		ChangeState(State::Chase);
		return;
	}

	if (m_investigateCooldown <= 0.0f && DetectDamageSound())
	{
		ChangeState(State::Investigate);
		return;
	}

	//============================================================
	// Return Pathがない場合
	//============================================================

	if (m_navPath.empty())
	{
		//========================================================
		// 現在位置に一番近いPatrol Node
		//========================================================

		const int nearestNode = FindNearestPatrolNode(m_pos);

		if (nearestNode < 0)
		{
			ChangeState(State::Patrol);
			return;
		}

		const auto node = AStarNodeManager::Instance().FindNode(nearestNode);

		if (!node)
		{
			ChangeState(State::Patrol);
			return;
		}

		//========================================================
		// Patrol NodeへNavMeshで移動
		//========================================================

		m_navPath = FindNavPath(m_pos, node->GetPos());

		m_navPathIndex = 0;

		// Path作成失敗
		if (m_navPath.empty())
		{
			m_currentStartNode = nearestNode;

			ChangeState(State::Patrol);

			FindPatrolPath();

			return;
		}
	}

	//============================================================
	// Patrol Nodeへ移動
	//============================================================

	if (FollowNavPath() != MoveResult::Arrived)
	{
		return;
	}

	//============================================================
	// 現在位置に一番近いPatrol Node
	//============================================================

	const int nearestNode = FindNearestPatrolNode(m_pos);

	if (nearestNode >= 0)
	{
		m_currentStartNode = nearestNode;
	}

	//============================================================
	// Patrolへ復帰
	//============================================================

	ChangeState(State::Patrol);

	FindPatrolPath();
}

void Police::UpdateInvestigate()
{
	//============================================================
	// 視認できたらChase
	//============================================================

	if (SearchTarget())
	{
		ChangeState(State::Chase);
		return;
	}

	const Math::Vector3 prevPos = m_investigatePos;

	if (DetectDamageSound())
	{
		Math::Vector3 diff = m_investigatePos - prevPos;
		diff.y = 0.0f;

		const float rebuildDist = 0.5f * m_sceneScale;
		if (diff.LengthSquared() > rebuildDist * rebuildDist)
		{
			ClearNavPath();
		}
	}

	if (m_navPath.empty())
	{
		m_navPath = FindNavPath(m_pos, m_investigatePos);

		m_navPathIndex = 0;

		if (m_navPath.empty())
		{
			ChangeState(State::Return); return; 
		}
	}

	if (FollowNavPath() == MoveResult::Moving)
	{
		return;
	}

	//============================================================
	// 到着 → 見回す
	//============================================================

	m_investigateCooldown = 5.0f;
	ChangeState(State::Search);
}

bool Police::DetectDamageSound()
{
	const float range = kHearRange * m_sceneScale;
	const float rangeSq = range * range;

	float nearestDistSq = rangeSq;
	Math::Vector3 nearestPos = Math::Vector3::Zero;
	bool found = false;

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->GetCategory() != KdGameObject::ObjectCategory::Character) continue;
		const auto character = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!character || character.get() == this) continue;
		if (std::dynamic_pointer_cast<Police>(character)) continue;
		if (character->IsArrested()) continue;
		if (!character->IsDamaging()) continue;

		const Math::Vector3 p = character->GetDamagePos();
		Math::Vector3 diff = p - m_pos;
		diff.y = 0.0f;

		const float distSq = diff.LengthSquared();
		if (distSq > nearestDistSq) continue;

		nearestDistSq = distSq;
		nearestPos = p;
		found = true;
	}

	if (!found)
	{
		return false;
	}

	m_investigatePos = nearestPos;
	return true;
}

bool Police::CanDispatch() const
{
	// 追跡・確保・連行・最後の目撃地点へ移動中は呼べない
	switch (m_state)
	{
	case State::Patrol:
	case State::Return:
	case State::Search:
	case State::Investigate:
		return true;
	default:
		return false;
	}
}

void Police::Dispatch(const Math::Vector3& pos)
{
	m_investigatePos = pos;
	m_wpTarget.reset();
	m_chasingForCrime = false;

	ChangeState(State::Investigate);
}

void Police::FindPatrolPath()
{
	const AStarNodeManager nodeManager = AStarNodeManager::Instance();

	const auto& nodes = nodeManager.GetNodes();

	if (nodes.empty())
	{
		return;
	}

	//============================================================
	// Start Node
	//============================================================

	int startID = m_currentStartNode;

	// 初回は現在位置に一番近いNode
	if (startID < 0)
	{
		startID = FindNearestPatrolNode(m_pos);
	}

	if (startID < 0)
	{
		return;
	}

	//============================================================
	// Goal候補
	//============================================================

	std::vector<int> candidates;

	for (const auto& node : nodes)
	{
		if (!node)
		{
			continue;
		}

		const int nodeID = node->GetID();

		if (nodeID == startID)
		{
			continue;
		}

		candidates.push_back(nodeID);
	}

	if (candidates.empty())
	{
		return;
	}

	//============================================================
	// ランダムにGoalを選択
	//============================================================

	const int goalID = candidates[KdRandom::GetInt(0, static_cast<int>(candidates.size()) - 1)];

	//============================================================
	// A*
	//============================================================

	m_currentPath =
		m_pathfinder.FindPath(
			nodeManager,
			startID,
			goalID
		);

	m_currentPathIndex = 0;
}

int Police::FindNearestPatrolNode(const Math::Vector3& pos) const
{
	const auto& nodes = AStarNodeManager::Instance().GetNodes();

	if (nodes.empty())
	{
		return -1;
	}

	//============================================================
	// 最近傍Node
	//============================================================

	int nearestNodeID = -1;

	float nearestDistanceSq = std::numeric_limits<float>::max();

	for (const auto& node : nodes)
	{
		if (!node)
		{
			continue;
		}

		//========================================================
		// Nodeとの距離
		//========================================================

		Math::Vector3 diff = node->GetPos() - pos;

		diff.y = 0.0f;

		const float distanceSq = diff.LengthSquared();

		//========================================================
		// 最短
		//========================================================

		if (distanceSq < nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;

			nearestNodeID = node->GetID();
		}
	}

	return nearestNodeID;
}

bool Police::MoveToNextPathNode()
{
	if (m_currentPath.empty())
	{
		return false;
	}

	if (m_currentPathIndex >= static_cast<int>(m_currentPath.size()))
	{
		return false;
	}

	//============================================================
	// 現在のTarget Node
	//============================================================

	const int nodeID = m_currentPath[m_currentPathIndex];

	const auto node = AStarNodeManager::Instance().FindNode(nodeID);

	if (!node)
	{
		return false;
	}

	//============================================================
	// Nodeへの方向
	//============================================================

	Math::Vector3 dir = node->GetPos() - m_pos;

	dir.y = 0.0f;

	//============================================================
	// 到着
	//============================================================

	const float arriveDistance = 0.2f * m_sceneScale;

	if (dir.LengthSquared() <= arriveDistance * arriveDistance)
	{
		m_currentPathIndex++;

		//========================================================
		// Pathの途中
		//========================================================

		if (m_currentPathIndex < static_cast<int>(m_currentPath.size()))
		{
			return true;
		}

		//========================================================
		// Patrol Point到着
		//========================================================

		m_currentStartNode = m_currentPath.back();

		//========================================================
		// 次のPathを探索
		//========================================================

		FindPatrolPath();

		return true;
	}

	//============================================================
	// 移動
	//============================================================

	SetMoveDirection(dir);

	return true;
}

void Police::FindChasePath(bool visible)
{
	const auto target = m_wpTarget.lock();

	if (!target)
	{
		return;
	}

	//============================================================
	// target位置
	//============================================================

	const Math::Vector3 goal = visible ? target->GetPos() : m_lastTargetPos;

	//============================================================
	// NavMesh Path探索
	//============================================================

	m_navPath = FindNavPath(m_pos, goal);

	//============================================================
	// Path開始
	//============================================================

	m_navPathIndex = 0;
}

void Police::OnRepathRequested()
{
	EnemyBase::OnRepathRequested();

	if (m_state == State::Patrol)
	{
		// 今いる場所に一番近いノードを起点に、巡回経路を引き直す
		const int nearest = FindNearestPatrolNode(m_pos);
		if (nearest >= 0)
		{
			m_currentStartNode = nearest;
			FindPatrolPath();
		}
	}
}

bool Police::SearchTarget()
{
	const auto& objList = SceneManager::Instance().GetObjList();

	for (const auto& obj : objList)
	{
		if (obj->GetCategory() != KdGameObject::ObjectCategory::Character) continue;

		const auto character = std::dynamic_pointer_cast<CharacterBase>(obj);

		if (!character) continue;

		// 自分自身
		if (character.get() == this) continue;

		// Policeは対象外
		if (std::dynamic_pointer_cast<Police>(character)) continue;

		// すでに確保されている相手は対象外
		if (character->IsArrested())continue;

		// 確保対象でない相手は対象外
		if (!IsWanted(character))continue;

		//========================================================
		// 視認できるか
		//========================================================

		if (!CanSeeTarget(character))
		{
			continue;
		}

		//========================================================
		// Target発見
		//========================================================

		m_wpTarget = character;

		m_chasingForCrime = character->IsDamaging() || character->IsSuspicious();

		m_lastTargetPos = character->GetPos();

		return true;
	}

	return false;
}

bool Police::CanSeeTarget(const std::shared_ptr<CharacterBase>& target) const
{
	if (!target)
	{
		return false;
	}

	//============================================================
	// 距離
	//============================================================

	const float searchDistance = kSightDistance * m_sceneScale;

	Math::Vector3 toTarget = target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float distanceSq = toTarget.LengthSquared();

	if (distanceSq > searchDistance * searchDistance)
	{
		return false;
	}

	if (distanceSq <= 0.0001f)
	{
		return false;
	}

	//============================================================
	// Policeの正面方向
	//============================================================

	Math::Vector3 backward = GetFacingDir();

	backward.y = 0.0f;
	backward.Normalize();

	//============================================================
	// Target方向
	//============================================================

	toTarget.Normalize();

	const float cosViewAngle = std::cos(DirectX::XMConvertToRadians(kSightAngleDeg / 2.0f));

	float dot = backward.Dot(toTarget);

	dot = std::clamp(dot, -1.0f, 1.0f);

	if (dot < cosViewAngle)
	{
		return false;
	}

	//============================================================
	// Ray
	//============================================================

	KdCollider::RayInfo ray;

	ray.m_type = KdCollider::TypeGround | KdCollider::TypeBump;

	const Math::Vector3 eye{ 0.0f, 0.7f * GetScale().y, 0.0f };
	ray.m_pos = m_pos + eye;

	const Math::Vector3 targetPos = target->GetPos() + Math::Vector3{ 0.0f, 0.7f * target->GetScale().y, 0.0f };

	ray.m_dir = targetPos - ray.m_pos;

	const float rayDistance = ray.m_dir.Length();

	if (rayDistance <= 0.000001f)
	{
		return false;
	}

	ray.m_dir.Normalize();

	ray.m_range = rayDistance;

	//============================================================
	// Ray判定
	//============================================================

	std::list<KdCollider::CollisionResult> retRayList;

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj)
		{
			continue;
		}

		// Target自身は遮蔽物にしない
		if (obj.get() == target.get())
		{
			continue;
		}

		// 自分自身も除外
		if (obj.get() == this)
		{
			continue;
		}

		obj->Intersects(ray, &retRayList);
	}

	// 遮蔽物あり
	if (!retRayList.empty())
	{
		return false;
	}

	return true;
}

bool Police::CanArrestTarget(const std::shared_ptr<CharacterBase>& target) const
{
	if (!target)
	{
		return false;
	}

	if (target->IsArrested())
	{
		return false;
	}

	Math::Vector3 diff = target->GetPos() - m_pos;
	diff.y = 0.0f;

	const float dist = m_arrestDistance * m_sceneScale;

	return diff.LengthSquared() <= dist * dist;
}

void Police::BeginEscort(const std::shared_ptr<CharacterBase>& target)
{
	target->OnArrested(std::dynamic_pointer_cast<Police>(shared_from_this()));

	m_wpTarget.reset();

	ChangeState(State::Escort);

	FindPolicePostPath();
}

void Police::EndEscort(bool confiscate)
{
	const auto arrested = m_wpArrested.lock();

	if (arrested)
	{
		if (confiscate)
		{
			arrested->ConfiscateDangerousItems();
		}

		arrested->OnReleased();
	}

	m_wpArrested.reset();
	m_arrestTimer = 0.0f;
}

void Police::FindPolicePostPath()
{
	const auto policeStation = LocationPointManager::Instance().FindByTag(LocationTag::PoliceStation);

	if (!policeStation)
	{
		return;
	}

	const Math::Vector3 policeStationPos = policeStation->GetPos();

	m_navPath = FindNavPath(m_pos, policeStationPos);

	m_navPathIndex = 0;
}
	
void Police::ChangeState(State newState)
{
	ClearNavPath();

	if (m_state == newState)
	{
		return;
	}

	m_state = newState;

	if (m_state == State::Chase)
	{
		m_loseSightTimer = 0.0f;
	}

	if (m_state == State::Search)
	{
		m_searchTimer = 0.0f;

		m_searchStartRotY = m_rot.y;
	}

	ApplyStateSettings();

	switch (m_state)
	{
	case State::Patrol:
		HideEmote();
		break;
	case State::Chase:
		ShowEmote(EmoteIcon::IconType::Exclamation);
		break;
	case State::Arrest:
		ShowEmote(EmoteIcon::IconType::Exclamation);
		break;
	case State::Escort:
		ShowEmote(EmoteIcon::IconType::Angry, 2.0f);
		break;
	case State::LastSeen:
		ShowEmote(EmoteIcon::IconType::Question);
		break;
	case State::Search:
		ShowEmote(EmoteIcon::IconType::Question);
		break;
	case State::Return:
		HideEmote();
		break;
	case State::Investigate:
		ShowEmote(EmoteIcon::IconType::Question);
		break;
	}
}

void Police::ApplyStateSettings()
{
	switch (m_state)
	{
	case State::Patrol:
		SetMoveSpeed(kPatrolSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Chase:
		SetMoveSpeed(kChaseSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	case State::Arrest:
		SetMoveSpeed(0.0f);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));
		break;
	case State::Escort:
		SetMoveSpeed(kEscortSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	case State::LastSeen:
		SetMoveSpeed(kReturnSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Search:
		SetMoveSpeed(0.0f);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));
		break;
	case State::Return:
		SetMoveSpeed(kReturnSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Investigate:
		SetMoveSpeed(kInvestigateSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	}
}
