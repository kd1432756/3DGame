#pragma once

#include "../CameraBase.h"

class EventCamera : public CameraBase
{
public:
	EventCamera() {}
	~EventCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;
};