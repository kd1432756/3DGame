#pragma once

#include "../StaticMeshObject.h"

class Player;

class Ground :
	public StaticMeshObject
{
public:

	enum class GroundType
	{
		None,
		Road,
		Grass,
		Dirt,
		Lot,
		Concrete
	};

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Ground";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Stage;
	}

	void SetGroundType(GroundType type)
	{
		m_groundType = type;
	}

	GroundType GetGroundType() const
	{
		return m_groundType;
	}

	bool IsGrass() const
	{
		return m_groundType == GroundType::Grass;
	}

	void CollectGrassPoints(std::vector<Math::Vector3>& out, float density) const;

private:

	void RegisterCollider() override;

	GroundType m_groundType = GroundType::None;
};