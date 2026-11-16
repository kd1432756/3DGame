#include "SkillCheckFactory.h"

#include "GaugeStop/SkillCheckGaugeStop.h"
#include "Mash/SkillCheckMash.h"
#include "HoldRelease/SkillCheckHoldRelease.h"
#include "Arrow/SkillCheckArrow.h"
#include "Reaction/SkillCheckReaction.h"
#include "ShrinkRing/SkillCheckShrinkRing.h"
#include "AltMash/SkillCheckAltMash.h"

namespace
{
	struct Entry
	{
		SkillCheckType type;
		float          weight;				// 出やすさ
		float          minProgress;		// 進行度以降に解禁
	};

	using Pool = std::vector<Entry>;

	const Pool& RepairPool()
	{
		static const Pool pool =
		{
			{ SkillCheckType::GaugeStop,   1.0f, 0.0f },
			{ SkillCheckType::Mash,      1.0f, 0.0f },
			{ SkillCheckType::HoldRelease, 1.0f, 0.0f },
			{ SkillCheckType::Arrow,       0.7f, 0.3f },
			{ SkillCheckType::Reaction,  0.7f, 0.4f },
			{ SkillCheckType::ShrinkRing, 1.0f, 0.2f },
			{ SkillCheckType::AltMash,   1.0f, 0.0f },
		};
		return pool;
	}

	// 抽選候補を集める
	void Collect(const Pool& pool, float progress, int excludeType, Pool& out)
	{
		out.clear();

		for (const auto& e : pool)
		{
			if (e.weight <= 0.0f) continue;
			if (progress < e.minProgress) continue;
			if (static_cast<int>(e.type) == excludeType) continue;

			out.push_back(e);
		}
	}

	// 重み付き抽選
	SkillCheckType Pick(const Pool& candidates)
	{
		float total = 0.0f;
		for (const auto& e : candidates) total += e.weight;

		float r = KdRandom::GetFloat(0.0f, total);

		for (const auto& e : candidates)
		{
			if (r < e.weight) return e.type;
			r -= e.weight;
		}

		return candidates.back().type;
	}
}

std::unique_ptr<SkillCheckBase> SkillCheckFactory::Create(int prevType, float progress)
{
	const Pool& pool = RepairPool();

	if (pool.empty())
	{
		return Make(SkillCheckType::GaugeStop);
	}

	progress = std::clamp(progress, 0.0f, 1.0f);

	Pool candidates;
	candidates.reserve(pool.size());

	// 前と別の種類
	Collect(pool, progress, prevType, candidates);

	// 前も含む
	if (candidates.empty())
	{
		Collect(pool, progress, -1, candidates);
	}

	// 一番解禁がはやいやつ
	if (candidates.empty())
	{
		auto it = std::min_element(pool.begin(), pool.end(),
			[](const Entry& a, const Entry& b) { return a.minProgress < b.minProgress; });

		return Make(it->type);
	}

	return Make(Pick(candidates));
}

std::unique_ptr<SkillCheckBase> SkillCheckFactory::Make(SkillCheckType type)
{
	switch (type)
	{
	case SkillCheckType::GaugeStop:   return std::make_unique<SkillCheckGaugeStop>();
	case SkillCheckType::Mash:        return std::make_unique<SkillCheckMash>();
	case SkillCheckType::HoldRelease: return std::make_unique<SkillCheckHoldRelease>();
	case SkillCheckType::Arrow:       return std::make_unique<SkillCheckArrow>();
	case SkillCheckType::Reaction:    return std::make_unique<SkillCheckReaction>();
	case SkillCheckType::ShrinkRing:  return std::make_unique<SkillCheckShrinkRing>();
	case SkillCheckType::AltMash:     return std::make_unique<SkillCheckAltMash>();

	default:
		return std::make_unique<SkillCheckGaugeStop>();
	}
}