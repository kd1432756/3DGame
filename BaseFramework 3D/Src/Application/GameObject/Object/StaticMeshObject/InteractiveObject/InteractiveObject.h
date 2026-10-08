#pragma once

#include "../StaticMeshObject.h"
#include "../../../../System/Usable/Usable.h"

#include "../Item/ItemData.h"

class Player;
class Neighbor;
class CharacterBase;

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

	std::string ownerMessage;

	std::string modelPath;

	bool needItem = true;

	std::string actionText = "";	// needItem=false 用の表示文言

	bool npcCanUse = true;
};

class InteractiveObject
	: public StaticMeshObject
	, public Usable
{
public:

	void DrawLit() override;

	void DrawBright() override;

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
		const ItemData& item,
		const std::string& npcName) const override;
	float GetNPCUseTime(
		const ItemData& itemData,
		const std::string& npcName) const;
	void NPCUse(
		const ItemData& item,
		const std::shared_ptr<Neighbor>& npc) override;

	bool  CanNPCUseBare(const std::string& npcName) const;
	float GetNPCUseTimeBare(const std::string& npcName) const;
	void  NPCUseBare(const std::shared_ptr<Neighbor>& npc);

	bool IsRepairAction(
		const std::shared_ptr<Player>& player) const override;

protected:

	// 実際に状態が変わった後の特殊処理
	virtual void OnAction(
		const InteractiveAction& action,
		const std::shared_ptr<Player>& player)
	{}

	//==============================
	// Owner Check
	//==============================

	bool IsOwner(const std::shared_ptr<Player>& player) const;

	std::shared_ptr<CharacterBase>FindOwner() const;

	// アクション登録
	void RegisterAction(
		const InteractiveAction& action);

	const InteractiveAction* FindAction(
		const std::shared_ptr<Player>& player) const;

	const InteractiveAction* FindNPCAction(
		const ItemData& item,
		const std::string& npcName) const;

	const InteractiveAction* FindNPCBareAction(const std::string& npcName) const;
	void ExecuteNPCAction(const InteractiveAction& action,
		const std::shared_ptr<Neighbor>& npc);

	void NotifyOwner(const InteractiveAction& action);

	// 現在の状態
	InteractiveState m_state = InteractiveState::Normal;

	std::string m_owner;

	std::vector<InteractiveAction> m_actions;

	float m_limLightTimer = 1.0f;
};