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
	std::replace(key.begin(), key.end(), '\\', '/');   // Loadと同じ正規化

	auto& cache = Cache();

	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second.get();   // 失敗済みならnullptr
	}

	auto data = std::make_unique<ItemData>();
	if (!data->Load(key))
	{
		cache.emplace(key, nullptr);   // 失敗も記録して、毎回ファイルを開かない
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
	// [x,y,z] を安全に読む。形式が違えば既定値のまま
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
	if (str == "Pirece")  return ItemAction::Pirece;
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

	//--------------------------------------------------
	// 用途
	//--------------------------------------------------
	m_uses.clear();

	if (j.contains("Uses") && j["Uses"].is_array())
	{
		// 新形式: "Uses": [ { "Action": "...", "Target": "...", "Text": "..." } ]
		for (const auto& u : j["Uses"])
		{
			ItemUse use;
			use.action = ParseAction(u.value("Action", "None"));
			use.target = u.value("Target", "None");
			use.actionText = u.value("Text", "");
			m_uses.push_back(std::move(use));
		}
	}
	else
	{
		// 旧形式: "Action" / "Target" 単体 → Use 1件に変換
		const std::string actionStr = j.value("Action", "None");

		ItemUse use;
		use.action = ParseAction(actionStr);
		use.target = j.value("Target", "None");
		use.actionText = actionStr;	// 旧仕様: 文言にAction名をそのまま使っていた

		if (use.action != ItemAction::None)
		{
			m_uses.push_back(std::move(use));
		}
	}

	//--------------------------------------------------
	// 持ち手・スケール
	//--------------------------------------------------
	ReadVector3(j, "WorldScale", m_worldScale);
	ReadVector3(j, "HandOffset", m_handOffset);
	ReadVector3(j, "HandRotation", m_handRotation);
	ReadVector3(j, "HandScale", m_handScale);

	return true;
}