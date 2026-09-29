#pragma once

class DayCycle
{
public:

	void Init();
	void Update();

	enum class TimeZone
	{
		Day,
		Evening,
		Night
	};

	void AddTime(float time);

	float GetCurrentTime() const
	{
		return m_currentTime;
	}

	float GetTimeProgress() const
	{
		// 6:00 ～ 22:00 を 0.0 ～ 1.0 に変換
		float progress =
			(m_currentTime - StartTime) / (EndTime - StartTime);

		return std::clamp(progress, 0.0f, 1.0f);
	}

	TimeZone GetTimeZone() const
	{
		return m_timeZone;
	}

	bool IsFinished() const
	{
		return m_currentTime >= EndTime;
	}

private:
	void UpdateTimeZone();
	void UpdateLighting();

private:
	float m_currentTime = StartTime;

	float m_timeScale = 3.0f;

	TimeZone m_timeZone = TimeZone::Day;

	static constexpr float StartTime = 6.0f;
	static constexpr float EndTime = 22.0f;

public:

	static DayCycle& Instance()
	{
		static DayCycle instance;
		return instance;
	}
};