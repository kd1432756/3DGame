#pragma once

#include "../UIBase/UIBase.h"

class Player;

//==================================================
// Player を参照するUIの基底クラス
//==================================================
class PlayerUIBase : public UIBase
{
public:
	PlayerUIBase(bool alwaysUpdate = false) : UIBase(alwaysUpdate) {}
	~PlayerUIBase() override {}

	void SetPlayer(const std::shared_ptr<Player>& player) { m_wpPlayer = player; }
	void SetPlayer(const std::weak_ptr<Player>& player) { m_wpPlayer = player; }

protected:
	std::shared_ptr<Player> LockPlayer() const { return m_wpPlayer.lock(); }

private:
	std::weak_ptr<Player> m_wpPlayer;
};
