#pragma once

class StaticMeshObject : public KdGameObject
{
public:

	void Init() override;

	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	void SetModelPath(const std::string& path);

	const std::string& GetModelPath() const
	{
		return m_modelPath;
	}

	const char* GetTypeName() const override
	{
		return "StaticMeshObject";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Prop;
	}

	void CopyFrom(const std::shared_ptr<KdGameObject>& src)
	{
		KdGameObject::CopyFrom(src);

		auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(src);

		if (!mesh) return;

		SetModelPath(mesh->GetModelPath());
	}

	std::shared_ptr<KdModelData> GetModelData() const
	{
		if (!m_spModel)	 return nullptr;
		
		return m_spModel;
	}

	bool IsCollisionEnabled() const
	{
		return m_collisionEnabled;
	}

	void SetIsGround(bool isGround)
	{
		if (m_isGround == isGround)
			return;

		m_isGround = isGround;

		// 既にコライダーがある場合のみ再登録
		if (m_spModel && m_collisionEnabled)
		{
			RegisterCollider();
		}
	}

	bool IsGround() const
	{
		return m_isGround;
	}

	// ===== 描画関連 =====
	// 毎フレーム1回、シーン側から呼ぶ
	static void SetCullingCamera(const KdCamera& camera);

	// 影を落とす最大距離(カメラからの距離)
	static inline float s_shadowDistance = 10.0f;
	// ====================

protected:

	std::shared_ptr<KdModelData> m_spModel = nullptr;

	void ReloadModel();
	virtual void RegisterCollider();

	std::string m_modelPath;

	bool m_collisionEnabled = true;

	bool m_isGround = false;

	void SetCollisionEnabled(bool enabled);

	// ===== 描画処理負荷軽減 =====
	static inline DirectX::BoundingFrustum s_frustum;
	static inline Math::Vector3 s_camPos;

	DirectX::BoundingBox m_localBox;     // モデルローカルのAABB
	bool m_hasBounds = false;

	void CalcLocalBounds();
	bool IsInView() const;
	bool IsInShadowRange() const;
	// ====================
};