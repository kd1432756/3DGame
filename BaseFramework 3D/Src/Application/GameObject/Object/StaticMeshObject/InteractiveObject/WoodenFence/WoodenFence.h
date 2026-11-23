#pragma once

#include "../InteractiveObject.h"

class Player;

class WoodenFence :	public InteractiveObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "WoodenFence";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

private:

	void OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player) override;

	void RegisterCollider() override;
};