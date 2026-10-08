#pragma once

#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"

class FootstepSystem
{
public:
	// 素材ごとの設定を登録（起動時に1回）
	void Init();

	// 毎フレーム、リスナー（プレイヤー/カメラ）位置を更新
	void SetListenerPos(const Math::Vector3& pos) { m_listenerPos = pos; }

	// 足音＋煙を発火
	// scale: キャラの大きさやシーンスケール（m_sceneScale を渡す）
	void Play(Ground::GroundType type, const Math::Vector3& footPos,
		float scale = 1.0f, bool withSmoke = true);

private:
	FootstepSystem() = default;

	struct FootstepData
	{
		std::vector<std::string> sePaths;   // 足音SE（複数ならランダム）
		float seVolume = 1.0f;

		float smokeScale = 1.0f;
	};

	const FootstepData* FindData(Ground::GroundType type) const;

	void PlaySE(const FootstepData& data, const Math::Vector3& pos, float scale);
	void SpawnSmoke(const FootstepData& data, const Math::Vector3& pos, float scale);

	std::unordered_map<Ground::GroundType, FootstepData> m_table;
	FootstepData m_default;   // 未登録素材（None含む）用

	Math::Vector3 m_listenerPos = {};

	// 距離による制御（シーンスケール前の値）
	float m_seMaxDistance = 25.0f;   // これより遠い足音は鳴らさない
	float m_smokeMaxDistance = 18.0f;   // これより遠い煙は出さない

public:

	static FootstepSystem& Instance()
	{
		static FootstepSystem instance;
		return instance;
	}

};