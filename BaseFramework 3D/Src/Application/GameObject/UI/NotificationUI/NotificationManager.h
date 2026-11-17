#pragma once

class NotificationUI;

class NotificationManager
{
public:
	// 通知を追加
	void Push(const std::string& message);

	void Update();

	void Clear();

private:
	NotificationManager() = default;

	void Spawn(const std::string& message);
	void ApplyStackPositions();
	float GetSlotY(size_t index) const;

	static constexpr size_t kMaxActive = 3;
	static constexpr float  kBaseY = 180.0f;	// 一番上の座標
	static constexpr float  kLineSpace = 80.0f;

	std::deque<std::string> m_pending;	// 表示待ち
	std::vector<std::shared_ptr<NotificationUI>> m_active;	// [0]が一番上

public:
	static NotificationManager& Instance()
	{
		static NotificationManager instance;
		return instance;
	}
};