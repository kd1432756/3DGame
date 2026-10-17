#pragma once

// ゲーム上に存在するすべてのオブジェクトの基底となるクラス
class KdGameObject : public std::enable_shared_from_this<KdGameObject>
{
public:

	// どのような描画を行うのかを設定するTypeID：Bitフラグで複数指定可能
	enum
	{
		eDrawTypeLit = 1 << 0,
		eDrawTypeUnLit = 1 << 1,
		eDrawTypeBright = 1 << 2,
		eDrawTypeUI = 1 << 3,
		eDrawTypeDepthOfShadow = 1 << 4,
	};

	// 追加
	enum class UpdateGroup
	{
		World,      // ゲーム停止で止まる（キャラ、ギミック、世界のUIなど）
		Always,     // 停止中も動く（結果UI、紙吹雪、ARRESTED演出など）
	};

	KdGameObject() { UpdateWorldMatrix(); }
	virtual ~KdGameObject() { Release(); }

	// 生成される全てに共通するパラメータに対する初期化のみ
	virtual void Init() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	// それぞれの状況で描画する関数
	virtual void GenerateDepthMapFromLight() {}
	virtual void PreDraw() {}
	virtual void DrawLit() {}
	virtual void DrawUnLit() {}
	virtual void DrawEffect() {}
	virtual void DrawBright() {}
	virtual void DrawSprite() {}
	virtual void DrawDebug();
	// 追加
	virtual void DrawDebugUI() {}

	virtual void SetAsset(const std::string&) {}

	// ワールド座標を直接触らない
	//virtual void SetPos(const Math::Vector3& pos) { m_mWorld.Translation(pos); }
	//virtual Math::Vector3 GetPos() const { return m_mWorld.Translation(); }

	// 拡大率を変更する関数
	void SetScale(float scalar);
	// ワールド座標を直接触らない
	//virtual void SetScale(const Math::Vector3& scale);
	//virtual Math::Vector3 GetScale() const;

	const Math::Matrix& GetMatrix() const { return m_mWorld; }

	virtual bool IsExpired() const { return m_isExpired; }

	virtual bool IsVisible()	const { return false; }
	virtual bool IsRideable()	const { return false; }

	// 視錐台範囲内に入っているかどうか
	virtual bool CheckInScreen(const DirectX::BoundingFrustum&) const { return false; }

	// カメラからの距離を計算
	virtual void CalcDistSqrFromCamera(const Math::Vector3& camPos);

	float GetDistSqrFromCamera() const { return m_distSqrFromCamera; }

	UINT GetDrawType() const { return m_drawType; }

	bool Intersects(const KdCollider::SphereInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::BoxInfo& targetBox, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);

	// エディタ用 名前 追加
	void SetName(const std::string& name)
	{
		m_name = name;
	}
	std::string& GetName()
	{
		return m_name;
	}
	const std::string& GetName() const
	{
		return m_name;
	}
	// オブジェクトの種類
	virtual const char* GetTypeName() const
	{
		return "GameObject";
	}

	// 追加 ==========
	virtual void SetPos(const Math::Vector3& pos)
	{
		m_pos = pos;

		UpdateWorldMatrix();
	}
	virtual void SetRot(const Math::Vector3& rot)
	{
		m_rot = rot;

		UpdateWorldMatrix();
	}
	virtual void SetScale(const Math::Vector3& scale)
	{
		m_scale = scale;

		UpdateWorldMatrix();
	}

	const Math::Vector3& GetPos() const { return m_pos; }
	const Math::Vector3& GetRot() const { return m_rot; }
	const Math::Vector3& GetScale() const { return m_scale; }

	const Math::Color& GetColor() const
	{
		return m_color;
	}
	void SetColor(const Math::Color& color)
	{
		m_color = color;
	}

	// ===============

	// 追加
	virtual void Destroy()
	{
		m_isExpired = true;
	}

	// 追加
	virtual void CopyFrom(const std::shared_ptr<KdGameObject>& src)
	{
		SetPos(src->GetPos());
		SetRot(src->GetRot());
		SetScale(src->GetScale());
	}

	// 追加
	enum class ObjectCategory
	{
		Character,
		Stage,
		Prop,
		Interactive,
		Item,
		UI,
		Effect,
		System,
		Other
	};

	// 追加
	virtual ObjectCategory GetCategory() const
	{
		return ObjectCategory::Other;
	}

	// 追加
	UpdateGroup GetUpdateGroup() const { return m_updateGroup; }

protected:

	void Release() {}

	// 描画タイプ・何の描画を行うのかを決める / 最適な描画リスト作成用
	UINT m_drawType = 0;

	// カメラからの距離
	float m_distSqrFromCamera = 0;

	// 存在消滅フラグ
	bool m_isExpired = false;

	// 3D空間に存在する機能
	Math::Matrix	m_mWorld;

	// 追加 ==========
	Math::Vector3 m_pos;
	Math::Vector3 m_rot;
	Math::Vector3 m_scale = { 1,1,1 };

	Math::Color m_color = kWhiteColor;
	// ===============

	// 当たり判定クラス
	std::unique_ptr<KdCollider> m_pCollider = nullptr;

	// デバッグ情報クラス
	std::unique_ptr<KdDebugWireFrame> m_pDebugWire = nullptr;

	// エディタ用 名前 追加
	std::string m_name = "GameObject";

	// 追加 ワールド座標更新
	virtual void UpdateWorldMatrix()
	{
		Math::Matrix rotMat =
			Math::Matrix::CreateFromYawPitchRoll(
				DirectX::XMConvertToRadians(m_rot.y),
				DirectX::XMConvertToRadians(m_rot.x),
				DirectX::XMConvertToRadians(m_rot.z));

		m_mWorld =
			Math::Matrix::CreateScale(m_scale) *
			rotMat *
			Math::Matrix::CreateTranslation(m_pos);
	}

	// 追加
	UpdateGroup m_updateGroup = UpdateGroup::World;

private:

};
