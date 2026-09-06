#pragma once

//============================================================
// NavMesh Ground Tile
//============================================================
//
// NavMeshの元となるGround Tile。
// 1 Ground = 1 NavMesh Grid Cell
//
// Grid側では position を使用して
// XZ座標 → Grid座標
// Y座標 → Ground Height
// として扱う。
//
//============================================================

struct NavMeshGroundTile
{
	//============================================================
	// World Position
	//============================================================

	// Ground Tileの中心World座標
	Math::Vector3 position =
		Math::Vector3::Zero;
};
