#include "SceneSerializer.h"

#include "../SceneManager.h"
#include "../..//GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"
#include "../..//GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../GameObject/Object/StaticMeshObject/Item/Item.h"
#include "../../GameObject/Object/StaticMeshObject/Item/ItemSpawner.h"

#include "json.hpp"

using json = nlohmann::json;

bool SceneSerializer::Save(const std::string& path)
{
	json root;
	root["Objects"] = json::array();

	auto& objList = SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		if (!obj) continue;

		if (std::string(obj->GetTypeName()) == "GameObject")
		{
			continue;
		}

		json objData;

		objData["Type"] = obj->GetTypeName();
		objData["Name"] = obj->GetName();

		auto pos = obj->GetPos();
		auto rot = obj->GetRot();
		auto scale = obj->GetScale();

		objData["Position"] = { pos.x, pos.y, pos.z };
		objData["Rotation"] = { rot.x, rot.y, rot.z };
		objData["Scale"] = { scale.x, scale.y, scale.z };

		auto color = obj->GetColor();
		objData["Color"] = { color.x, color.y, color.z };

		if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			objData["ModelPath"] = mesh->GetModelPath();

			objData["IsGround"] = mesh->IsGround();
		}

		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			objData["GroundType"] =
				static_cast<int>(ground->GetGroundType());
		}

		if (auto interactive =
			std::dynamic_pointer_cast<InteractiveObject>(obj))
		{
			objData["Owner"] = interactive->GetOwner();
		}

		if (auto item = std::dynamic_pointer_cast<Item>(obj))
		{
			objData["ItemPath"] = item->GetItemPath();
		}

		root["Objects"].push_back(objData);
	}

	std::ofstream ofs(path);

	if (!ofs)
	{
		return false;
	}

	ofs << root.dump(4);

	return true;
}

bool SceneSerializer::Load(const std::string& path)
{
	std::ifstream ifs(path);

	if (!ifs)
	{
		return false;
	}

	SceneManager::Instance().ClearObjects();

	json root;
	ifs >> root;

	for (auto& objData : root["Objects"])
	{
		std::string type = objData["Type"];
		std::string name = objData["Name"];

		auto obj = SceneManager::Instance().CreateObjectRaw(type);

		if (!obj)
		{
			continue;
		}

		obj->SetName(name);

		size_t underPos = name.find_last_of('_');

		if (underPos != std::string::npos)
		{
			std::string baseName =
				name.substr(0, underPos);

			int number =
				std::stoi(name.substr(underPos + 1));

			SceneManager::Instance().UpdateObjectCount(
				baseName,
				number + 1);
		}

		auto pos = objData["Position"];
		obj->SetPos({ pos[0], pos[1], pos[2] });

		auto rot = objData["Rotation"];
		obj->SetRot({ rot[0], rot[1], rot[2] });

		auto scale = objData["Scale"];
		obj->SetScale({ scale[0], scale[1], scale[2] });

		auto color = objData["Color"];
		obj->SetColor({ color[0], color[1], color[2] });

		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			if (objData.contains("GroundType"))
			{
				ground->SetGroundType(
					static_cast<Ground::GroundType>(
						objData["GroundType"].get<int>()));
			}
		}

		if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (objData.contains("ModelPath"))
			{
				mesh->SetModelPath(objData["ModelPath"]);
			}

			if (objData.contains("IsGround"))
			{
				mesh->SetIsGround(objData["IsGround"].get<bool>());
			}
		}

		if (auto interactive =
			std::dynamic_pointer_cast<InteractiveObject>(obj))
		{
			if (objData.contains("Owner"))
			{
				interactive->SetOwner(objData["Owner"]);
			}
		}

		if (auto item = std::dynamic_pointer_cast<Item>(obj))
		{
			if (objData.contains("ItemPath"))
			{
				item->LoadItem(objData["ItemPath"]);

				auto spawner = std::make_shared<ItemSpawner>();
				spawner->SetItemPath(item->GetItemPath());
				spawner->SetSpawnPos(item->GetPos());
				spawner->SetSpawnScale(item->GetScale());

				SceneManager::Instance().AddObject(spawner);

				item->SetSpawner(spawner);
			}
		}

		SceneManager::Instance().AddObject(obj);
	}

	return true;
}

bool SceneSerializer::LoadWithScale(const std::string& path, float multiplier)
{
	std::ifstream ifs(path);

	if (!ifs)
	{
		return false;
	}

	SceneManager::Instance().ClearObjects();

	json root;
	ifs >> root;

	for (auto& objData : root["Objects"])
	{
		std::string type = objData["Type"];
		std::string name = objData["Name"];

		auto obj = SceneManager::Instance().CreateObjectRaw(type);

		if (!obj)
		{
			continue;
		}

		obj->SetName(name);

		size_t underPos = name.find_last_of('_');

		if (underPos != std::string::npos)
		{
			std::string baseName =
				name.substr(0, underPos);

			int number =
				std::stoi(name.substr(underPos + 1));

			SceneManager::Instance().UpdateObjectCount(
				baseName,
				number + 1);
		}

		auto pos = objData["Position"];
		obj->SetPos({ pos[0], pos[1], pos[2] });
		obj->SetPos(obj->GetPos() * multiplier);

		auto rot = objData["Rotation"];
		obj->SetRot({ rot[0], rot[1], rot[2] });

		auto scale = objData["Scale"];
		obj->SetScale({ scale[0], scale[1], scale[2] });
		obj->SetScale(obj->GetScale() * multiplier);

		auto color = objData["Color"];
		obj->SetColor({ color[0], color[1], color[2] });

		if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (objData.contains("ModelPath"))
			{
				mesh->SetModelPath(objData["ModelPath"]);
			}

			if (objData.contains("IsGround"))
			{
				mesh->SetIsGround(objData["IsGround"].get<bool>());
			}
		}

		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			if (objData.contains("GroundType"))
			{
				ground->SetGroundType(
					static_cast<Ground::GroundType>(
						objData["GroundType"].get<int>()));
			}
		}

		if (auto interactive =
			std::dynamic_pointer_cast<InteractiveObject>(obj))
		{
			if (objData.contains("Owner"))
			{
				interactive->SetOwner(objData["Owner"]);
			}
		}

		if (auto item = std::dynamic_pointer_cast<Item>(obj))
		{
			if (objData.contains("ItemPath"))
			{
				item->LoadItem(objData["ItemPath"]);

				auto spawner = std::make_shared<ItemSpawner>();
				spawner->SetItemPath(item->GetItemPath());
				spawner->SetSpawnPos(item->GetPos());
				spawner->SetSpawnScale(item->GetScale());

				SceneManager::Instance().AddObject(spawner);

				item->SetSpawner(spawner);

				// Item本体はシーンに入れない（Spawnerが出すため）
				continue;
			}
		}

		SceneManager::Instance().AddObject(obj);
	}

	return true;
}

bool SceneSerializer::GetModelPaths(
	const std::string& path,
	std::vector<std::string>& outPaths)
{
	std::ifstream ifs(path);

	if (!ifs)
	{
		return false;
	}

	json root;
	ifs >> root;

	std::unordered_set<std::string> uniquePaths;

	for (auto& objData : root["Objects"])
	{
		if (!objData.contains("ModelPath"))
		{
			continue;
		}

		std::string modelPath =
			objData["ModelPath"].get<std::string>();

		uniquePaths.insert(modelPath);
	}

	outPaths.assign(
		uniquePaths.begin(),
		uniquePaths.end()
	);

	return true;
}
