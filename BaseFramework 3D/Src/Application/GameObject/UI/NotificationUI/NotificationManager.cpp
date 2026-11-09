#include "NotificationManager.h"
#include "NotificationUI.h"
#include "../../../Scene/SceneManager.h"

void NotificationManager::Push(const std::string& message)
{
	m_pending.push_back(message);
}

void NotificationManager::Update()
{
	// 1. 終了した通知を除去
	m_active.erase(
		std::remove_if(m_active.begin(), m_active.end(),
			[](const std::shared_ptr<NotificationUI>& ui)
			{
				return ui->IsExpired();
			}),
		m_active.end());

	// 2. 空きがあれば待機キューから出す
	while (m_active.size() < kMaxActive && !m_pending.empty())
	{
		Spawn(m_pending.front());
		m_pending.pop_front();
	}

	// 3. 並び順から目標Yを割り当てる
	ApplyStackPositions();
}

void NotificationManager::Clear()
{
	m_pending.clear();
	m_active.clear();
}

void NotificationManager::Spawn(const std::string& message)
{
	auto ui = std::make_shared<NotificationUI>();
	ui->Init();
	ui->SetText(message);
	ui->SetStartY(GetSlotY(0));	// 一番上の位置に出現

	SceneManager::Instance().AddObject(ui);

	// 先頭に挿入 → 既存の通知は1つ下にずれる
	m_active.insert(m_active.begin(), ui);
}

void NotificationManager::ApplyStackPositions()
{
	for (size_t i = 0; i < m_active.size(); ++i)
	{
		m_active[i]->SetTargetY(GetSlotY(i));
	}
}

float NotificationManager::GetSlotY(size_t index) const
{
	// Y上向きの座標系を想定。下向きなら + にしてください
	return kBaseY - kLineSpace * static_cast<float>(index);
}