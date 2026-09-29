#pragma once

#include"../InteractiveObject.h"

class Player;

class Flower : public InteractiveObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Flower";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

private:

	void RegisterCollider() override;

	enum class FlowerState
	{
		Normal,
		Cut,
		Repaired
	}; 
	
	FlowerState m_state = FlowerState::Normal;
};