#pragma once
#include "IEditorMode.h"
#include "../Windows/StageEditorWindow.h"
#include <Application/GameObject/Object/StaticMeshObject/Ground/Ground.h>

class StageMode : public IEditorMode
{
public:
	StageMode() { m_window.Init(); }

	void ApplyLayout(EditorContext&) override;
	void Update(EditorContext&, EditorIO&) override;
	void DrawWindows(EditorContext&) override;
	void DrawDebug(EditorContext&) override;
	void OnSceneClick(EditorContext&, const Math::Vector3& pos) override;

private:
	struct GridCoord
	{
		int x, y, z;
		// 構造体同士を比較できる
		bool operator==(const GridCoord& other) const
		{
			return x == other.x
				&& y == other.y
				&& z == other.z;
		}
	};

	static Math::Vector3 Snap(const Math::Vector3& pos);
	static GridCoord ToGrid(const Math::Vector3& pos);

	std::shared_ptr<Ground> FindGroundAt(const GridCoord& grid) const;

	void PlaceTile(const Math::Vector3& snapped);
	void EraseTile(const Math::Vector3& snapped);

	void DrawPlacePreview();
	void DrawErasePreview();

	StageEditorWindow m_window;

	std::shared_ptr<KdModelWork> m_previewModel;
	std::string m_previewPath;
	Math::Vector3 m_previewPos;

	KdDebugWireFrame m_wire;
};