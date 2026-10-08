#pragma once

namespace FootstepPhase
{
	// 元アニメの総フレーム数
	constexpr float kTotalFrames = 60.0f;

	inline std::vector<float> FromFrames(std::initializer_list<int> frames)
	{
		std::vector<float> out;
		for (int f : frames) out.push_back(f / kTotalFrames);
		return out;
	}

	inline const std::vector<float>& Sprint()
	{
		static const auto v = FromFrames({ 0, 6, 13, 19, 25, 31, 37, 43, 49, 54 });
		return v;
	}

	inline const std::vector<float>& Walk()
	{
		static const auto v = FromFrames({ 0, 10, 21, 30, 41, 50 });
		return v;
	}
}