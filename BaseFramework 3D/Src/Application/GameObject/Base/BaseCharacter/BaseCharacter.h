#pragma once

class BaseCharacter : public KdGameObject
{
public:
	virtual void Init() override;
	virtual void Update() override;
	virtual void DrawLit() override;
	virtual void GenerateDepthMapFromLight() override;

	void AddScore(int score)
	{
		m_score += score;
	}

	int GetScore() const
	{
		return m_score;
	}

protected:
	void CheckGround();
	void ResolveCollision(
		const Math::Vector3& moveDir);

	void ApplyGravity();

	void RotateToMoveDirection(const Math::Vector3& dir);

	void UpdateAnimation();
	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	float m_sceneScale = 1.0f;

	float m_gravity = 0.0f;

	float m_gravityPower = 0.015f;

	bool m_isGround = false;

	float m_jumpPower = 0.30f;

	float m_angle = 0.0f;

	int m_score = 0;
};