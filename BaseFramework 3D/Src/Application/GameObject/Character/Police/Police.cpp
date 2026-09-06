/*#include "Police.h"

#include "../../../Scene/SceneManager.h"
#include "../../../System/Node/RoadNode/RoadNodeManager.h"
#include "../../../System/Node/RoadNode/RoadNode.h"

#include "../../../System/VisionLight/VisionLight.h"

#include "../../Effect/AlertIcon/AlertIcon.h"

#include "../Player/Player.h"

void Police::Init()
{
	BaseCharacter::Init();

	SetName("Police");

	m_spModel->SetModelData(
		"Asset/Models/Character/Police/Police2.gltf"
	);

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape(
		"Editor",
		m_spModel,
		KdCollider::TypeEditor
	);

	m_state = State::Patrol;

	m_currentNode.reset();
	m_previousNode.reset();

	FindNearestNode();

	// -----------------------------------------
	// 視界ライト
	// -----------------------------------------
	m_spVisionLight =
		std::make_shared<VisionLight>();

	m_spVisionLight->SetVision(
		0.8f,		// 視界距離
		180.0f		// 視野角
	);

	m_spVisionLight->SetDivision(32);

	m_spVisionLight->SetVisionColor(
		Math::Color{
			1.0f,
			1.0f,
			0.0f,
			1.0f
		}
	);

	//======
	if (!m_icon)
	{
		m_icon = std::make_shared<AlertIcon>();
		m_icon->Init();
		m_icon->SetOwner(std::dynamic_pointer_cast<BaseCharacter>(
			shared_from_this()));
		m_icon->SetIconScale(0.1f);
		SceneManager::Instance().AddObject(m_icon);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Police::Update()
{
	switch (m_state)
	{
	case State::Patrol:
		UpdatePatrol();
		break;

	case State::Alert:
		UpdateAlert();
		break;

	case State::Chase:
		UpdateChase();
		break;

	case State::Search:
		UpdateSearch();
		break;

	case State::ReturnToPatrol:
		UpdateReturnToPatrol();
		break;
	}

	ApplyGravity();
	
	UpdateWorldMatrix();
}

void Police::PostUpdate()
{
	UpdateAnimation();
}

void Police::DrawUnLit()
{
	if (m_spVisionLight)
	{
		Math::Vector3 backward = m_mWorld.Backward();
		backward.y = 0.0f;
		backward.Normalize();

		float yaw = atan2f(backward.x, backward.z);

		Math::Vector3 pos = m_pos;

		pos.y -= GetScaleFactor() * 1.15f;

		//pos.y = std::min(pos.y, 0.05f);

		Math::Matrix lightWorld =
			Math::Matrix::CreateRotationY(yaw) *
			Math::Matrix::CreateTranslation(pos);

		KdShaderManager::Instance().
			m_StandardShader.DrawPolygon(
				*m_spVisionLight,
				lightWorld,
				m_spVisionLight->GetVisionColor(),
				Math::Vector3::Zero
			);
	}
}

void Police::SetPos(const Math::Vector3& pos)
{
	m_pos = pos;

	UpdateWorldMatrix();

	m_currentNode.reset();
	m_previousNode.reset();

	FindNearestNode();
}

void Police::SetScale(const Math::Vector3& scale)
{
	m_scale = scale;

	UpdateWorldMatrix();

	if (m_spVisionLight)
	{
		m_spVisionLight->SetVision(
			GetScaleFactor() * 16.0f,		// 視界距離
			180.0f		// 視野角
		);
	}
}

void Police::UpdatePatrol()
{
	if (CanDetectPlayer())
	{
		auto player =
			SceneManager::Instance().FindObject<Player>();

		if (player)
		{
			m_lastSeenPlayerPos = player->GetPos();
		}

		m_alertTimer = 0.0f;

		m_icon->SetIcon(
			AlertIcon::IconType::Exclamation
		);

		ChangeState(State::Alert);

		return;
	}

	if (!m_targetNode)
	{
		return;
	}

	Math::Vector3 targetPos =
		m_targetNode->GetPos();

	Math::Vector3 moveDir =
		targetPos - m_pos;

	float distance =
		moveDir.Length();

	// -----------------------------------------
	// ノード到着
	// -----------------------------------------

	float speed =
		GetMoveSpeed(MoveSpeed::Patrol);

	if (distance <= speed)
	{
		m_pos = targetPos;

		// 今までのノードを記録
		m_previousNode = m_currentNode;

		// 現在のノードを更新
		m_currentNode = m_targetNode;

		// 次のノードを決定
		SelectNextNode();

		return;
	}

	// -----------------------------------------
	// ノードへ移動
	// -----------------------------------------
	moveDir.Normalize();

	RotateToMoveDirection(moveDir);

	SetAnimation(AnimState::Walk);

	MoveCharacter(
		moveDir,
		GetMoveSpeed(MoveSpeed::Patrol)
	);
}

void Police::UpdateAlert()
{
	SetAnimation(AnimState::Idle);

	auto player =
		SceneManager::Instance().FindObject<Player>();

	// Playerがまだ見えているなら
	// 最後に見た位置を更新
	if (player && CanDetectPlayer())
	{
		m_lastSeenPlayerPos = player->GetPos();
	}

	// Alert時間が終わったらChase
	m_alertTimer += 1.0f / 60.0f;

	if (m_alertTimer >= m_alertTime)
	{
		m_alertTimer = 0.0f;

		ChangeState(State::Chase);
	}
}

void Police::UpdateChase()
{
	auto player = SceneManager::Instance().FindObject<Player>();

	// Playerがいない
	if (!player)
	{
		ChangeState(State::Patrol);
		return;
	}

	// -----------------------------------------
	// Playerを発見している
	// -----------------------------------------
	if (CanDetectPlayer())
	{
		// 最後に見た場所を更新
		m_lastSeenPlayerPos = player->GetPos();

		Math::Vector3 moveDir = player->GetPos() - m_pos;

		moveDir.y = 0.0f;

		float distance = moveDir.Length();

		if (distance <= GetScaleFactor() * 3)
		{
			SetAnimation(AnimState::Idle);
			return;
		}

		moveDir.Normalize();

		RotateToMoveDirection(moveDir);

		SetAnimation(AnimState::Sprint);

		MoveCharacter(
			moveDir,
			GetMoveSpeed(MoveSpeed::Chase)
		);

		return;

	}

	// -----------------------------------------
	// Playerを見失った
	// -----------------------------------------
	Math::Vector3 moveDir =
		m_lastSeenPlayerPos - m_pos;

	moveDir.y = 0.0f;

	float distance =
		moveDir.Length();

	// 最後に見た場所へ到着
	if (distance <= 0.01f)
	{
		m_searchTimer = 0.0f;

		m_icon->SetIcon(
			AlertIcon::IconType::Question
		);

		ChangeState(State::Search);
		return;
	}

	moveDir.Normalize();

	Math::Vector3 wallNormal;

	if (CheckWallAhead(moveDir, wallNormal))
	{
		m_isWallFollowing = true;
		m_wallFollowTimer = 0.0f;

		m_wallFollowDir =
			GetWallFollowDirection(
				wallNormal,
				moveDir);

		UpdateWallFollow();

		return;
	}

	RotateToMoveDirection(moveDir);

	SetAnimation(AnimState::Sprint);

	MoveCharacter(
		moveDir,
		GetMoveSpeed(MoveSpeed::Chase)
	);
}

void Police::UpdateSearch()
{
	// -----------------------------------------
	// Playerを再発見
	// -----------------------------------------
	if (CanDetectPlayer())
	{
		m_searchTimer = 0.0f;

		auto player =
			SceneManager::Instance().FindObject<Player>();

		if (player)
		{
			m_lastSeenPlayerPos = player->GetPos();
		}

		m_alertTimer = 0.0f;

		m_icon->SetIcon(
			AlertIcon::IconType::Exclamation
		);

		ChangeState(State::Alert);

		return;
	}

	// -----------------------------------------
	// 最後に見た場所へ移動
	// -----------------------------------------
	Math::Vector3 moveDir =
		m_lastSeenPlayerPos - m_pos;

	moveDir.y = 0.0f;

	float distance =
		moveDir.Length();

	float speed =
		GetMoveSpeed(MoveSpeed::Chase);

	if (distance <= speed)
	{
		m_pos = m_lastSeenPlayerPos;
	}
	else
	{
		moveDir.Normalize();

		RotateToMoveDirection(moveDir);

		SetAnimation(AnimState::Sprint);

		MoveCharacter(
			moveDir,
			GetMoveSpeed(MoveSpeed::Chase)
		);

		return;
	}

	// -----------------------------------------
	// 到着したので探索開始
	// -----------------------------------------

	SetAnimation(AnimState::Idle);

	m_searchTimer += 1.0f / 60.0f;

	// -----------------------------------------
	// 一定時間探しても見つからない
	// -----------------------------------------
	if (m_searchTimer >= m_searchTime)
	{
		m_searchTimer = 0.0f;

		m_icon->SetIcon(
			AlertIcon::IconType::None
		);

		m_targetNode.reset();

		FindNearestNode();

		ChangeState(State::ReturnToPatrol);
	}
}

void Police::UpdateReturnToPatrol()
{
	if (CanDetectPlayer())
	{
		auto player =
			SceneManager::Instance().FindObject<Player>();

		if (player)
		{
			m_lastSeenPlayerPos = player->GetPos();
		}

		m_alertTimer = 0.0f;

		m_icon->SetIcon(
			AlertIcon::IconType::Exclamation
		);

		ChangeState(State::Alert);

		return;
	}

	if (!m_targetNode)
	{
		FindNearestNode();

		if (!m_targetNode)
		{
			ChangeState(State::Patrol);
			return;
		}
	}

	// =========================================
	// ノードへの方向
	// =========================================
	Math::Vector3 toNode =
		m_targetNode->GetPos() - m_pos;

	toNode.y = 0.0f;

	float dist = toNode.Length();

	// =========================================
	// ノード到着
	// =========================================

	float speed =
		GetMoveSpeed(MoveSpeed::ReturnToPatrol);

	if (dist <= speed)
	{
		m_pos = m_targetNode->GetPos();

		m_currentNode = m_targetNode;
		m_previousNode.reset();

		m_isWallFollowing = false;
		m_wallFollowTimer = 0.0f;
		m_wallClearTimer = 0.0f;

		SelectNextNode();

		ChangeState(State::Patrol);
		return;
	}

	toNode.Normalize();


	// =========================================
	// 壁沿い中
	// =========================================
	if (m_isWallFollowing)
	{
		m_wallFollowTimer += 1.0f / 60.0f;

		// 壁沿いに進む
		RotateToMoveDirection(
			m_wallFollowDir);

		SetAnimation(AnimState::Walk);

		MoveCharacter(
			m_wallFollowDir,
			GetMoveSpeed(MoveSpeed::ReturnToPatrol)
		);

		// -----------------------------------------
		// ノード方向に進めるか確認
		// -----------------------------------------
		Math::Vector3 wallNormal;

		if (!CheckWallAhead(
			toNode,
			wallNormal))
		{
			m_wallClearTimer += 1.0f / 60.0f;
		}
		else
		{
			m_wallClearTimer = 0.0f;
		}


		// -----------------------------------------
		// ある程度長く壁がなければ解除
		// -----------------------------------------
		if (m_wallClearTimer >= 0.3f)
		{
			m_isWallFollowing = false;

			m_wallFollowTimer = 0.0f;
			m_wallClearTimer = 0.0f;
		}

		// -----------------------------------------
		// 長時間抜けられなければ別処理
		// -----------------------------------------
		if (m_wallFollowTimer >= 3.0f)
		{
			m_isWallFollowing = false;

			m_wallFollowTimer = 0.0f;
			m_wallClearTimer = 0.0f;

			FindNearestNode();

			return;
		}

		return;
	}


	// =========================================
	// 通常：ノードへ向かう
	// =========================================
	Math::Vector3 wallNormal;

	if (CheckWallAhead(
		toNode,
		wallNormal))
	{
		m_isWallFollowing = true;

		m_wallFollowTimer = 0.0f;
		m_wallClearTimer = 0.0f;

		m_wallFollowDir =
			GetWallFollowDirection(
				wallNormal,
				toNode);

		RotateToMoveDirection(
			m_wallFollowDir);

		SetAnimation(AnimState::Walk);

		MoveCharacter(
			m_wallFollowDir,
			GetMoveSpeed(MoveSpeed::ReturnToPatrol)
		);

		return;
	}


	// =========================================
	// 壁なし
	// =========================================
	RotateToMoveDirection(toNode);

	SetAnimation(AnimState::Walk);


	ResolveCollision(toNode);

	MoveCharacter(
		toNode,
		GetMoveSpeed(MoveSpeed::ReturnToPatrol)
	);
}

void Police::UpdateWallFollow()
{
	auto player =
		SceneManager::Instance().FindObject<Player>();

	if (!player)
	{
		m_isWallFollowing = false;
		ChangeState(State::Patrol);
		return;
	}

	// Playerが再び見えた
	if (CanDetectPlayer())
	{
		m_isWallFollowing = false;
		m_wallFollowTimer = 0.0f;

		ChangeState(State::Chase);
		return;
	}

	m_wallFollowTimer += 1.0f / 60.0f;

	// 一定時間以上壁沿いに進んでもダメなら探索へ
	if (m_wallFollowTimer >= 3.0f)
	{
		m_isWallFollowing = false;
		m_wallFollowTimer = 0.0f;

		ChangeState(State::Search);
		return;
	}

	Math::Vector3 targetDir =
		player->GetPos() - m_pos;

	targetDir.y = 0.0f;

	if (targetDir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	targetDir.Normalize();

	// -----------------------------------------
	// まだ壁が目の前にあるか確認
	// -----------------------------------------

	Math::Vector3 wallNormal;

	if (!CheckWallAhead(
		targetDir,
		wallNormal))
	{
		// 壁がなくなったので通常追跡へ
		m_isWallFollowing = false;
		m_wallFollowTimer = 0.0f;

		return;
	}
	
	// -----------------------------------------
	// 壁沿いに移動
	// -----------------------------------------

	m_wallFollowDir =
		GetWallFollowDirection(
			wallNormal,
			targetDir);

	if (m_wallFollowDir.LengthSquared()
		<= 0.000001f)
	{
		return;
	}

	RotateToMoveDirection(
		m_wallFollowDir);

	MoveCharacter(
		m_wallFollowDir,
		GetMoveSpeed(MoveSpeed::Chase)
	);
}

float Police::GetMoveSpeed(MoveSpeed speed) const
{
	float scale = GetScaleFactor();

	switch (speed)
	{
	case MoveSpeed::Patrol:
		return 0.1f * scale;

	case MoveSpeed::Chase:
		return 0.15f * scale;

	case MoveSpeed::ReturnToPatrol:
		return 0.12f * scale;
	}

	return 0.0f;
}

void Police::ChangeState(State newState)
{
	if (m_state == newState)
	{
		return;
	}

	m_state = newState;

	ChangeVisionColor();
}

void Police::ChangeVisionColor()
{
	if (!m_spVisionLight)
	{
		return;
	}

	switch (m_state)
	{
	case State::Patrol:
		m_spVisionLight->SetVisionColor(
			Math::Color{ 1.0f, 1.0f, 0.0f, 1.0f }
		);
		break;

	case State::Alert:
		m_spVisionLight->SetVisionColor(
			Math::Color{ 1.0f, 0.5f, 0.0f, 1.0f }
		);
		break;

	case State::Chase:
		m_spVisionLight->SetVisionColor(
			Math::Color{ 1.0f, 0.0f, 0.0f, 1.0f }
		);
		break;

	case State::Search:
		m_spVisionLight->SetVisionColor(
			Math::Color{ 1.0f, 0.5f, 0.0f, 1.0f }
		);
		break;

	case State::ReturnToPatrol:
		m_spVisionLight->SetVisionColor(
			Math::Color{ 1.0f, 1.0f, 0.0f, 1.0f }
		);
		break;
	}
}

void Police::SetAnimation(AnimState state)
{
	if (m_animState == state)
	{
		return;
	}

	m_animState = state;

	switch (state)
	{
	case AnimState::Idle:
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation("Armature|Idle"));
		break;

	case AnimState::Walk:
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation("Armature|Walk"));
		break;

	case AnimState::Sprint:
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation("Armature|Sprint"));
		break;
	}
}

void Police::SelectNextNode()
{
	if (!m_currentNode)
	{
		return;
	}

	const auto& connections = m_currentNode->GetConnections();

	std::vector<std::shared_ptr<RoadNode>> candidates;

	for (const auto& weakNode : connections)
	{
		auto node = weakNode.lock();

		if (!node)
		{
			continue;
		}

		// 直前のノードには戻らない
		if (node == m_previousNode)
		{
			continue;
		}

		candidates.push_back(node);
	}

	// 候補がない場合
	// 一本道なら前のノードに戻るしかない
	if (candidates.empty())
	{
		for (const auto& weakNode : connections)
		{
			auto node = weakNode.lock();

			if (node)
			{
				candidates.push_back(node);
			}
		}
	}

	if (candidates.empty())
	{
		m_targetNode.reset();
		return;
	}

	// ランダムに次のノードを選ぶ
	int randomIndex = KdRandom::GetInt(0, candidates.size() - 1);

	m_targetNode = candidates[randomIndex];
}

void Police::MoveCharacter(const Math::Vector3& moveDir, float speed)
{
	m_pos +=
		moveDir * speed;

	ResolveCollision(
		moveDir
	);
}

bool Police::CanDetectPlayer()
{
	auto player =
		SceneManager::Instance().FindObject<Player>();

	if (!player)
	{
		return false;
	}

	// =====================================================
	// Playerまでの方向・距離
	// =====================================================

	Math::Vector3 toPlayer =
		player->GetPos() - m_pos;

	float distance =
		toPlayer.Length();

	// 検知距離
	const float detectRange = GetScaleFactor() * 16;

	if (distance <= 0.000001f)
	{
		return true;
	}

	if (distance > detectRange)
	{
		return false;
	}


	// =====================================================
	// 視野角
	// =====================================================

	toPlayer.Normalize();

	// Policeの正面方向
	Math::Vector3 forward =
		m_mWorld.Backward();

	forward.y = 0.0f;
	forward.Normalize();

	// Player方向も水平だけで判定
	Math::Vector3 playerDir = toPlayer;
	playerDir.y = 0.0f;

	if (playerDir.LengthSquared() <= 0.000001f)
	{
		return true;
	}

	playerDir.Normalize();

	float dot =
		forward.Dot(playerDir);

	// 視野角 180度
	// 半分の90度を判定
	const float cosFov =
		cosf(DirectX::XMConvertToRadians(90.0f));

	if (dot < cosFov)
	{
		return false;
	}


	// =====================================================
	// 壁越し判定
	// =====================================================

	KdCollider::RayInfo ray;

	ray.m_type = KdCollider::TypeGround | KdCollider::TypeBump;

	ray.m_pos = m_pos;
	ray.m_pos.y += 1.0f * GetScaleFactor();

	ray.m_dir =
		player->GetPos() - ray.m_pos;

	float rayDistance =
		ray.m_dir.Length();

	if (rayDistance <= 0.000001f)
	{
		return true;
	}

	ray.m_dir.Normalize();
	ray.m_range = rayDistance;


	std::list<KdCollider::CollisionResult> retRayList;

	for (auto& obj :
		SceneManager::Instance().GetObjList())
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

		// Player自身は遮蔽物として扱わない
		if (obj.get() == player.get())
		{
			continue;
		}

		obj->Intersects(
			ray,
			&retRayList);
	}


	// =====================================================
	// Rayが何かに当たった
	// =====================================================

	if (!retRayList.empty())
	{
		return false;
	}

	return true;
}

bool Police::CheckWallAhead(
	const Math::Vector3& moveDir,
	Math::Vector3& outNormal)
{
	KdCollider::RayInfo ray;

	ray.m_type =
		KdCollider::TypeGround |
		KdCollider::TypeBump;

	ray.m_pos = m_pos;
	ray.m_pos.y += 1.0f;

	ray.m_dir = moveDir;
	ray.m_dir.y = 0.0f;
	ray.m_dir.Normalize();

	ray.m_range = 0.8f * GetScaleFactor();

	std::list<KdCollider::CollisionResult> retList;

	for (auto& obj :
		SceneManager::Instance().GetObjList())
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
			ray,
			&retList);
	}

	if (retList.empty())
	{
		return false;
	}

	// 一番近い衝突結果を探す
	float maxOverlap = 0.0f;

	KdCollider::CollisionResult* hit = nullptr;

	for (auto& ret : retList)
	{
		if (ret.m_overlapDistance > maxOverlap)
		{
			maxOverlap = ret.m_overlapDistance;
			hit = &ret;
		}
	}

	if (!hit)
	{
		return false;
	}

	// 衝突面の法線
	outNormal = hit->m_hitNDir;

	outNormal.y = 0.0f;

	if (outNormal.LengthSquared() <= 0.000001f)
	{
		return false;
	}

	outNormal.Normalize();

	return true;
}

Math::Vector3 Police::GetWallFollowDirection(
	const Math::Vector3& wallNormal,
	const Math::Vector3& targetDir)
{
	Math::Vector3 wallDir =
		wallNormal.Cross({ 0, 1, 0 });

	wallDir.y = 0.0f;
	wallDir.Normalize();

	Math::Vector3 reverseWallDir =
		-wallDir;

	float dot1 =
		wallDir.Dot(targetDir);

	float dot2 =
		reverseWallDir.Dot(targetDir);

	if (dot1 > dot2)
	{
		return wallDir;
	}

	return reverseWallDir;
}

void Police::FindNearestNode()
{
	float nearestDist = FLT_MAX;

	m_targetNode.reset();

	const auto& nodes =
		RoadNodeManager::Instance().GetNodes();

	for (auto& node : nodes)
	{
		if (!node) continue;

		float dist =
			(node->GetPos() - m_pos).LengthSquared();

		if (dist < nearestDist)
		{
			nearestDist = dist;
			m_targetNode = node;
		}
	}
}*/
