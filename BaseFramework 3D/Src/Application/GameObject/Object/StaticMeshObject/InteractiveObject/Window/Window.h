#pragma once

#include "../InteractiveObject.h"

class Player;

class Window :	public InteractiveObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Window";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

private:

	void OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player) override;

	void RegisterCollider() override;
};