#include "ItemData.h"

#include <json.hpp>

namespace
{
	std::unordered_map<std::string, std::unique_ptr<ItemData>>& Cache()
	{
		static std::unordered_map<std::string, std::unique_ptr<ItemData>> s_cache;
		return s_cache;
	}
}

const ItemData* ItemData::Get(const std::string& path)
{
	std::string key = path;
	std::replace(key.begin(), key.end(), '\\', '/');   // パスの正規化

	auto& cache = Cache();

	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second.get();
	}

	auto data = std::make_unique<ItemData>();
	if (!data->Load(key))
	{
		cache.emplace(key, nullptr);
		return nullptr;
	}

	const ItemData* p = data.get();
	cache.emplace(key, std::move(data));
	return p;
}

void ItemData::ClearCache() { Cache().clear(); }
using json = nlohmann::json;

namespace
{
	// [x,y,z] 座標を読む
	bool ReadVector3(const json& j, const char* key, Math::Vector3& out)
	{
		auto it = j.find(key);
		if (it == j.end() || !it->is_array() || it->size() < 3)
		{
			return false;
		}

		out = { (*it)[0].get<float>(), (*it)[1].get<float>(), (*it)[2].get<float>() };
		return true;
	}
}

ItemAction ItemData::ParseAction(const std::string& str)
{
	if (str == "Repair") return ItemAction::Repair;
	if (str == "Cut")    return ItemAction::Cut;
	if (str == "Break")  return ItemAction::Break;
	if (str == "Wrench")  return ItemAction::Wrench;
	if (str == "Pierce")  return ItemAction::Pierce;
	return ItemAction::None;
}

bool ItemData::Load(const std::string& path)
{
	*this = ItemData();   // 前回の値を持ち越さない

	std::string normalizedPath = path;
	std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

	std::ifstream ifs(normalizedPath);
	if (!ifs)
	{
		return false;
	}

	json j;
	try
	{
		ifs >> j;
	}
	catch (const json::exception&)
	{
		return false;
	}

	m_itemPath = normalizedPath;
	m_name = j.value("Name", "");
	m_modelPath = j.value("ModelPath", "");
	m_iconPath = j.value("IconPath", "");
	m_maxCount = j.value("MaxCount", -1);
	m_dangerous = j.value("Dangerous", false);

	m_uses.clear();

	if (j.contains("Uses") && j["Uses"].is_array())
	{
		for (const auto& u : j["Uses"])
		{
			ItemUse use;
			use.action = ParseAction(u.value("Action", "None"));
			use.target = u.value("Target", "None");
			use.actionText = u.value("Text", "");
			m_uses.push_back(std::move(use));
		}
	}

	m_worldScale = {
		j["WorldScale"][0].get<float>(),
		j["WorldScale"][1].get<float>(),
		j["WorldScale"][2].get<float>()
	};

	m_handOffset = {
		j["HandOffset"][0].get<float>(),
		j["HandOffset"][1].get<float>(),
		j["HandOffset"][2].get<float>()
	};

	m_handRotation = {
		j["HandRotation"][0].get<float>(),
		j["HandRotation"][1].get<float>(),
		j["HandRotation"][2].get<float>()
	};

	m_handScale = {
		j["HandScale"][0].get<float>(),
		j["HandScale"][1].get<float>(),
		j["HandScale"][2].get<float>()
	};

	return true;
}