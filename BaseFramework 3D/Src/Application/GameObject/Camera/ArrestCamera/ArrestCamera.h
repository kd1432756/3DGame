#pragma once
#include "../CameraBase.h"

class ArrestCamera : public CameraBase
{
public:

	enum class State
	{
		PullingOut,		// ズームアウト
		Normal,			// マウス操作可能
		PushingIn,		// ズームイン
	};

	ArrestCamera()							{}
	~ArrestCamera()			override	{}

	void Init()				override;
	void PostUpdate()		override;

	void SetEyeHeight(float height) override;

	// 釈放時のズームインを開始
	void StartPushIn()
	{
		m_state = State::PushingIn;
		m_transitionTimer = 0.0f;
		m_isFinished = false;
	}

	// 遷移が完了したかどうか
	bool IsTransitionFinished() const
	{
		return m_isFinished;
	}

private:
	
	float m_height = 0.0f;

	State m_state = State::PullingOut;
	float m_transitionTimer = 0.0f;	 // 遷移にかかる時間計測用
	static constexpr float kTransitionDuration = 0.5f;		// ズームにかかる秒数

	bool m_isFinished = false;
};