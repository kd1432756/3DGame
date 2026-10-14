#pragma once

#include "../InteractiveObject.h"

class Player;

class PaintCan : public InteractiveObject
{
public:

	void Init() override;

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

	void OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player) override;

	void RegisterCollider() override;

};