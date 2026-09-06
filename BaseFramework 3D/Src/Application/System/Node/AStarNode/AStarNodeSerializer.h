#pragma once

#include "AStarNodeManager.h"

class AStarNodeSerializer
{
public:

	//============================================================
	// Save
	//============================================================

	static bool Save(
		const std::string& path,
		const AStarNodeManager& manager);

	//============================================================
	// Load
	//============================================================

	static bool Load(
		const std::string& path,
		AStarNodeManager& manager);

	static bool LoadWithScale(
		const std::string& path,
		AStarNodeManager& manager,
		float scale);
};
