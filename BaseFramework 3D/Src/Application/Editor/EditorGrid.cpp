#include "EditorGrid.h"
#include "../GameObject/Camera/EditorCamera/EditorCamera.h"

void EditorGrid::Draw(EditorCamera& camera)
{
	const Math::Vector3 camPos = camera.GetPos();
	const float distance = camera.GetDistance();

	// 段階的にグリッドの間隔を変える
	std::array<float, 4> gridLevels = { 1, 5, 10, 50 };
	for (float size : gridLevels)
	{
		DrawLevel(camPos, distance, size);
	}

	m_wire.Draw();
}

void EditorGrid::DrawLevel(const Math::Vector3& camPos, float distance, float gridSize)
{
	constexpr float kScreenHeight = 720.0f;
	const float pixelSize = gridSize / distance * kScreenHeight;

	constexpr float kMinPixelSize = 12.0f;
	constexpr float kMaxPixelSize = 80.0f;

	// 小さい、大きいグリッドは描かない
	if (pixelSize < kMinPixelSize || pixelSize > kMaxPixelSize)
	{
		return;
	}

	const float alpha = std::clamp((pixelSize - kMinPixelSize) / 20.0f, 0.0f, 1.0f);

	const float centerX = std::floor(camPos.x / gridSize) * gridSize;
	const float centerZ = std::floor(camPos.z / gridSize) * gridSize;

	// 150本分の半分の幅
	constexpr int halfLine = 150;

	const float minX = centerX - halfLine * gridSize;
	const float maxX = centerX + halfLine * gridSize;

	const float minZ = centerZ - halfLine * gridSize;
	const float maxZ = centerZ + halfLine * gridSize;

	const Math::Color gridColor(0.45f, 0.45f, 0.45f, alpha);
	const Math::Color axisX(1.0f, 0.0f, 0.0f, 1.0f);	// X軸
	const Math::Color axisZ(0.0f, 0.5f, 1.0f, 1.0f);	// Z軸

	for (int i = -halfLine; i <= halfLine; ++i)
	{
		const float x = centerX + i * gridSize;
		const float z = centerZ + i * gridSize;

		// X固定
		m_wire.AddDebugLine(
			{ x, 0, minZ },
			{ x, 0, maxZ },
			(std::fabs(x) < 0.001f) ? axisX : gridColor);

		// Z固定
		m_wire.AddDebugLine(
			{ minX, 0, z },
			{ maxX, 0, z },
			(std::fabs(z) < 0.001f) ? axisZ : gridColor);
	}
}