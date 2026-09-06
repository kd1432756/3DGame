#include "CameraManager.h"

#include "../../GameObject/Camera/CameraBase.h"

#include "../../GameObject/Camera/FPSCamera/FPSCamera.h"

void CameraManager::SetCamera(
	const std::shared_ptr<CameraBase>& camera)
{
	m_spCamera = camera;
}

void CameraManager::Update()
{
	if (!m_spCamera) return;

	m_spCamera->Update();
}

void CameraManager::PostUpdate()
{
	if (!m_spCamera) return;

	m_spCamera->PostUpdate();
}

void CameraManager::PreDraw()
{
	if (!m_spCamera) return;

	m_spCamera->PreDraw();
}