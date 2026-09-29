#pragma once
#include "SkillCheckBase.h"

//==================================================
// スキルチェック抽選
//  ・いたずら用 / 修復用でプールを分ける
//  ・直前と同じ種類は除外(候補が1種類しかない場合のみ許可)
//  ・progress(0.0=朝 〜 1.0=夜)で、難しい種類を後半から解禁
//  ・重み付き抽選
//==================================================
class SkillCheckFactory
{
public:
	// prevType : 直前に使った種類(static_cast<int>(SkillCheckType))。初回は -1
	// progress : ゲーム進行度 0.0〜1.0(不要なら省略)
	static std::unique_ptr<SkillCheckBase> Create(
		bool isRepair,
		int  prevType = -1,
		float progress = 0.0f);

	// 種類を指定して生成(デバッグ・特定イベント用)
	static std::unique_ptr<SkillCheckBase> Make(SkillCheckType type);
};