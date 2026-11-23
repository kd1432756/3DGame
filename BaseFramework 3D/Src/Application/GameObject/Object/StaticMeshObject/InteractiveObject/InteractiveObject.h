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

enum class DamageSource
{
	None,
	Initial,		// 開始時から壊れている
	NPC			// NPCに壊された
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

	std::string actionText = "";

	bool npcCanUse = true;
};

class InteractiveObject
	: public StaticMeshObject
	, public Usable
{
public:

	void DrawLit() override;
	void DrawBright() override;

	bool CanUse(const std::shared_ptr<Player>& player) override;
	void Use(const std::shared_ptr<Player>& player) override;
	virtual float GetUseTime(const std::shared_ptr<Player>& player) const;

	virtual std::string GetActionText(const std::shared_ptr<Player>& player) const;

	bool CanNPCUse(const ItemData& item, const std::string& npcName) const override;
	float GetNPCUseTime(const ItemData& itemData, const std::string& npcName) const;
	void NPCUse(const ItemData& item, const std::shared_ptr<Neighbor>& npc) override;

	bool  CanNPCUseBare(const std::string& npcName) const;
	float GetNPCUseTimeBare(const std::string& npcName) const;
	void  NPCUseBare(const std::shared_ptr<Neighbor>& npc);

	bool IsDamaged() const { return m_state == InteractiveState::Damaged; }

	bool SetInitialDamaged();
	DamageSource GetDamageSource() const { return m_damageSource; }
	float GetDamagedTime() const { return m_damagedTime; }

	const std::string& GetIconPath() const { return m_iconPath; }

	bool CanBeTargetedBy(const CharacterBase* npc) const;
	bool TryReserve(const std::shared_ptr<CharacterBase>& npc);
	void ReleaseReservation(const CharacterBase* npc);

	float GetSecondsSinceDamaged() const;

	// NPCが壊せる可能性があるか
	bool CanEverBeBrokenBy(const ItemData& item) const;

protected:

	// 状態が変わった後の処理
	virtual void OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
	{}

	// アクション登録
	void RegisterAction(const InteractiveAction& action);

	const InteractiveAction* FindAction(const std::shared_ptr<Player>& player) const;

	const InteractiveAction* FindNPCAction(const ItemData& item, const std::string& npcName) const;

	const InteractiveAction* FindNPCBareAction(const std::string& npcName) const;
	void ExecuteNPCAction(const InteractiveAction& action, const std::shared_ptr<Neighbor>& npc);

	virtual void OnInitialDamage() {} // 初期損壊の時
	void NotifyPlayer(const InteractiveAction& action);
	void OnStateChanged(DamageSource source);

	// 現在の状態
	InteractiveState m_state = InteractiveState::Normal;

	std::vector<InteractiveAction> m_actions;

	float m_limLightTimer = 1.0f;

	DamageSource m_damageSource = DamageSource::None;
	float m_damagedTime = 0.0f;

	std::string m_iconPath = "";

	std::weak_ptr<CharacterBase> m_reservedBy;
	float m_repairedTime = -100.0f;

	// 修復直後に壊されない(ゲーム内時間)
	static constexpr float kRepairCooldownHours = 0.5f;
};