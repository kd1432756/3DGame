#pragma once

#include "../MenuUI/MenuUI.h"

#include "../../../System/Score/ScoreManager.h"

class GameScene;
class ResultFadeUI;
class ConfettiUI;

class ResultUI : public MenuUI
{
public:
	// リザルトはポーズ中も動くので常時更新
	ResultUI() : MenuUI(true) {}

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetGameScene(std::shared_ptr<GameScene> gameScene)
	{
		m_wpGameScene = gameScene;
	}

	void SetConfetti(const std::shared_ptr<ConfettiUI>& confetti) { m_wpConfetti = confetti; }

protected:
	// フェードが終わったらタイトルへ
	void OnFadeOutFinished() override;

private:

	enum class Phase
	{
		ScoreCount,		// スコアのカウントアップ
		RankWait,		// ランク登場前の間
		RankReveal,		// ランク登場
		Done			// 演出完了(ボタン操作可)
	};

	void ShowResult();
	void StartCelebration();
	void ChangePhase(Phase phase);

	std::weak_ptr<GameScene> m_wpGameScene;
	std::shared_ptr<ResultFadeUI> m_spFadeUI;

	bool m_visible = false;
	bool m_resultInitialized = false;

	Phase m_phase = Phase::ScoreCount;
	float m_phaseTimer = 0.0f;

	int m_finalScore = 0;
	int m_displayScore = 0;
	ScoreManager::Rank m_rank = ScoreManager::Rank::C;

	// Titleボタン
	MenuButton m_titleButton;

	float m_animTime = 0.0f;
	float m_animOffsetY = 0.0f;

	std::weak_ptr<ConfettiUI> m_wpConfetti;

	std::shared_ptr<KdSoundInstance> m_resultBgm;

	// ---- 調整用パラメータ ----
	static constexpr float kCountTime = 1.5f;		// スコアのカウントアップ時間
	static constexpr float kRankWaitTime = 0.6f;	// ランクが出るまでの間
	static constexpr float kRankRevealTime = 0.4f;	// ランクの登場アニメ時間
};
