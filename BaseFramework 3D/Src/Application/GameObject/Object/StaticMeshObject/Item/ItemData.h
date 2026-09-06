#pragma once

enum class ItemAction
{
	None,
	Repair,
	Cut,
	Break
};

class ItemData
{
public:

	bool Load(const std::string& path);

	const std::string& GetName() const
	{
		return m_name;
	}

	const std::string& GetModelPath() const
	{
		return m_modelPath;
	}

	const std::string& GetIconPath() const
	{
		return m_iconPath;
	}

	ItemAction GetAction() const
	{
		return m_action;
	}

	const std::string& GetActionText() const
	{
		return m_actionText;
	}

	const std::string& GetTarget() const
	{
		return m_target;
	}

	const std::string& GetItemPath() const
	{
		return m_itemPath;
	}

	const Math::Vector3& GetWorldScale() const
	{
		return m_worldScale;
	}

	const Math::Vector3& GetHandOffset() const
	{
		return m_handOffset;
	}

	const Math::Vector3& GetHandRotation() const
	{
		return m_handRotation;
	}

	const Math::Vector3& GetHandScale() const
	{
		return m_handScale;
	}

	int GetMaxCount() const
	{
		return m_maxCount;
	}

private:

	std::string m_itemPath;

	std::string m_name;

	std::string m_modelPath;

	std::string m_iconPath;

	ItemAction m_action = ItemAction::None;

	std::string m_actionText;

	std::string m_target;

	Math::Vector3 m_worldScale = { 1,1,1 };

	Math::Vector3 m_handOffset = Math::Vector3::Zero;
	Math::Vector3 m_handRotation = Math::Vector3::Zero;
	Math::Vector3 m_handScale = { 1,1,1 };

	int m_maxCount = 1;
};