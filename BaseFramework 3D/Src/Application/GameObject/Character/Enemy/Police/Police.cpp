#include "Police.h"

#include "../../../../System/Node/AStarNode/AStarNodeManager.h"
#include "../../../../System/Navigation/NavMesh/NavMeshManager.h"
#include "../../../../System/Location/LocationPointManager.h"

#include "../../../../Scene/SceneManager.h"

#include "../../../Effect/AlertIcon/AlertIcon.h"

#include "../Neighbor/Neighbor.h"

void Police::Init()
{
	EnemyBase::Init();

	m_spModel->SetModelData(
		"Asset/Models/Character/Police/Police2.gltf"
	);

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape(
		"Police",
		m_spModel,
		KdCollider::TypeEditor
	);

	//============================================================
	// A* Node Manager
	//============================================================

	m_pNodeManager =
		&AStarNodeManager::Instance();

	//============================================================
	// Location Point Manager
	//============================================================

	m_pLocationPointManager =
		&LocationPointManager::Instance();

	//============================================================
	// Path
	//============================================================

	FindPatrolPath();

	//============================================================
	// Emote Icon
	//============================================================

	InitEmoteIcon();

	//============================================================
	// State
	//============================================================

	ApplyStateSettings();
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

void Police::UpdateExtra()
{
	bool enable = false;

	// 視界（コーンライト）の更新
	if (m_state == State::Arrest || m_state == State::Escort)
	{

	}
	else
	{
		enable = true;
	}

	Math::Vector3 backward = m_mWorld.Backward();
	backward.y = 0.0f;
	backward.Normalize();

	KdShaderManager::Instance().WorkAmbientController().AddConeLight(
		Math::Vector3{ 3,0,0 },
		8.0f * m_sceneScale,
		m_pos,
		DirectX::XMConvertToRadians(45),
		backward,
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

	if (m_investigateCooldown <= 0.0f && DetectPrankSound())
	{
		m_navPath.clear();
		m_navPathIndex = 0;
		ChangeState(State::Investigate);
		return;
	}

	MoveToNextPathNode();
}

void Police::UpdateChase()
{
	auto target = m_wpTarget.lock();

	if (!target)
	{
		ChangeState(State::LastSeen);

		m_navPath.clear();
		m_navPathIndex = 0;

		m_loseSightTimer = 0.0f;

		return;
	}

	//============================================================
	// すでに連行中なら追跡を終了
	//============================================================

	if (target->IsArrested())
	{
		m_wpTarget.reset();

		m_navPath.clear();
		m_navPathIndex = 0;

		m_chasingForPrank = false;

		ChangeState(State::Return);
		return;
	}

	//============================================================
	// ターゲットとの距離
	//============================================================

	Math::Vector3 toTarget =
		target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	float distance =
		toTarget.Length();

	//============================================================
	// 逮捕対象でないなら追跡終了
	//============================================================

	if (!m_chasingForPrank && !IsWanted(target))
	{
		m_wpTarget.reset();

		m_navPath.clear();
		m_navPathIndex = 0;

		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 確保判定
	//============================================================

	if (CanArrestTarget(target))
	{
		m_wpArrested = target;

		m_navPath.clear();
		m_navPathIndex = 0;

		ChangeState(State::Arrest);
		return;
	}

	//============================================================
	// 追跡判定
	//============================================================

	if (CanSeeTarget(target))
	{
		m_loseSightTimer = 0.0f;

		m_lastTargetPos =
			target->GetPos();
	}
	else
	{
		m_loseSightTimer += 1.0f / 60.0f;

		if (m_loseSightTimer >= m_loseSightTime)
		{
			ChangeState(State::LastSeen);

			m_navPath.clear();
			m_navPathIndex = 0;

			m_loseSightTimer = 0.0f;

			return;
		}
	}

	//============================================================
	// Path更新
	//============================================================

	m_pathUpdateTimer += 1.0f / 60.0f;

	if (m_navPath.empty() ||
		m_pathUpdateTimer >= m_pathUpdateInterval)
	{
		m_pathUpdateTimer = 0.0f;

		FindChasePath();
	}

	//============================================================
	// NavMesh Pathを進む
	//============================================================
	
	if (!MoveToNextNavPathPoint())
	{
		if (CanSeeTarget(target))
		{
			// 経路は無いが見えている → 直進で近づく
			Math::Vector3 dir = target->GetPos() - m_pos;
			dir.y = 0.0f;
			SetMoveDirection(dir);
		}
		else
		{
			// 経路も無く見えもしない → 見失いカウントを進める
			m_loseSightTimer += 1.0f / 60.0f;
		}
	}
}

void Police::UpdateArrest()
{
	auto target = m_wpArrested.lock();

	// 相手が消えた
	if (!target)
	{
		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 相手の方を向く(移動はしない)
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

	if (m_arrestTimer < m_arrestTime)
	{
		return;
	}

	m_arrestTimer = 0.0f;

	//============================================================
	// 確保確定 → 連行開始
	//============================================================

	Math::Vector3 pos = m_pos + Math::Vector3{ 0,GetScale().y * -0.95f,0 };
	KdEffekseerManager::GetInstance().Play("arrest.efkefc", pos, SceneManager::Instance().GetSceneScale() * 3.0f);

	BeginEscort(target);
}

void Police::UpdateEscort()
{
	auto arrested = m_wpArrested.lock();

	// 連行相手が消えた
	if (!arrested)
	{
		EndEscort(false);
		ChangeState(State::Return);
		return;
	}

	//============================================================
	// 連行相手を警察の背後に追従させる
	//============================================================

	{
		Math::Vector3 back = m_mWorld.Forward(); // Forwardが背後
		back.y = 0.0f;
		back.Normalize();

		const float followDist = 1.2f * m_sceneScale;

		Math::Vector3 pos = m_pos + back * followDist;
		pos.y = arrested->GetPos().y;

		arrested->SetPos(pos);
	}

	//============================================================
	// Pathがなければ作る
	//============================================================

	if (m_navPath.empty())
	{
		FindPolicePostPath();

		// 交番へ行けない → その場で没収して解放
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

	if (MoveToNextNavPathPoint())
	{
		return;
	}

	//============================================================
	// 交番に到着 → 没収して解放
	//============================================================

	m_navPath.clear();
	m_navPathIndex = 0;

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

		m_navPath.clear();
		m_navPathIndex = 0;

		m_searchTimer = 0.0f;

		return;
	}
	//============================================================
	// LastSeen Pathがない場合
	//============================================================

	if (m_navPath.empty())
	{
		if (!m_pNavMeshManager)
		{
			ChangeState(State::Return);
			return;
		}

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

	if (MoveToNextNavPathPoint())
	{
		return;
	}

	//============================================================
	// LastSeen到着
	//============================================================

	m_navPath.clear();
	m_navPathIndex = 0;

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
	// Timer
	//============================================================

	const float deltaTime = 1.0f / 60.0f;

	m_searchTimer += deltaTime;

	//============================================================
	// Search終了
	//============================================================

	if (m_searchTimer >= m_searchTime)
	{
		m_searchTimer = 0.0f;

		m_wpTarget.reset(); 
		
		m_chasingForPrank = false;

		ChangeState(State::Return);

		ShowEmote(EmoteIcon::IconType::Sad, 2.0f);

		return;
	}

	//============================================================
	// 見回す
	//============================================================

	const float swingPeriod = 5.0f;   // 左→右→左 1往復の秒数
	const float swingAngle = 90.0f;

	float offsetDeg =
		sinf(m_searchTimer * DirectX::XM_2PI / swingPeriod) * swingAngle;

	const float rad =
		DirectX::XMConvertToRadians(m_searchStartRotY + offsetDeg);

	Math::Vector3 lookDir(-sinf(rad), 0.0f, -cosf(rad));

	lookDir.y = 0.0f;
	lookDir.Normalize();

	//============================================================
	// 回転
	//============================================================

	RotateToMoveDirection(lookDir);

	// Search中の視界方向も現在の向きに合わせる
	m_moveDir = lookDir;
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

	if (m_investigateCooldown <= 0.0f && DetectPrankSound())
	{
		m_navPath.clear();
		m_navPathIndex = 0;
		ChangeState(State::Investigate);
		return;
	}

	//============================================================
	// Return Pathがない場合
	//============================================================

	if (m_navPath.empty())
	{
		if (!m_pNavMeshManager)
		{
			ChangeState(State::Patrol);
			return;
		}

		//========================================================
		// 現在位置に一番近いPatrol Node
		//========================================================

		int nearestNode =
			FindNearestPatrolNode(m_pos);

		if (nearestNode < 0)
		{
			ChangeState(State::Patrol);
			return;
		}

		auto node =
			m_pNodeManager->FindNode(nearestNode);

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
			m_currentStartNode =
				nearestNode;

			ChangeState(State::Patrol);

			FindPatrolPath();

			return;
		}
	}

	//============================================================
	// Patrol Nodeへ移動
	//============================================================

	if (MoveToNextNavPathPoint())
	{
		return;
	}

	//============================================================
	// Return完了
	//============================================================

	m_navPath.clear();
	m_navPathIndex = 0;

	//============================================================
	// 現在位置に一番近いPatrol Node
	//============================================================

	int nearestNode =
		FindNearestPatrolNode(m_pos);

	if (nearestNode >= 0)
	{
		m_currentStartNode =
			nearestNode;
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
		m_navPath.clear();
		m_navPathIndex = 0;
		return;
	}

	if (m_navPath.empty())
	{
		if (!m_pNavMeshManager) 
		{
			ChangeState(State::Return); return; 
		}

		m_navPath = FindNavPath(m_pos, m_investigatePos);

		m_navPathIndex = 0;

		if (m_navPath.empty()) 
		{
			ChangeState(State::Return); return; 
		}
	}

	if (MoveToNextNavPathPoint()) return;

	//============================================================
	// 到着 → 見回す
	//============================================================

	m_navPath.clear(); 
	m_navPathIndex = 0;
	m_investigateCooldown = 5.0f;
	ChangeState(State::Search);
}

bool Police::DetectPrankSound()
{
	const float range = m_hearRange * m_sceneScale;
	const float rangeSq = range * range;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto character = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!character || character.get() == this) continue;
		if (std::dynamic_pointer_cast<Police>(character)) continue;
		if (character->IsArrested()) continue;
		if (!character->IsPranking()) continue;

		const Math::Vector3 p = character->GetPrankPos();
		Math::Vector3 diff = p - m_pos;
		diff.y = 0.0f;
		if (diff.LengthSquared() > rangeSq) continue;

		m_investigatePos = p;
		return true;
	}
	return false;
}

void Police::FindPatrolPath()
{
	if (!m_pNodeManager)
	{
		return;
	}

	const auto& nodes =
		m_pNodeManager->GetNodes();

	if (nodes.empty())
	{
		return;
	}

	//============================================================
	// Start Node
	//============================================================

	int startID =
		m_currentStartNode;

	// 初回は現在位置に一番近いNode
	if (startID < 0)
	{
		startID =
			FindNearestPatrolNode(m_pos);
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

		const int nodeID =
			node->GetID();

		// 現在のNodeは除外
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

	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dist(
		0,
		static_cast<int>(candidates.size()) - 1
	);

	const int goalID =
		candidates[dist(gen)];


	//============================================================
	// A*
	//============================================================

	m_currentPath =
		m_pathfinder.FindPath(
			*m_pNodeManager,
			startID,
			goalID
		);

	m_currentPathIndex = 0;
}

int Police::FindNearestPatrolNode(
	const Math::Vector3& pos)
{
	if (!m_pNodeManager)
	{
		return -1;
	}

	const auto& nodes =
		m_pNodeManager->GetNodes();

	if (nodes.empty())
	{
		return -1;
	}


	//============================================================
	// 最近傍Node
	//============================================================

	int nearestNodeID = -1;

	float nearestDistanceSq =
		std::numeric_limits<float>::max();


	for (const auto& node : nodes)
	{
		if (!node)
		{
			continue;
		}


		//========================================================
		// Nodeとの距離
		//========================================================

		Math::Vector3 diff =
			node->GetPos() - pos;

		diff.y = 0.0f;

		float distanceSq =
			diff.LengthSquared();


		//========================================================
		// 最短
		//========================================================

		if (distanceSq < nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;

			nearestNodeID =
				node->GetID();
		}
	}

	return nearestNodeID;
}

bool Police::MoveToNextPathNode()
{
	if (!m_pNodeManager)
	{
		return false;
	}

	if (m_currentPath.empty())
	{
		return false;
	}

	if (m_currentPathIndex >=
		static_cast<int>(m_currentPath.size()))
	{
		return false;
	}


	//============================================================
	// 現在のTarget Node
	//============================================================

	const int nodeID =
		m_currentPath[m_currentPathIndex];

	auto node =
		m_pNodeManager->FindNode(nodeID);

	if (!node)
	{
		return false;
	}


	//============================================================
	// Nodeへの方向
	//============================================================

	Math::Vector3 dir =
		node->GetPos() - m_pos;

	dir.y = 0.0f;


	//============================================================
	// 到着
	//============================================================

	const float arriveDistance = 0.2f * m_sceneScale;

	if (dir.LengthSquared() <=
		arriveDistance * arriveDistance)
	{
		m_currentPathIndex++;

		//========================================================
		// Pathの途中
		//========================================================

		if (m_currentPathIndex <
			static_cast<int>(m_currentPath.size()))
		{
			return true;
		}


		//========================================================
		// Patrol Point到着
		//========================================================

		m_currentStartNode =
			m_currentPath.back();


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

void Police::FindChasePath()
{
	if (!m_pNavMeshManager)
	{
		return;
	}

	auto target = m_wpTarget.lock();

	if (!target)
	{
		return;
	}

	//============================================================
	// 見えているか？
	//============================================================

	const bool visible = CanSeeTarget(target);

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

bool Police::MoveToNextNavPathPoint()
{
	if (m_navPath.empty())
	{
		return false;
	}

	if (m_navPathIndex >=
		static_cast<int>(m_navPath.size()))
	{
		return false;
	}

	//============================================================
	// 現在のTarget Point
	//============================================================

	const Math::Vector3 targetPoint =
		m_navPath[m_navPathIndex];


	//============================================================
	// 移動方向
	//============================================================

	Math::Vector3 dir =
		targetPoint - m_pos;

	dir.y = 0.0f;


	//============================================================
	// 到着判定
	//============================================================

	const float arriveDistance = 0.2f * m_sceneScale;

	if (dir.LengthSquared() <=
		arriveDistance * arriveDistance)
	{
		m_navPathIndex++;

		//========================================================
		// 最後まで到着
		//========================================================

		if (m_navPathIndex >=
			static_cast<int>(m_navPath.size()))
		{
			SetMoveDirection(Math::Vector3::Zero);
			return false;
		}

		return true;
	}

	//============================================================
	// 移動
	//============================================================

	SetMoveDirection(dir);

	return true;
}

bool Police::SearchTarget()
{
	auto& objList =
		SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		if (!obj)
		{
			continue;
		}

		auto character =
			std::dynamic_pointer_cast<CharacterBase>(obj);

		if (!character)
		{
			continue;
		}

		// 自分自身
		if (character.get() == this)
		{
			continue;
		}

		// Policeは対象外
		if (std::dynamic_pointer_cast<Police>(character))
		{
			continue;
		}

		// すでに確保されている相手は対象外(二重連行防止)
		if (character->IsArrested())
		{
			continue;
		}

		// 確保対象でない相手は対象外
		if (!IsWanted(character))
		{
			continue;
		}

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

		m_chasingForPrank = character->IsPranking();

		m_lastTargetPos =
			character->GetPos();

		return true;
	}

	return false;
}

bool Police::CanSeeTarget(
	const std::shared_ptr<CharacterBase>& target)
{
	if (!target)
	{
		return false;
	}

	//============================================================
	// 距離
	//============================================================

	const float searchDistance =
		8.0f * m_sceneScale;

	Math::Vector3 toTarget =
		target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float distanceSq =
		toTarget.LengthSquared();

	if (distanceSq >
		searchDistance * searchDistance)
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

	Math::Vector3 backward =
		m_mWorld.Backward();

	backward.y = 0.0f;
	backward.Normalize();

	//============================================================
	// Target方向
	//============================================================

	toTarget.Normalize();

	const float viewAngle = 90.0f;

	const float cosViewAngle =
		std::cos(
			DirectX::XMConvertToRadians(
				viewAngle * 0.5f
			)
		);

	float dot =
		backward.Dot(toTarget);

	dot =
		std::clamp(dot, -1.0f, 1.0f);

	if (dot < cosViewAngle)
	{
		return false;
	}

	//============================================================
	// Ray
	//============================================================

	KdCollider::RayInfo ray;

	ray.m_type =
		KdCollider::TypeGround |
		KdCollider::TypeBump;

	ray.m_pos =
		m_pos + GetScale() * 0.7f;	// 目の高さに合わせる

	Math::Vector3 targetPos =
		target->GetPos() + target->GetScale() * 0.7f;	// 目の高さに合わせる

	ray.m_dir =
		targetPos - ray.m_pos;

	float rayDistance =
		ray.m_dir.Length();

	if (rayDistance <= 0.000001f)
	{
		return false;
	}

	ray.m_dir.Normalize();

	ray.m_range =
		rayDistance;

	//============================================================
	// Ray判定
	//============================================================

	std::list<KdCollider::CollisionResult> retRayList;

	for (auto& obj :
		SceneManager::Instance().GetObjList())
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

		obj->Intersects(
			ray,
			&retRayList
		);
	}

	// 遮蔽物あり
	if (!retRayList.empty())
	{
		return false;
	}

	return true;
}

bool Police::CanArrestTarget(
	const std::shared_ptr<CharacterBase>& target) const
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
	target->OnArrested(
		std::dynamic_pointer_cast<Police>(shared_from_this())
	);

	m_wpTarget.reset();

	m_navPath.clear();
	m_navPathIndex = 0;

	ChangeState(State::Escort);

	FindPolicePostPath();
}

void Police::EndEscort(bool confiscate)
{
	auto arrested = m_wpArrested.lock();

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
	if (!m_pNavMeshManager || !m_pLocationPointManager)
	{
		return;
	}

	auto policeStation =
		m_pLocationPointManager->FindByTag(LocationTag::PoliceStation);

	if (!policeStation)
	{
		return;
	}

	Math::Vector3 policeStationPos = policeStation->GetPos();

	m_navPath = FindNavPath(m_pos, policeStationPos);

	m_navPathIndex = 0;
}
	
void Police::ChangeState(State newState)
{
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

	if (m_state == State::Arrest)
	{
		//KdShaderManager::Instance().WorkAmbientController().
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
		SetMoveSpeed(m_patrolSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Chase:
		SetMoveSpeed(m_chaseSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	case State::Arrest:
		SetMoveSpeed(0.0f);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));
		break;
	case State::Escort:
		SetMoveSpeed(m_escortSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	case State::LastSeen:
		SetMoveSpeed(m_returnSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Search:
		SetMoveSpeed(0.0f);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));
		break;
	case State::Return:
		SetMoveSpeed(m_returnSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Walk"));
		break;
	case State::Investigate:
		SetMoveSpeed(m_investigateSpeed);
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
		break;
	}
}
