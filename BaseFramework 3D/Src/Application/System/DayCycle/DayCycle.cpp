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
	// 60秒でゲーム内1時間進む
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
	// timeは「ゲーム内時間」
	// 0.5f = 30分
	// 1.0f = 1時間
	m_currentTime += time;

	// 22:00を超えないようにする
	if (m_currentTime >= EndTime)
	{
		m_currentTime = EndTime;
	}

	UpdateTimeZone();
	UpdateLighting();
}

void DayCycle::UpdateTimeZone()
{
	if (m_currentTime >= 6.0f &&
		m_currentTime < 17.0f)
	{
		m_timeZone = TimeZone::Day;
	}
	else if (m_currentTime >= 17.0f &&
		m_currentTime < 20.0f)
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
	auto& ambientController =
		KdShaderManager::Instance().WorkAmbientController();

	//------------------------------------------------------------
	// ① 太陽の位置
	//
	// 6:00  → 太陽が低い
	// 14:00 → 太陽が高い
	// 22:00 → 太陽が沈む
	//------------------------------------------------------------

	float normalizedTime =
		(m_currentTime - 6.0f) / 16.0f;

	normalizedTime =
		std::clamp(normalizedTime, 0.0f, 1.0f);


	// 6:00 ～ 22:00を180度回転させる
	float angle =
		normalizedTime * DirectX::XM_PI;


	float x = cosf(angle);
	float y = -sinf(angle);
	float z = 1.0f;


	DirectX::XMFLOAT3 sunDirection =
	{
		x,
		y,
		z
	};


	//------------------------------------------------------------
	// ② 光の色・強さ
	//
	// 昼    → 明るい白
	// 夕方  → 少し暖色
	// 夜    → 少し青みを残して明るめ
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
	// ③ Ambient Light
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
	// ④ 時間に応じて補間
	//------------------------------------------------------------

	DirectX::XMFLOAT3 lightColor;

	DirectX::XMFLOAT4 ambientColor;


	if (m_currentTime < 17.0f)
	{
		// 6:00 ～ 17:00
		//
		// 昼の光 → 夕方の光

		float t =
			(m_currentTime - 6.0f) / 11.0f;

		t = std::clamp(t, 0.0f, 1.0f);


		lightColor =
		{
			dayLight.x +
			(eveningLight.x - dayLight.x) * t,

			dayLight.y +
			(eveningLight.y - dayLight.y) * t,

			dayLight.z +
			(eveningLight.z - dayLight.z) * t
		};


		ambientColor =
		{
			dayAmbient.x +
			(eveningAmbient.x - dayAmbient.x) * t,

			dayAmbient.y +
			(eveningAmbient.y - dayAmbient.y) * t,

			dayAmbient.z +
			(eveningAmbient.z - dayAmbient.z) * t,

			1.0f
		};
	}
	else
	{
		// 17:00 ～ 22:00
		//
		// 夕方の光 → 夜の光

		float t =
			(m_currentTime - 17.0f) / 5.0f;

		t = std::clamp(t, 0.0f, 1.0f);


		lightColor =
		{
			eveningLight.x +
			(nightLight.x - eveningLight.x) * t,

			eveningLight.y +
			(nightLight.y - eveningLight.y) * t,

			eveningLight.z +
			(nightLight.z - eveningLight.z) * t
		};


		ambientColor =
		{
			eveningAmbient.x +
			(nightAmbient.x - eveningAmbient.x) * t,

			eveningAmbient.y +
			(nightAmbient.y - eveningAmbient.y) * t,

			eveningAmbient.z +
			(nightAmbient.z - eveningAmbient.z) * t,

			1.0f
		};
	}


	//------------------------------------------------------------
	// ⑤ シェーダーに反映
	//------------------------------------------------------------

	ambientController.SetDirLight(
		sunDirection,
		lightColor
	);


	ambientController.SetAmbientLight(
		ambientColor
	);
}