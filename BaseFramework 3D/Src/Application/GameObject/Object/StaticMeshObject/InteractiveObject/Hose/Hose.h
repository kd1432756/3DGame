#pragma once

#include "../InteractiveObject.h"

class Player;

class Hose : public InteractiveObject
{
public:

	void Init() override;

	void DrawBright() override;

	const char* GetTypeName() const override
	{
		return "Hose";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

/*	bool CanUse(const std::shared_ptr<Player> player) override;

	void Use(std::shared_ptr<Player> player) override;

	float GetUseTime() const override;

	std::string GetActionText(const std::shared_ptr<Player>& player) const override
	{
		if (IsOwner(player))
		{
			return "Close";
		}

		return "Open";
	}*/

private:

	void RegisterCollider() override;

	enum class HoseState
	{
		Closed,
		Open,
	}; 
	
	HoseState m_state = HoseState::Closed;

	std::string m_target = "Hose";
};