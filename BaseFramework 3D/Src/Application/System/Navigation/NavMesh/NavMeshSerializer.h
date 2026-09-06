#pragma once

#include "NavMeshManager.h"

//============================================================
// NavMesh Serializer
//============================================================
//
// NavMeshをJSONに保存 / 読み込みする
//
// 保存内容
// ・Polygon ID
// ・3頂点
// ・Connections
//
// CenterはLoad時に再計算する
//
//============================================================

class NavMeshSerializer
{
public:

	//============================================================
	// Save
	//============================================================

	static bool Save(
		const std::string& path,
		const NavMeshManager& manager);

	//============================================================
	// Load
	//============================================================

	static bool Load(
		const std::string& path,
		NavMeshManager& manager);

	static bool LoadWithScale(
		const std::string& path,
		NavMeshManager& manager,
		float scale);
};
