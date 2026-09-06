#pragma once

#include "../CameraBase.h"

class EditorCamera : public CameraBase
{
public:
	EditorCamera() {}
	~EditorCamera() {}

	void Init() override;
	void Update(bool enableInput);
	void PreDraw() override;

	void Focus(const Math::Vector3& pos);

	float GetDistance() const
	{
		return m_distance;
	}
private:

	void UpdateOrbit(ImGuiIO& io);
	void UpdatePan(ImGuiIO& io);
	void UpdateZoom(ImGuiIO& io);
	void UpdateFly(ImGuiIO& io);
	void UpdateFocus();
	void UpdateCameraTransform();

	Math::Vector3 m_target = { 0,0,0 };

	Math::Vector3 m_targetGoal;
	bool m_isFocusing = false;

	float m_distance = 10.0f;

	float m_yaw = 0.0f;
	float m_pitch = 20.0f;
};