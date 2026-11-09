#pragma once

class Player;
class Neighbor;

class ItemData;

class Usable
{
public:
	virtual ~Usable() = default;

	virtual bool CanUse(const std::shared_ptr<Player>& player) = 0;
	virtual void Use(const std::shared_ptr<Player>& player) = 0;

	virtual bool CanNPCUse(
		const ItemData& item,
		const std::string& npcName) const
	{
		return false;
	}

	virtual void NPCUse(
		const ItemData& item,
		const std::shared_ptr<Neighbor>& npc)
	{}

	virtual float GetUseTime(const std::shared_ptr<Player>& player) const
	{
		return 0.0f;
	}

	virtual std::string GetActionText(
		const std::shared_ptr<Player>& player) const
	{
		return "";
	}

	void SetFocused(bool focus)
	{
		m_isFocused = focus;
	}

protected:

	bool IsFocused() const
	{
		return m_isFocused;
	}

	bool m_isFocused = false;

};