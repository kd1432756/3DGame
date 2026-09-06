#pragma once

#include"../StaticMeshObject.h"

class Tree : public StaticMeshObject
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Tree";
	}

private:
	void RegisterCollider() override;

};