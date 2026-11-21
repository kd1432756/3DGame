#pragma once

#include "NavMeshManager.h"

class NavMeshSerializer
{
public:

	// Save
	static bool Save(const std::string& path, const NavMeshManager& manager);

	// Load
	static bool Load(const std::string& path, NavMeshManager& manager);

	static bool LoadWithScale(const std::string& path, NavMeshManager& manager, float scale);
};
