#pragma once

#include"../InteractiveObject.h"

class Player;

class Faucet : public InteractiveObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Faucet";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Interactive;
	}

private:

	void Update() override;

	void OnAction(
		const InteractiveAction& action,
		const std::shared_ptr<Player>& player);

	void RegisterCollider() override;

	std::weak_ptr<KdEffekseerObject> m_wpEffect;
};