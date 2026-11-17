#pragma once

enum class SkillKey
{
	MouseLeft,
	W,
	A,
	S,
	D,
	Count,
};

enum class SkillKeyAnim
{
	Static,		// 静止
	Timing,		// タイミングよく押す
	Mash,		// 連打する
	Hold,			// 長押しする
};

class SkillKeyUI
{
public:
	void Draw(SkillKey key, float x, float y,
		SkillKeyAnim anim, float time, bool isDown,
		float alpha = 1.0f);

	static int ToVirtualKey(SkillKey key);

private:
	SkillKeyUI() = default;

	struct KeyData
	{
		std::shared_ptr<KdTexture> tex[2];	// 0=通常 / 1=押された
		float aspect[2] = { 1.0f, 1.0f };
	};

	void LoadIfNeeded();

	KeyData m_keys[static_cast<int>(SkillKey::Count)];
	bool   m_loaded = false;

	static constexpr float kKeyHeight = 44.0f;

public:

	static SkillKeyUI& Instance()
	{
		static SkillKeyUI instance;
		return instance;
	}

};