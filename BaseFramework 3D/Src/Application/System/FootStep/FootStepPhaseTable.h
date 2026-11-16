#pragma once

namespace FootstepPhase
{
	constexpr float kTotalFrames = 60.0f; 	// 元アニメの総フレーム数

	inline std::vector<float> FromFrames(std::initializer_list<int> frames)
	{
		std::vector<float> out;
		for (int f : frames) out.push_back(f / kTotalFrames);
		return out;
	}

	inline const std::vector<float>& Sprint()
	{
		static const auto v = FromFrames({ 0, 6, 12, 18, 24, 30, 36, 42, 48, 54 });
		return v;
	}

	inline const std::vector<float>& Walk()
	{
		static const auto v = FromFrames({ 0, 10, 20, 30, 40, 50 });
		return v;
	}
}