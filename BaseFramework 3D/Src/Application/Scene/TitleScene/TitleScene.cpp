#include "TitleScene.h"
#include "../SceneManager.h"

#include "../../Scene/CameraManager/CameraManager.h"

#include "../../GameObject/Camera/EventCamera/EventCamera.h"

#include "../../GameObject/UI/TitleUI/TitleUI.h"

#include "../../GameObject/SkyBox/SkyBox.h"

void TitleScene::Event()
{
	SceneManager::Instance().SetStage(SceneManager::StageType::Stage2);

	const KdShaderManager::cbLight& cbLight = KdShaderManager::Instance().GetLightCB();

	Math::Vector3 DLColor{ 2.5f,2.5f,2.5f };
	Math::Vector3 DLDir = { 0.577f,-0.577f,0.577f };
	Math::Vector4 ALColor = { 0.3f,0.3f,0.3f,1.0f };
	KdShaderManager::Instance().WorkAmbientController().SetDirLight(DLDir, DLColor);
	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(ALColor);

	if (GetAsyncKeyState(VK_F2) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Edit
		);
	}
}

void TitleScene::Init()
{
	auto camera = std::make_shared<EventCamera>();
	camera->Init();
	CameraManager::Instance().SetCamera(camera);

	auto skyBox = std::make_shared<SkyBox>();
	skyBox->Init();
	m_objList.push_back(skyBox);

	auto titleUI = std::make_shared<TitleUI>();
	titleUI->Init();
	m_objList.push_back(titleUI);
}
