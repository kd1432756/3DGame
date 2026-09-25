#include "CameraBase.h"

#include "../../Scene/CameraManager/CameraManager.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;
}

void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	if (!target) { return; }

	m_wpTarget = target;
}

void CameraBase::UpdateRotateByMouse()
{    
	// Tabキーを押した瞬間にカメラロックを切り替える
	static bool prevTab = false;

	bool nowTab = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;

	if (nowTab && !prevTab)
	{
		m_mouseLock = !m_mouseLock;

		ShowCursor(!m_mouseLock);

		// ロック解除時にカーソルを中央へ戻す
		if (!m_mouseLock)
		{
			SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
		}
	}

	prevTab = nowTab;

	if (!m_mouseLock)
	{
		return;
	}

	// マウスでカメラを回転させる処理
	POINT _nowPos;
	GetCursorPos(&_nowPos);

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}

bool CameraBase::ConvertWorldToScreenDetail(const Math::Vector3& pos, Math::Vector3& result)
{
	// ビューポートを取得する
	Math::Viewport vp;
	KdDirect3D::Instance().CopyViewportInfo(vp);

	// ①ワールド変換行列×ビュー行列×射影行列
	Math::Matrix world = Math::Matrix::CreateTranslation(pos);
	Math::Matrix wvp =
		world *
		m_spCamera->GetCameraViewMatrix() *
		m_spCamera->GetProjMatrix();

	if (wvp._44 <= 0.0f)
	{
		return false;
	}

	// ②奥行情報(w = ._44で割る必要がある)
	wvp._41 /= wvp._44;
	wvp._42 /= wvp._44;
	wvp._43 /= wvp._44;

	// 射影行列系での2D(みたいな)座標
	// ↑-1~1の範囲の座標の事
	Math::Vector3 localPos = wvp.Translation();

	// ここで幅や高さを考慮して計算する(これで正確なスクリーン座標になる)
	result.x = localPos.x * (vp.width * 0.5f);
	result.y = localPos.y * (vp.height * 0.5f);
	result.z = wvp._44;

	return true;
}