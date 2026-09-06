#pragma once

#include "../InteractiveObject.h"

class Player;

class WoodenFence :	public InteractiveObject
{
public:

	void Init() override;

	void DrawBright() override;

	const char* GetTypeName() const override
	{
		return "WoodenFence";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

private:

	void RegisterCollider() override;

	enum class WoodenFenceState
	{
		Normal,
		Broken,
	}; 
	
	WoodenFenceState m_state = WoodenFenceState::Normal;

	std::string m_target = "WoodenFence";
};