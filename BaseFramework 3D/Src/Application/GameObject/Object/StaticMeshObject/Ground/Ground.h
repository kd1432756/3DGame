#pragma once

#include "../StaticMeshObject.h"

class Player;

class Ground :
	public StaticMeshObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Ground";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Stage;
	}

private:

	void RegisterCollider() override;

};