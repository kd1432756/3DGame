#pragma once

class CameraBase;

class CameraManager
{
public:

	void SetCamera(const std::shared_ptr<CameraBase>& camera);

	std::shared_ptr<CameraBase> GetCamera() const
	{
		return m_spCamera;
	}

	void Update();
	void PostUpdate();
	void PreDraw();

private:
	CameraManager() = default;

	std::shared_ptr<CameraBase> m_spCamera;

public:

	static CameraManager& Instance()
	{
		static CameraManager instance;
		return instance;
	}

};