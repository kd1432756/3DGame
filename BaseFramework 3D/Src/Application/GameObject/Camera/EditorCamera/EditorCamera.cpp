/*#include "EditorCamera.h"

void EditorCamera::Init()
{
	CameraBase::Init();

	m_target = { 0,0,0 };

	m_distance = 10.0f;

	m_yaw = 0;
	m_pitch = 20;

	UpdateWorldMatrix();
}

void EditorCamera::Update(bool enableInput)
{
	ImGuiIO& io = ImGui::GetIO();

	if (enableInput)
	{
		// Alt + 左ドラッグ
		if (io.KeyAlt && ImGui::IsMouseDragging(0))
		{
			m_yaw += io.MouseDelta.x * 0.3f;
			m_pitch += io.MouseDelta.y * 0.3f;

			m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
		}

		// ホイール
		m_distance -= io.MouseWheel;

		m_distance = std::clamp(m_distance, 1.0f, 100.0f);

		// 中ドラッグ
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			Math::Matrix rot =
				Math::Matrix::CreateFromYawPitchRoll(
					DirectX::XMConvertToRadians(m_yaw),
					DirectX::XMConvertToRadians(m_pitch),
					0);

			Math::Vector3 right = rot.Right();
			Math::Vector3 up = rot.Up();

			Math::Vector3 forward = rot.Forward();

			float speed = 0.01f * m_distance;

			m_target -= right * io.MouseDelta.x * speed;
			m_target += up * io.MouseDelta.y * speed;
		}

		// 右ドラッグ
		if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			m_yaw += io.MouseDelta.x * 0.3f;
			m_pitch += io.MouseDelta.y * 0.3f;

			m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

			float moveSpeed = 0.02f * m_distance;

			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (ImGui::IsKeyDown(ImGuiKey_W))
					m_target += forward * moveSpeed;

				if (ImGui::IsKeyDown(ImGuiKey_S))
					m_target -= forward * moveSpeed;

				if (ImGui::IsKeyDown(ImGuiKey_A))
					m_target -= right * moveSpeed;

				if (ImGui::IsKeyDown(ImGuiKey_D))
					m_target += right * moveSpeed;

				if (ImGui::IsKeyDown(ImGuiKey_Q))
					m_target -= up * moveSpeed;

				if (ImGui::IsKeyDown(ImGuiKey_E))
					m_target += up * moveSpeed;
			}
		}
	}

	Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_yaw),
			DirectX::XMConvertToRadians(m_pitch),
			0);

	Math::Vector3 offset = { 0,0,-m_distance };

	offset = Math::Vector3::Transform(offset, rot);

	SetPos(m_target + offset);
	SetRot({ m_pitch,m_yaw,0 });
}

void EditorCamera::PreDraw()
{
	CameraBase::PreDraw();
}*/

#include "EditorCamera.h"

void EditorCamera::Init()
{
	CameraBase::Init();

	m_target = { 0,0,0 };

	m_distance = 10.0f;

	m_yaw = 0.0f;
	m_pitch = 20.0f;

	UpdateWorldMatrix();
}

void EditorCamera::Update(bool enableInput)
{
	if (enableInput)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Fly中はFlyのみ
		if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			UpdateFly(io);
		}
		else
		{
			UpdateOrbit(io);
			UpdatePan(io);
			UpdateZoom(io);
		}
	}

	UpdateFocus();

	UpdateCameraTransform();
}

void EditorCamera::UpdateOrbit(ImGuiIO& io)
{
	if (!(io.KeyAlt && ImGui::IsMouseDragging(ImGuiMouseButton_Left)))
		return;

	m_yaw += io.MouseDelta.x * 0.3f;
	m_pitch += io.MouseDelta.y * 0.3f;

	m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void EditorCamera::UpdatePan(ImGuiIO& io)
{
	if (!ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		return;

	Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_yaw),
			DirectX::XMConvertToRadians(m_pitch),
			0);

	Math::Vector3 right = rot.Right();
	Math::Vector3 up = rot.Up();

	float speed = 0.01f * m_distance;

	m_target -= right * io.MouseDelta.x * speed;
	m_target += up * io.MouseDelta.y * speed;
}

void EditorCamera::UpdateZoom(ImGuiIO& io)
{
	m_distance -= io.MouseWheel;

	m_distance = std::clamp(m_distance, 1.0f, 100.0f);
}

void EditorCamera::UpdateFly(ImGuiIO& io)
{
	m_yaw += io.MouseDelta.x * 0.3f;
	m_pitch += io.MouseDelta.y * 0.3f;

	m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

	Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_yaw),
			DirectX::XMConvertToRadians(m_pitch),
			0);

	Math::Vector3 forward = rot.Forward();
	Math::Vector3 right = rot.Right();
	Math::Vector3 up = rot.Up();

	float moveSpeed = 0.02f * m_distance;

	if (ImGui::IsKeyDown(ImGuiKey_W))
		m_target -= forward * moveSpeed;

	if (ImGui::IsKeyDown(ImGuiKey_S))
		m_target += forward * moveSpeed;

	if (ImGui::IsKeyDown(ImGuiKey_A))
		m_target -= right * moveSpeed;

	if (ImGui::IsKeyDown(ImGuiKey_D))
		m_target += right * moveSpeed;

	if (ImGui::IsKeyDown(ImGuiKey_Q))
		m_target -= up * moveSpeed;

	if (ImGui::IsKeyDown(ImGuiKey_E))
		m_target += up * moveSpeed;
}

void EditorCamera::UpdateFocus()
{
	if (!m_isFocusing)
		return;

	m_target = Math::Vector3::Lerp(
		m_target,
		m_targetGoal,
		0.15f);

	if ((m_targetGoal - m_target).Length() < 0.05f)
	{
		m_target = m_targetGoal;
		m_isFocusing = false;
	}
}

void EditorCamera::UpdateCameraTransform()
{
	Math::Matrix rot =
		Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(m_yaw),
			DirectX::XMConvertToRadians(m_pitch),
			0);

	Math::Vector3 offset = { 0,0,-m_distance };

	offset = Math::Vector3::Transform(offset, rot);

	SetPos(m_target + offset);
	SetRot({ m_pitch, m_yaw, 0 });
}

void EditorCamera::Focus(const Math::Vector3& pos)
{
	m_targetGoal = pos;
	m_isFocusing = true;
}

void EditorCamera::PreDraw()
{
	CameraBase::PreDraw();
}