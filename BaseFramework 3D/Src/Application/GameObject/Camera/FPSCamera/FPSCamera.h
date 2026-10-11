#pragma once
#include "../CameraBase.h"

class FPSCamera : public CameraBase
{
public:
	FPSCamera()							{}
	~FPSCamera()			override	{}

	void Init()				override;
	void Update()		override;

	void SetEyeHeight(float height);

private:

	float m_smoothY = 0.0f;       // 平滑化したY座標
	bool  m_smoothYInit = false;  // 初回は補間せず即座に合わせる

	// 追従の速さ（大きいほど速い）。8〜15くらいが目安
	float m_ySmoothSpeed = 10.0f;
};