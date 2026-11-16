#pragma once

#include "LocationPoint.h"

class LocationPointManager
{
public:
	std::shared_ptr<LocationPoint> CreatePoint(
		LocationTag tag,
		const Math::Vector3& pos);

	void DeletePoint(int id);

	void Clear();

	std::shared_ptr<LocationPoint> FindPoint(int id) const;

	// 単数取得
	std::shared_ptr<LocationPoint> FindByTag(LocationTag tag) const;

	// 複数取得
	std::vector<std::shared_ptr<LocationPoint>> FindAllByTag(LocationTag tag) const;

	const std::vector<std::shared_ptr<LocationPoint>>& GetPoints() const { return m_points; }

	// Serializer用（IDを指定して直接復元する）
	std::shared_ptr<LocationPoint> CreatePointWithID(
		int id,
		LocationTag tag,
		const Math::Vector3& pos);

private:
	LocationPointManager() = default;

	std::vector<std::shared_ptr<LocationPoint>> m_points;

	int m_nextID = 0;

public:
	// シングルトンパターン
	static LocationPointManager& Instance()
	{
		static LocationPointManager instance;
		return instance;
	}
};