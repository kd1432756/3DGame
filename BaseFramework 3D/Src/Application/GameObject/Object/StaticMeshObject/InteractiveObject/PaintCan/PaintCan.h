#pragma once

#include "../InteractiveObject.h"

class Player;

class PaintCan : public InteractiveObject
{
public:

	void Init() override;

	void DrawBright() override;

	const char* GetTypeName() const override
	{
		return "PaintCan";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}
/*
	bool CanUse(const std::shared_ptr<Player> player) override;

	void Use(std::shared_ptr<Player> player) override;

	float GetUseTime() const override;

	std::string GetActionText(const std::shared_ptr<Player>& player) const override
	{
		if (IsOwner(player))
		{
			return "Clean";
		}

		return "Spill";
	}*/

private:

	void RegisterCollider() override;

	enum class PaintCanState
	{
		Normal,
		Spilled,
	}; 
	
	PaintCanState m_state = PaintCanState::Normal;

	std::string m_target = "PaintCan";
};