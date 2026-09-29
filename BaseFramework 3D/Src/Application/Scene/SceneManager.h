#pragma once

class BaseScene;

class SceneManager
{
public :

	// シーン情報
	enum class SceneType
	{
		Title,
		Game,
		Loading,
		Edit,
	};

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();
	// 追加
	void DrawDebugUI();

	// 次のシーンをセット (次のフレームから切り替わる)
	void SetNextScene(SceneType _nextScene)
	{
		m_nextSceneType = _nextScene;
	}

	// 現在のシーンのオブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList();

	// 現在のシーンにオブジェクトを追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

	// 追加 オブジェクトリストを初期化
	void ClearObjects();

	// 追加 Factoryを取得
	KdGameObjectFactory& GetFactory()
	{
		return m_factory;
	}

	// 追加 オブジェクト作成
	std::shared_ptr<KdGameObject> CreateObject(const std::string& type);
	std::shared_ptr<KdGameObject> CreateObject(
		const std::string& type, const std::string& baseName);

	// 追加 オブジェクトを複製
	std::shared_ptr<KdGameObject> DuplicateObject(std::shared_ptr<KdGameObject> src);

	// 追加 同じオブジェクトのカウント
	void UpdateObjectCount(const std::string& type, int count);

	// 追加
	std::shared_ptr<KdGameObject> CreateObjectRaw(const std::string& type);

	// 追加 
	template<class T>
	std::shared_ptr<T> FindObject()
	{
		for (auto& obj : GetObjList())
		{
			auto cast = std::dynamic_pointer_cast<T>(obj);

			if (cast)
			{
				return cast;
			}
		}

		return nullptr;
	}

	// 追加 
	template<class T>
	std::vector<std::shared_ptr<T>> FindObjects()
	{
		std::vector<std::shared_ptr<T>> result;

		for (auto& obj : GetObjList())
		{
			auto cast =
				std::dynamic_pointer_cast<T>(obj);

			if (cast)
			{
				result.push_back(cast);
			}
		}

		return result;
	}

	// 追加
	void SetSceneScale(float scale)
	{
		m_sceneScale = scale;
	}

	float GetSceneScale() const
	{
		return m_sceneScale;
	}

	enum class StageType
	{
		Stage1,
		Stage2
	};

	struct StageData
	{
		std::string scenePath;
		std::string astarPath;
		std::string navMeshPath;
		std::string locationPointPath;
	};

	void SetStage(StageType stage)
	{
		m_stageType = stage;
	}

	StageData GetStageData() const;

private :

	// マネージャーの初期化
	// インスタンス生成(アプリ起動)時にコンストラクタで自動実行
	void Init()
	{
		// 追加 Factory
		RegisterGameObjects();

		// 開始シーンに切り替え
		ChangeScene(m_currentSceneType);
	}

	// シーン切り替え関数
	void ChangeScene(SceneType _sceneType);

	// 現在のシーンのインスタンスを保持しているポインタ
	std::shared_ptr<BaseScene> m_currentScene = nullptr;

	// 現在のシーンの種類を保持している変数
	SceneType m_currentSceneType = SceneType::Title;
	
	// 次のシーンの種類を保持している変数
	SceneType m_nextSceneType = m_currentSceneType;

	// 追加 Factory
	KdGameObjectFactory m_factory;

	// 追加 typeごとに連番を持つように
	std::unordered_map<std::string, int> m_objectCount;

	void RegisterGameObjects();

	float m_sceneScale = 1.0f;

	StageType m_stageType = StageType::Stage1;
private:

	SceneManager() { Init(); }
	~SceneManager() {}

public:

	// シングルトンパターン
	// 常に存在する && 必ず1つしか存在しない(1つしか存在出来ない)
	// どこからでもアクセスが可能で便利だが
	// 何でもかんでもシングルトンという思考はNG
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};
