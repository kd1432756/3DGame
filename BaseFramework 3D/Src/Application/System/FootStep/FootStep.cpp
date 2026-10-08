#include "Footstep.h"

void FootstepSystem::Init()
{
	using GT = Ground::GroundType;

	// ---- 既定（None など）----
	m_default.sePaths = { "Asset/Sound/Footstep/default_01.wav",
							"Asset/Sound/Footstep/default_02.wav" };
	m_default.seVolume = 0.5f;

	// ---- 道路（アスファルト）----
	{
		FootstepData d;
		d.sePaths = { "Asset/Sound/Footstep/road_01.wav",
						"Asset/Sound/Footstep/road_02.wav" };
		d.seVolume = 0.5f;
		d.smokeScale = 0.6f;
		m_table[GT::Road] = d;
	}

	// ---- 草 ----
	{
		FootstepData d;
		d.sePaths = { "Asset/Sound/Footstep/grass_01.wav",
						"Asset/Sound/Footstep/grass_02.wav" };
		d.seVolume = 0.4f;
		d.smokeScale = 0.8f;
		m_table[GT::Grass] = d;
	}

	// ---- 土（砂埃）----
	{
		FootstepData d;
		d.sePaths = { "Asset/Sound/Footstep/dirt_01.wav",
						 "Asset/Sound/Footstep/dirt_02.wav" };
		d.seVolume = 0.5f;
		d.smokeScale = 1.2f;
		m_table[GT::Dirt] = d;
	}

	// ---- 空き地（Lot）----
	{
		FootstepData d;
		d.sePaths = { "Asset/Sound/Footstep/gravel_01.wav",
						 "Asset/Sound/Footstep/gravel_02.wav" };
		d.seVolume = 0.5f;
		d.smokeScale = 0.6f;
		m_table[GT::Lot] = d;
	}

	// ---- コンクリート ----
	{
		FootstepData d;
		d.sePaths = { "Asset/Sound/Footstep/concrete_01.wav",
						 "Asset/Sound/Footstep/concrete_02.wav" };
		d.seVolume = 0.55f;
		d.smokeScale = 0.6f;               // 小さめの砂埃
		m_table[GT::Concrete] = d;
	}
}

const FootstepSystem::FootstepData* FootstepSystem::FindData(Ground::GroundType type) const
{
	auto it = m_table.find(type);
	return (it != m_table.end()) ? &it->second : &m_default;
}

void FootstepSystem::Play(Ground::GroundType type, const Math::Vector3& footPos, float scale)
{
	const FootstepData* data = FindData(type);
	if (!data) return;

	// リスナーからの距離で判定（遠いキャラは何もしない＝負荷削減）
	const float distSq = (footPos - m_listenerPos).LengthSquared();
	const float seMax = m_seMaxDistance * scale;
	const float smokeMax = m_smokeMaxDistance * scale;

	if (distSq <= seMax * seMax)
	{
		PlaySE(*data, footPos, scale);
	}

	if (distSq <= smokeMax * smokeMax)
	{
		SpawnSmoke(*data, footPos, scale);
	}
}

void FootstepSystem::PlaySE(const FootstepData& data, const Math::Vector3& pos, float scale)
{
	if (data.sePaths.empty()) return;

	// 同じ音の連続を避けるためランダム選択
	const size_t idx = static_cast<size_t>(rand()) % data.sePaths.size();
	const std::string& path = data.sePaths[idx];

	// ★ここをエンジンのサウンドAPIに差し替え★
	// KdFramework の場合の例（関数名・引数はプロジェクトの実装に合わせて確認）:
	//
	//   auto inst = KdAudioManager::Instance().Play3D(path, pos, false);
	//   if (inst)
	//   {
	//       inst->SetVolume(data.seVolume);
	//       // 距離減衰を調整したい場合
	//       // inst->SetCurveDistanceScaler(scale);
	//   }
	//
	(void)path; (void)pos; (void)scale;
}

void FootstepSystem::SpawnSmoke(const FootstepData& data, const Math::Vector3& pos, float scale)
{
	Math::Vector3 spawnPos = pos + Math::Vector3{ 0,scale * -0.7f,0 };
	KdEffekseerManager::GetInstance().Play("footstep.efkefc", spawnPos, data.smokeScale * scale);
	(void)data; (void)pos; (void)scale;
}