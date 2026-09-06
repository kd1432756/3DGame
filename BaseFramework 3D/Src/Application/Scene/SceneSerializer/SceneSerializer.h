#pragma once

class SceneSerializer
{
public:
	static bool Save(const std::string& path);

	static bool Load(const std::string& path);

	static bool LoadWithScale(const std::string& path, float scale);

	static bool GetModelPaths(
		const std::string& path,
		std::vector<std::string>& outPaths
	);
};