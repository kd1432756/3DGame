#include "LocationPointManager.h"

std::shared_ptr<LocationPoint> LocationPointManager::CreatePoint(
	LocationTag tag,
	const Math::Vector3& pos)
{
	auto point =
		std::make_shared<LocationPoint>(
			m_nextID++,
			tag,
			pos);

	m_points.push_back(point);

	return point;
}

std::shared_ptr<LocationPoint> LocationPointManager::CreatePointWithID(
	int id,
	LocationTag tag,
	const Math::Vector3& pos)
{
	auto point =
		std::make_shared<LocationPoint>(
			id,
			tag,
			pos);

	m_points.push_back(point);

	if (id >= m_nextID)
	{
		m_nextID = id + 1;
	}

	return point;
}

void LocationPointManager::DeletePoint(int id)
{
	m_points.erase(
		std::remove_if(
			m_points.begin(),
			m_points.end(),
			[id](const std::shared_ptr<LocationPoint>& p)
			{
				return p && p->GetID() == id;
			}),
		m_points.end());
}

void LocationPointManager::Clear()
{
	m_points.clear();

	m_nextID = 0;
}

std::shared_ptr<LocationPoint> LocationPointManager::FindPoint(int id) const
{
	for (auto& p : m_points)
	{
		if (p && p->GetID() == id)
		{
			return p;
		}
	}

	return nullptr;
}

std::shared_ptr<LocationPoint> LocationPointManager::FindByTag(LocationTag tag) const
{
	for (auto& p : m_points)
	{
		if (p && p->GetTag() == tag)
		{
			return p;
		}
	}

	return nullptr;
}

std::vector<std::shared_ptr<LocationPoint>> LocationPointManager::FindAllByTag(LocationTag tag) const
{
	std::vector<std::shared_ptr<LocationPoint>> result;

	for (auto& p : m_points)
	{
		if (p && p->GetTag() == tag)
		{
			result.push_back(p);
		}
	}

	return result;
}