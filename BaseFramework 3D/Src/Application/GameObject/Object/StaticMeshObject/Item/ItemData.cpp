#include "ItemData.h"

#include <json.hpp>

using json = nlohmann::json;

bool ItemData::Load(const std::string& path)
{
	std::string normalizedPath = path;

	std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

	std::ifstream ifs(normalizedPath);

	if (!ifs)
	{
		return false;
	}

	json j;
	ifs >> j;

	m_itemPath = normalizedPath;

	m_name = j.value("Name", "");

	m_modelPath = j.value("ModelPath", "");

	m_iconPath = j.value("IconPath", "");

	m_actionText = j.value("Action", "None");

	if (m_actionText == "Cut")
	{
		m_action = ItemAction::Cut;
	}
	else if (m_actionText == "Repair")
	{
		m_action = ItemAction::Repair;
	}
	else if (m_actionText == "Break")
	{
		m_action = ItemAction::Break;
	}
	else
	{
		m_action = ItemAction::None;
	}

	m_target = j.value("Target", "None");

	if (j.contains("WorldScale"))
	{
		auto& v = j["WorldScale"];

		m_worldScale =
		{
			v[0],
			v[1],
			v[2]
		};
	}

	if (j.contains("HandOffset"))
	{
		auto& v = j["HandOffset"];

		m_handOffset =
		{
			v[0],
			v[1],
			v[2]
		};
	}

	if (j.contains("HandRotation"))
	{
		auto& v = j["HandRotation"];

		m_handRotation =
		{
			v[0],
			v[1],
			v[2]
		};
	}

	if (j.contains("HandScale"))
	{
		auto& v = j["HandScale"];

		m_handScale =
		{
			v[0],
			v[1],
			v[2]
		};
	}

	return true;
}