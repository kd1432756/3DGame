#pragma once

class SkyBox : public KdGameObject
{
public:

	SkyBox() {}
	~SkyBox() { Release(); }

	void Init() override;
	void Update() override;
	void DrawUnLit() override;

	const char* GetTypeName() const override
	{
		return "SkyBox";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::System;
	}

private:

	std::shared_ptr<KdModelData> m_spModel = nullptr;

};