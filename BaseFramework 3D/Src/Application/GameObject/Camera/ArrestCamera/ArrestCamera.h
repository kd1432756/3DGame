#pragma once
#include "../CameraBase.h"

class ArrestCamera : public CameraBase
{
public:

	enum class State
	{
		PullingOut,   // 捕まってだんだん引く（ズームアウト）
		Normal,       // 通常の連行中TPS（マウス操作可能）
		PushingIn,    // 釈放に向かってだんだん近づく（ズームイン）
	};

	ArrestCamera()							{}
	~ArrestCamera()			override	{}

	void Init()				override;
	void PostUpdate()		override;

	void SetEyeHeight(float height) override;

	// 釈放時のズームインを開始するトリガー
	void StartPushIn() { m_state = State::PushingIn; m_transitionTimer = 0.0f; }

	// 遷移が完了したかどうか（Scene側やPlayer側で判定してカメラを切り替える用）
	bool IsTransitionFinished() const { return m_isFinished; }

private:
	
	float m_height = 0.0f;

	State m_state = State::PullingOut;
	float m_transitionTimer = 0.0f;       // 遷移にかかる時間計測用
	const float m_transitionDuration = 0.5f; // ズームにかかる秒数（例: 1秒）

	bool m_isFinished = false;
};