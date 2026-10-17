#include "SkyBox.h"

#include "../../System/DayCycle/DayCycle.h"

void SkyBox::Init()
{
	m_spModel[0] = std::make_shared<KdModelData>();
	m_spModel[0]->Load("Asset/Models/SkyBox/SkyBoxDay.gltf");

	m_spModel[1] = std::make_shared<KdModelData>();
	m_spModel[1]->Load("Asset/Models/SkyBox/SkyBoxEvening.gltf");

	m_spModel[2] = std::make_shared<KdModelData>();
	m_spModel[2]->Load("Asset/Models/SkyBox/SkyBoxNight.gltf");

	SetPos({0,5,0});
	SetScale(50.0f);
	SetRot({ 0,-75,0 });

	// 初期時刻に合わせてSkyBoxのアルファ値を設定
	float time = DayCycle::Instance().GetCurrentTime();

	auto CalcAlpha = [](float t, float start,
		float peakStart, float peakEnd, float end)
		{
			if (t < start || t > end) return 0.0f;
			if (t >= peakStart && t <= peakEnd) return 1.0f;

			if (t < peakStart)
				return (t - start) / (peakStart - start);

			return (end - t) / (end - peakEnd);
		};

	m_dayAlpha = CalcAlpha(time, 6.0f, 6.0f, 15.0f, 17.0f);
	m_eveningAlpha = CalcAlpha(time, 15.0f, 16.5f, 18.0f, 19.5f);
	m_nightAlpha = CalcAlpha(time, 18.0f, 19.5f, 22.0f, 22.0f);

	// 初回Updateを待たずにワールド行列も設定
	UpdateWorldMatrix();
}

void SkyBox::Update()
{
	// DayCycleから現在の時刻を取得 (6.0f ～ 22.0f)
	float time = DayCycle::Instance().GetCurrentTime();

	float normalizedTime = (time - 6.0f) / 16.0f;
	normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);

	float startDeg = -75.0f;
	float endDeg = -175.0f;

	m_rot.y = startDeg + (endDeg - startDeg) * normalizedTime;

	// 修正したCalcAlpha関数（安全に補間を行えるように調整）
	auto CalcAlpha = [](float t, float start, float peakStart, float peakEnd, float end) {
		if (t < start || t > end) return 0.0f;
		if (t >= peakStart && t <= peakEnd) return 1.0f;
		if (t < peakStart) {
			return (t - start) / (peakStart - start);
		}
		return (end - t) / (end - peakEnd);
		};

	// --- 常に背景が透けないように時間が連続して繋がるように調整 ---
	// Day     : 6:00 ～ 17:00 (完全にDayなのは 6:00～15:00)
	// Evening : 15:00 ～ 19:30 (完全にEveningなのは 16:30～18:00)
	// Night   : 18:00 ～ 22:00 (完全にNightなのは 19:30～22:00)

	m_dayAlpha = CalcAlpha(time, 6.0f, 6.0f, 15.0f, 17.0f);
	m_eveningAlpha = CalcAlpha(time, 15.0f, 16.5f, 18.0f, 19.5f);
	m_nightAlpha = CalcAlpha(time, 18.0f, 19.5f, 22.0f, 22.0f);

	UpdateWorldMatrix();
}

void SkyBox::DrawUnLit()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	KdShaderManager::Instance().ChangeDepthStencilState(KdDepthStencilState::ZWriteDisable);


	// Day
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel[0],
		m_mWorld,
		Math::Vector4(1, 1, 1, m_dayAlpha),
		Math::Vector3(1, 1, 1)
	);

	// Evening
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel[1],
		m_mWorld,
		Math::Vector4(1, 1, 1, m_eveningAlpha),
		Math::Vector3(1, 1, 1)
	);

	// Night
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel[2],
		m_mWorld,
		Math::Vector4(1, 1, 1, m_nightAlpha),
		Math::Vector3(1, 1, 1)
	);

	KdShaderManager::Instance().UndoBlendState();
	KdShaderManager::Instance().UndoDepthStencilState();
}
