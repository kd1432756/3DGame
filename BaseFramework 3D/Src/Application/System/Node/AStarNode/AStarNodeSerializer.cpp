#include "AStarNodeSerializer.h"

#include "json.hpp"

using json = nlohmann::json;

bool AStarNodeSerializer::Save(
	const std::string& path,
	const AStarNodeManager& manager)
{
	json root;

	root["nodes"] = json::array();

	for (const auto& node :
		manager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		json nodeJson;

		nodeJson["id"] =
			node->GetID();

		const auto& pos =
			node->GetPos();

		nodeJson["position"]["x"] =
			pos.x;

		nodeJson["position"]["y"] =
			pos.y;

		nodeJson["position"]["z"] =
			pos.z;

		nodeJson["connections"] =
			node->GetConnections();

		root["nodes"].push_back(
			nodeJson);
	}

	std::ofstream ofs(path);

	if (!ofs)
	{
		return false;
	}

	ofs << root.dump(4);

	return true;
}

bool AStarNodeSerializer::Load(
	const std::string& path,
	AStarNodeManager& manager)
{
	if (manager.IsLoaded())
	{
		return true;
	}

	std::ifstream ifs(path);

	if (!ifs)
	{
		return false;
	}

	json root;

	try
	{
		ifs >> root;
	}
	catch (...)
	{
		return false;
	}

	manager.Clear();

	if (!root.contains("nodes"))
	{
		return true;
	}

	for (const auto& nodeJson :
		root["nodes"])
	{
		int id =
			nodeJson["id"];

		Math::Vector3 pos;

		pos.x =
			nodeJson["position"]["x"];

		pos.y =
			nodeJson["position"]["y"];

		pos.z =
			nodeJson["position"]["z"];

		auto node =
			manager.CreateNode(
				id,
				pos);

		if (!node)
		{
			continue;
		}

		if (nodeJson.contains("connections"))
		{
			node->SetConnections(
				nodeJson["connections"]
				.get<std::vector<int>>());
		}
	}

	manager.SetLoaded(true);

	return true;
}

bool AStarNodeSerializer::LoadWithScale(const std::string& path, AStarNodeManager& manager,	float scale)
{
	if (manager.IsLoaded())
	{
		return true;
	}

	std::ifstream ifs(path);

	if (!ifs)
	{
		return false;
	}

	json root;

	try
	{
		ifs >> root;
	}
	catch (...)
	{
		return false;
	}

	manager.Clear();

	if (!root.contains("nodes"))
	{
		return true;
	}

	for (const auto& nodeJson :
		root["nodes"])
	{
		int id =
			nodeJson["id"];

		Math::Vector3 pos;

		pos.x =
			nodeJson["position"]["x"];

		pos.y =
			nodeJson["position"]["y"];

		pos.z =
			nodeJson["position"]["z"];

		pos *= scale;

		auto node =
			manager.CreateNode(
				id,
				pos);

		if (!node)
		{
			continue;
		}

		if (nodeJson.contains("connections"))
		{
			node->SetConnections(
				nodeJson["connections"]
				.get<std::vector<int>>());
		}
	}

	manager.SetLoaded(true);

	return true;
}

