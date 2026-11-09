#pragma once

class NotificationUI;

class NotificationManager
{
public:
	static NotificationManager& Instance()
	{
		static NotificationManager instance;
		return instance;
	}

	// 通知を追加（満杯なら待機キューへ）
	void Push(const std::string& message);

	// 毎フレーム呼ぶ
	void Update();

	// シーン開始時・終了時に呼ぶ
	void Clear();

private:
	NotificationManager() = default;
	NotificationManager(const NotificationManager&) = delete;
	NotificationManager& operator=(const NotificationManager&) = delete;

	void Spawn(const std::string& message);
	void ApplyStackPositions();
	float GetSlotY(size_t index) const;

	// ---- 調整用パラメータ ----
	static constexpr size_t kMaxActive = 3;
	static constexpr float  kBaseY = 180.0f;	// 一番上のY
	static constexpr float  kLineSpace = 80.0f;		// 行間（フォントサイズに合わせる）

	std::deque<std::string>                      m_pending;	// 表示待ち
	std::vector<std::shared_ptr<NotificationUI>> m_active;	// [0]が一番上（最新）
};