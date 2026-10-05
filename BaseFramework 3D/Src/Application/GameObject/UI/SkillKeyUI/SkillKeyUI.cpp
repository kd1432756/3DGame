#include "SkillKeyUI.h"

#include <cmath>

namespace
{
	// 画像の3コマ
	enum Frame
	{
		kNormal = 0,	// 通常
		kPressed = 1,	// 押された
		kHalf = 2,	// ちょい押し
	};

	// Frame → 画像内の位置(左から何コマ目か)
	// 画像の並びが「通常 / 押された / ちょい押し」と違う場合は、ここだけ変更する
	constexpr int kSrcIndex[3] = { 0, 1, 2 };

	// アニメーションの1コマ分(どの画像を何秒見せるか)
	struct Step
	{
		Frame frame;
		float duration;
	};

	//---------------------------------------------------
	// アニメーションパターン 見た目を調整するならここ
	//---------------------------------------------------
	// タイミング: 1回だけ「ぐっ」と押して戻る
	constexpr Step kTiming[] =
	{
		{ kNormal,  0.60f },
		{ kHalf,    0.10f },
		{ kPressed, 0.15f },
		{ kNormal,  0.15f },
	};

	// 連打: 素早く上下する
	constexpr Step kMash[] =
	{
		{ kNormal,  0.07f },
		{ kPressed, 0.07f },
	};

	// 長押し: 押し込んだまま維持して、離す
	constexpr Step kHold[] =
	{
		{ kNormal,  0.40f },
		{ kHalf,    0.15f },
		{ kPressed, 0.95f },
		{ kNormal,  0.30f },
	};

	// パターンを time 秒目で評価(ループ再生)
	Frame Evaluate(const Step* steps, int count, float time)
	{
		float total = 0.0f;
		for (int i = 0; i < count; ++i) total += steps[i].duration;

		float t = std::fmod(time, total);
		if (t < 0.0f) t = 0.0f;

		for (int i = 0; i < count; ++i)
		{
			if (t < steps[i].duration) return steps[i].frame;
			t -= steps[i].duration;
		}

		return steps[count - 1].frame;
	}
}

SkillKeyUI& SkillKeyUI::Instance()
{
	static SkillKeyUI instance;
	return instance;
}

void SkillKeyUI::LoadIfNeeded()
{
	if (m_loaded) return;
	m_loaded = true;

	struct Def
	{
		SkillKey    key;
		const char* path;
		int         frameW;
		int         frameH;
	};

	// ※ W/A/S/D のファイル名は仮です。実際のファイル名に合わせてください
	const Def defs[] =
	{
		{ SkillKey::Space, "Asset/Textures/UI/SpaceKey.png", 41, 21 },
		{ SkillKey::W,     "Asset/Textures/UI/WKey.png",     19, 21 },
		{ SkillKey::A,     "Asset/Textures/UI/AKey.png",     19, 21 },
		{ SkillKey::S,     "Asset/Textures/UI/SKey.png",     19, 21 },
		{ SkillKey::D,     "Asset/Textures/UI/DKey.png",     19, 21 },
	};

	for (const auto& d : defs)
	{
		KeyData& k = m_keys[static_cast<int>(d.key)];

		k.tex = std::make_shared<KdTexture>();
		k.tex->Load(d.path);
		k.frameW = d.frameW;
		k.frameH = d.frameH;
	}
}

void SkillKeyUI::Draw(SkillKey key, float x, float y,
	SkillKeyAnim anim, float time, bool isDown, float alpha, int scale)
{
	LoadIfNeeded();

	const KeyData& k = m_keys[static_cast<int>(key)];
	if (!k.tex) return;

	// 実際に押している間は、アニメより優先して「押された」画像
	Frame frame = kNormal;

	if (isDown)
	{
		frame = kPressed;
	}
	else
	{
		switch (anim)
		{
		case SkillKeyAnim::Timing:
			frame = Evaluate(kTiming, static_cast<int>(std::size(kTiming)), time);
			break;
		case SkillKeyAnim::Mash:
			frame = Evaluate(kMash, static_cast<int>(std::size(kMash)), time);
			break;
		case SkillKeyAnim::Hold:
			frame = Evaluate(kHold, static_cast<int>(std::size(kHold)), time);
			break;
		default:
			frame = kNormal;
			break;
		}
	}

	// 3コマ並びの画像から、該当コマだけを切り出す
	Math::Rectangle src = { kSrcIndex[frame] * k.frameW, 0, k.frameW, k.frameH };
	Math::Color color = { 1.0f, 1.0f, 1.0f, alpha };

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		k.tex.get(),
		static_cast<int>(x), static_cast<int>(y),
		k.frameW * scale, k.frameH * scale,
		&src, &color);
}

int SkillKeyUI::ToVirtualKey(SkillKey key)
{
	switch (key)
	{
	case SkillKey::Space: return VK_SPACE;
	case SkillKey::W:     return 'W';
	case SkillKey::A:     return 'A';
	case SkillKey::S:     return 'S';
	case SkillKey::D:     return 'D';
	default:              return 0;
	}
}