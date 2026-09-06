/*#pragma once

#include"../../Base/BaseCharacter/BaseCharacter.h"

class RoadNode;

class VisionLight;

class AlertIcon;

class Police : public BaseCharacter
{
public:

	enum class State
	{
		Patrol,	// パトロール
		Alert,	// プレイヤーを発見した瞬間
		Chase,	// プレイヤーを追跡
		Search,	// プレイヤーを捜索
		ReturnToPatrol	// パトロールに戻る
	};

	void Init() override;
	void Update() override;
	void PostUpdate() override;

	void DrawUnLit() override;

	const char* GetTypeName() const override
	{
		return "Police";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Character;
	}

	void SetPos(const Math::Vector3& pos) override;

	void SetScale(const Math::Vector3& scale) override;
private:

	void UpdatePatrol();
	void UpdateAlert();
	void UpdateChase();
	void UpdateSearch();
	void UpdateReturnToPatrol();

	void UpdateWallFollow();

	enum class MoveSpeed
	{
		Patrol,
		Chase,
		ReturnToPatrol,
	};

	float GetMoveSpeed(MoveSpeed speed) const;

	void ChangeState(State newState);

	State m_state = State::Patrol;

	enum class AnimState
	{
		Idle,
		Walk,
		Sprint
	};

	void SetAnimation(AnimState state);

	AnimState m_animState = AnimState::Idle;
		
	std::weak_ptr<KdGameObject> m_target;

	void SelectNextNode();

	std::shared_ptr<RoadNode> m_currentNode;
	std::shared_ptr<RoadNode> m_targetNode;
	std::shared_ptr<RoadNode> m_previousNode;

	void MoveCharacter(
		const Math::Vector3& moveDir,
		float speed);

	bool CanDetectPlayer();

	Math::Vector3 m_lastSeenPlayerPos;

	float m_searchTimer = 0.0f;
	const float m_searchTime = 3.0f;

	bool CheckWallAhead(const Math::Vector3& moveDir,
		Math::Vector3& outNormal);

	Math::Vector3 GetWallFollowDirection(
		const Math::Vector3& wallNormal,
		const Math::Vector3& targetDir);

	// 壁沿い移動中か
	bool m_isWallFollowing = false;

	// 壁沿いに進む方向
	Math::Vector3 m_wallFollowDir = { 0, 0, 0 };

	// 壁沿い移動を開始してからの時間
	float m_wallFollowTimer = 0.0f;

	void FindNearestNode();

	float m_wallClearTimer = 0.0f;

	float m_alertTimer = 0.0f;
	float m_alertTime = 0.5f;

	std::shared_ptr<AlertIcon> m_icon = nullptr;;

private:

	std::shared_ptr<VisionLight> m_spVisionLight;

	void ChangeVisionColor();
};*/