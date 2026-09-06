#pragma once

#include "../BaseScene/BaseScene.h"

class LoadingUI;

class LoadingScene : public BaseScene
{
public:

	LoadingScene() { Init(); }
	~LoadingScene() {}

	void Event() override;
	void Init() override;

	float GetProgress() const;

private:

	enum class LoadStep
	{
		AStarNode,
		NavMeshStart,
		NavMesh,
		Model,
		Finished
	};

	LoadStep m_loadStep = LoadStep::AStarNode;

	// ロード対象のモデルパス
	std::vector<std::string> m_modelPaths;

	// 現在何個目までロードしたか
	size_t m_modelIndex = 0;

	// ロード開始
	bool m_startedLoading = false;
	// ロード完了
	bool m_loadFinished = false;

	std::shared_ptr<LoadingUI> m_loadingUI;
};