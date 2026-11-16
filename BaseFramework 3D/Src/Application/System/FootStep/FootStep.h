#pragma once

#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"

class FootstepSystem
{
public:
	void Init();

	// 毎フレームリスナー位置を更新
	void SetListenerPos(const Math::Vector3& pos) { m_listenerPos = pos; }

	// 足音＋煙
	void Play(Ground::GroundType type, const Math::Vector3& footPos,
		float scale = 1.0f, bool withSmoke = true);

private:
	FootstepSystem() = default;

	struct FootstepData
	{
		std::vector<std::string> sePaths;   // 足音SE
		float seVolume = 1.0f;

		float smokeScale = 1.0f;
	};

	const FootstepData* FindData(Ground::GroundType type) const;

	void PlaySE(const FootstepData& data, const Math::Vector3& pos, float scale);
	void SpawnSmoke(const FootstepData& data, const Math::Vector3& pos, float scale);

	std::unordered_map<Ground::GroundType, FootstepData> m_table;
	FootstepData m_default;

	Math::Vector3 m_listenerPos = {};

	// 距離
	float m_seMaxDistance = 25.0f;   // これより遠い足音は鳴らさない
	float m_smokeMaxDistance = 18.0f;   // これより遠い煙は出さない

public:

	static FootstepSystem& Instance()
	{
		static FootstepSystem instance;
		return instance;
	}

};