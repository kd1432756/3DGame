#pragma once

class CameraBase;

class CameraManager
{
public:

	static CameraManager& Instance()
	{
		static CameraManager instance;
		return instance;
	}

	void SetCamera(const std::shared_ptr<CameraBase>& camera);

	std::shared_ptr<CameraBase> GetCamera() const
	{
		return m_spCamera;
	}

	void Update();
	void PostUpdate();
	void PreDraw();

private:

	std::shared_ptr<CameraBase> m_spCamera;
};