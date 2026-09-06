#pragma once

#include "../StaticMeshObject.h"
#include "../../../../System/Usable/Usable.h"

#include "../Item/ItemData.h"

class Player;

enum class InteractiveState
{
	Normal,
	Damaged,
	Repaired
};

enum class ItemAction;

struct InteractiveAction
{
	InteractiveState fromState = InteractiveState::Normal;
	InteractiveState toState = InteractiveState::Normal;

	ItemAction action = ItemAction::None;
	std::string target;

	float useTime = 0.0f;

	int score = 0;
	std::string message;

	std::string modelPath;

	bool npcCanUse = false;
};

class InteractiveObject
	: public StaticMeshObject
	, public Usable
{
public:

	//==============================
	// Owner
	//==============================

	void SetOwner(const std::string& owner)
	{
		m_owner = owner;
	}

	const std::string& GetOwner() const
	{
		return m_owner;
	}

	//==============================
	// Usable
	//==============================

	bool CanUse(
		const std::shared_ptr<Player>& player) override;

	void Use(
		const std::shared_ptr<Player>& player) override;

	virtual float GetUseTime(
		const std::shared_ptr<Player>& player) const;

	virtual std::string GetActionText(
		const std::shared_ptr<Player>& player) const;

	bool CanNPCUse(
		const ItemData& item) const override;

	void NPCUse(
		const ItemData& item) override;

	float GetNPCUseTime(
		const ItemData& itemData) const;

protected:

	//==============================
	// Owner Check
	//==============================

	bool IsOwner(const std::shared_ptr<Player>& player) const;

	// アクション登録
	void RegisterAction(
		const InteractiveAction& action);

	// 実際に状態が変わった後の特殊処理
	virtual void OnAction(
		const InteractiveAction& action,
		const std::shared_ptr<Player>& player)
	{}

	const InteractiveAction* FindAction(
		const std::shared_ptr<Player>& player) const;

	const InteractiveAction* FindNPCAction(
		const ItemData& item) const;

	// 現在の状態
	InteractiveState m_state = InteractiveState::Normal;

	std::string m_owner;

	std::vector<InteractiveAction> m_actions;
};