#pragma once
#include "../Scene/SceneManager.h"

class EditorIO
{
public:
	void SetStage(SceneManager::StageType stage) { m_stage = stage; }
	SceneManager::StageType GetStage() const { return m_stage; }

	// エディタを開いた時に、ゲームが今いるステージへ合わせる
	void SyncStageWithGame()
	{
		m_stage = SceneManager::Instance().GetStageType();
	}

	static const char* StageName(SceneManager::StageType stage);
	const char* GetStageName() const { return StageName(m_stage); }

	bool Save();
	bool Load();

	bool SaveNodes();
	bool LoadNodes();

	bool SaveNavMesh();
	bool LoadNavMesh();

	bool SaveLocations();
	bool LoadLocations();

	std::string GetScenePath() const { return ResolvePaths().scene; }
	const std::string& GetStatus() const { return m_status; }

private:
	struct Paths
	{
		std::string scene;
		std::string astarNode;
		std::string navMesh;
		std::string location;
	};

	Paths ResolvePaths() const;

	bool SaveScene();
	bool LoadScene();

	SceneManager::StageType m_stage = SceneManager::StageType::Stage1;
	std::string m_status;
};