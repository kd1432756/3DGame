#include "DayCycle.h"

void DayCycle::Init()
{
	m_currentTime = StartTime;

	m_timeScale = 3.0f;

	m_timeZone = TimeZone::Day;

	UpdateLighting();
}

void DayCycle::Update()
{
	// 現実1秒 = ゲーム内1分
	m_currentTime += 1.0f / 60.0f / 60.0f * m_timeScale;

	// 22:00を超えたら終了
	if (m_currentTime >= EndTime)
	{
		m_currentTime = EndTime;
	}

	UpdateTimeZone();
	UpdateLighting();
}


void DayCycle::AddTime(float time)
{
	m_currentTime += time;

	if (m_currentTime >= EndTime)
	{
		m_currentTime = EndTime;
	}

	UpdateTimeZone();
	UpdateLighting();
}

void DayCycle::UpdateTimeZone()
{
	if (m_currentTime >= 6.0f && m_currentTime < 17.0f)
	{
		m_timeZone = TimeZone::Day;
	}
	else if (m_currentTime >= 17.0f && m_currentTime < 20.0f)
	{
		m_timeZone = TimeZone::Evening;
	}
	else
	{
		m_timeZone = TimeZone::Night;
	}
}

void DayCycle::UpdateLighting()
{
	auto& ambientController = KdShaderManager::Instance().WorkAmbientController();

	//------------------------------------------------------------
	// 太陽の位置
	//------------------------------------------------------------

	float normalizedTime = (m_currentTime - 6.0f) / 16.0f;

	normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);

	float angle = normalizedTime * DirectX::XM_PI;

	const float sunHeightScale = 2.5f;
	const float sunHeightBias = 0.6f;  // 最低限の高さ

	float x = cosf(angle);
	float y = -(sinf(angle) * sunHeightScale + sunHeightBias);
	float z = 1.0f;

	DirectX::XMFLOAT3 sunDirection =
	{
		x,
		y,
		z
	};

	//------------------------------------------------------------
	// 光の色・強さ
	//------------------------------------------------------------

	DirectX::XMFLOAT3 dayLight =
	{
		2.25f,
		2.25f,
		2.25f
	};

	DirectX::XMFLOAT3 eveningLight =
	{
		1.8f,
		1.2f,
		0.8f
	};

	DirectX::XMFLOAT3 nightLight =
	{
		1.0f,
		1.0f,
		1.1f
	};

	//------------------------------------------------------------
	// Ambient Light
	//------------------------------------------------------------

	DirectX::XMFLOAT4 dayAmbient =
	{
		0.45f,
		0.45f,
		0.45f,
		1.0f
	};

	DirectX::XMFLOAT4 eveningAmbient =
	{
		0.30f,
		0.24f,
		0.20f,
		1.0f
	};

	DirectX::XMFLOAT4 nightAmbient =
	{
		0.18f,
		0.18f,
		0.22f,
		1.0f
	};

	//------------------------------------------------------------
	// 時間に応じて補間
	//------------------------------------------------------------

	const float redStartTime = 15.0f;			// 赤くなり始める
	const float eveningStartTime = 16.5f;		// 夕方の光が完全になる
	const float eveningEndTime = 18.0f;		// 夕方の光が終わり始める
	const float nightFullTime = 19.5f;			// 夜の光が完全になる

	DirectX::XMFLOAT3 lightColor;
	DirectX::XMFLOAT4 ambientColor;

	// 2色の補間
	auto LerpColor3 = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t)
		{
			return DirectX::XMFLOAT3
			{
				a.x + (b.x - a.x) * t,
				a.y + (b.y - a.y) * t,
				a.z + (b.z - a.z) * t
			};
		};
	auto LerpColor4 = [](const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t)
		{
			return DirectX::XMFLOAT4
			{
				a.x + (b.x - a.x) * t,
				a.y + (b.y - a.y) * t,
				a.z + (b.z - a.z) * t,
				1.0f
			};
		};

	if (m_currentTime < redStartTime)
	{
		// 6:00 ～ 15:00 : 昼の光のまま
		lightColor = dayLight;
		ambientColor = dayAmbient;
	}
	else if (m_currentTime < eveningStartTime)
	{
		// 15:00 ～ 16:30 : 昼 → 夕方
		float t = (m_currentTime - redStartTime) / (eveningStartTime - redStartTime);
		t = std::clamp(t, 0.0f, 1.0f);

		lightColor = LerpColor3(dayLight, eveningLight, t);
		ambientColor = LerpColor4(dayAmbient, eveningAmbient, t);
	}
	else if (m_currentTime < eveningEndTime)
	{
		// 16:30 ～ 18:00 : 夕方の光のまま
		lightColor = eveningLight;
		ambientColor = eveningAmbient;
	}
	else
	{
		// 18:00 ～ 19:30 : 夕方 → 夜（19:30以降は夜のまま）
		float t = (m_currentTime - eveningEndTime) / (nightFullTime - eveningEndTime);
		t = std::clamp(t, 0.0f, 1.0f);

		lightColor = LerpColor3(eveningLight, nightLight, t);
		ambientColor = LerpColor4(eveningAmbient, nightAmbient, t);
	}

	//------------------------------------------------------------
	// シェーダーに反映
	//------------------------------------------------------------

	ambientController.SetDirLight(sunDirection, lightColor);
	ambientController.SetAmbientLight(ambientColor);
}