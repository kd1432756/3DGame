#include "SkillCheckFactory.h"

#include "GaugeStop/SkillCheckGaugeStop.h"
#include "Mash/SkillCheckMash.h"
#include "HoldRelease/SkillCheckHoldRelease.h"
#include "Arrow/SkillCheckArrow.h"
#include "Reaction/SkillCheckReaction.h"

namespace
{
	// プールの1エントリ
	struct Entry
	{
		SkillCheckType type;
		float          weight;		// 出やすさ(大きいほど出やすい / 0以下なら無効)
		float          minProgress;	// この進行度(朝0〜夜1)以降に解禁
	};

	using Pool = std::vector<Entry>;

	//----------------------------------------------
	// プール定義 ここを編集して調整する
	// いたずら: スリル重視(焦らせる系)
	// 修復    : 落ち着いた系
	//----------------------------------------------
	const Pool& PrankPool()
	{
		static const Pool pool =
		{
			{ SkillCheckType::GaugeStop, 1.0f, 0.0f },
			{ SkillCheckType::Mash,      1.0f, 0.0f },
			{ SkillCheckType::Reaction,  0.7f, 0.4f },
		};
		return pool;
	}

	const Pool& RepairPool()
	{
		static const Pool pool =
		{
			{ SkillCheckType::GaugeStop,   1.0f, 0.0f },
			{ SkillCheckType::HoldRelease, 1.0f, 0.0f },
			{ SkillCheckType::Arrow,       0.7f, 0.3f },
		};
		return pool;
	}

	// 抽選候補を集める
	//  ・重みが0以下のものは無効として除外
	//  ・解禁前(progress < minProgress)のものは除外
	//  ・excludeType と同じ種類は除外(負の値なら除外なし)
	void Collect(const Pool& pool, float progress, int excludeType, Pool& out)
	{
		out.clear();

		for (const auto& e : pool)
		{
			if (e.weight <= 0.0f)                       continue;
			if (progress < e.minProgress)               continue;
			if (static_cast<int>(e.type) == excludeType) continue;

			out.push_back(e);
		}
	}

	// 重み付き抽選(candidates は空でなく、全て weight > 0 が前提)
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

		return candidates.back().type;	// 浮動小数の誤差対策
	}
}

std::unique_ptr<SkillCheckBase> SkillCheckFactory::Create(
	bool isRepair, int prevType, float progress)
{
	const Pool& pool = isRepair ? RepairPool() : PrankPool();

	// プールが空の場合の保険
	if (pool.empty())
	{
		return Make(SkillCheckType::GaugeStop);
	}

	progress = std::clamp(progress, 0.0f, 1.0f);

	Pool candidates;
	candidates.reserve(pool.size());

	// 1. 解禁済み かつ 直前と別の種類
	Collect(pool, progress, prevType, candidates);

	// 2. 空なら(種類が1つしかない等)、直前除外を諦める
	if (candidates.empty())
	{
		Collect(pool, progress, -1, candidates);
	}

	// 3. それでも空なら(全て未解禁 or 無効)、解禁が最も早いものを使う
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

	default:
		return std::make_unique<SkillCheckGaugeStop>();
	}
}