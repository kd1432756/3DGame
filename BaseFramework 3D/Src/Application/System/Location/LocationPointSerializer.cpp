#include "LocationPointSerializer.h"

#include "LocationPointManager.h"

#include "json.hpp"

using json = nlohmann::json;

bool LocationPointSerializer::Save(
	const std::string& path,
	const LocationPointManager& manager)
{
	json j;

	json pointsArray = json::array();

	for (auto& point : manager.GetPoints())
	{
		if (!point)
		{
			continue;
		}

		json p;

		p["id"] = point->GetID();
		p["tag"] = static_cast<int>(point->GetTag());

		const Math::Vector3& pos = point->GetPos();

		p["pos"] = { pos.x, pos.y, pos.z };

		pointsArray.push_back(p);
	}

	j["points"] = pointsArray;

	std::ofstream ofs(path);

	if (!ofs.is_open())
	{
		return false;
	}

	ofs << j.dump(4);

	return true;
}

bool LocationPointSerializer::Load(
	const std::string& path,
	LocationPointManager& manager)
{
	std::ifstream ifs(path);

	if (!ifs.is_open())
	{
		return false;
	}

	json j;

	ifs >> j;

	manager.Clear();

	if (!j.contains("points"))
	{
		return true; // 空ファイルは許容
	}

	for (auto& p : j["points"])
	{
		int id = p.value("id", -1);

		LocationTag tag =
			static_cast<LocationTag>(
				p.value("tag", 0));

		Math::Vector3 pos = Math::Vector3::Zero;

		if (p.contains("pos") && p["pos"].is_array() && p["pos"].size() == 3)
		{
			pos.x = p["pos"][0];
			pos.y = p["pos"][1];
			pos.z = p["pos"][2];
		}

		manager.CreatePointWithID(id, tag, pos);
	}

	return true;
}