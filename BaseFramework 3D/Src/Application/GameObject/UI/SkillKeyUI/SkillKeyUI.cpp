#include "SkillKeyUI.h"

namespace
{
	// 見た目の段階
	enum Frame
	{
		kNormal = 0,	// 通常
		kPressed = 1,	// 押された
	};

	struct FrameStyle
	{
		int   image;
		float scale;
		float offsetY;
		float brightness;
	};

	constexpr FrameStyle kStyle[2] =
	{
		/* 通常     */ { 0, 1.00f,  0.00f, 1.00f },
		/* 押された */ { 1, 0.90f, -0.06f, 0.90f },
	};

	// アニメーションの1コマ分(どの段階を何秒見せるか)
	struct Step
	{
		Frame frame;
		float duration;
	};

	// タイミング: 1回だけ押して戻る
	constexpr Step kTiming[] =
	{
		{ kNormal,  0.70f },
		{ kPressed, 0.20f },
		{ kNormal,  0.10f },
	};

	// 連打: 素早く上下する
	constexpr Step kMash[] =
	{
		{ kNormal,  0.07f },
		{ kPressed, 0.07f },
	};

	// 長押し: 押したまま維持して、離す
	constexpr Step kHold[] =
	{
		{ kNormal,  0.40f },
		{ kPressed, 1.10f },
		{ kNormal,  0.30f },
	};

	// 経過時間から、再生すべきフレームを返す
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

void SkillKeyUI::LoadIfNeeded()
{
	if (m_loaded) return;
	m_loaded = true;

	const std::string dir = "Asset/Textures/UI/Key/";

	struct Def
	{
		SkillKey    key;
		const char* normal;	// 通常
		const char* pressed;	// 押された
	};

	const Def defs[] =
	{
		{ SkillKey::MouseLeft, "mouse_outline.png",  "mouse_left_outline.png"     },
		{ SkillKey::A,         "keyboard_a.png",     "keyboard_a.png"     },
		{ SkillKey::S,         "keyboard_s.png",     "keyboard_s.png"     },
		{ SkillKey::W,         "keyboard_w.png",     "keyboard_w.png"     },
		{ SkillKey::D,         "keyboard_d.png",     "keyboard_d.png"     },
	};

	for (const auto& d : defs)
	{
		KeyData& k = m_keys[static_cast<int>(d.key)];

		const char* names[2] = { d.normal, d.pressed };

		for (int i = 0; i < 2; ++i)
		{
			auto tex = std::make_shared<KdTexture>();

			if (!tex->Load(dir + names[i])) continue;

			// 画像サイズから縦横比を取得
			const auto& info = tex->GetInfo();

			k.tex[i] = tex;
			k.aspect[i] = static_cast<float>(info.Width) / static_cast<float>(info.Height);
		}

		if (!k.tex[1])
		{
			k.tex[1] = k.tex[0];
			k.aspect[1] = k.aspect[0];
		}
	}
}

void SkillKeyUI::Draw(SkillKey key, float x, float y,
	SkillKeyAnim anim, float time, bool isDown, float alpha)
{
	LoadIfNeeded();

	const KeyData& k = m_keys[static_cast<int>(key)];

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

	const FrameStyle& style = kStyle[frame];

	const auto& tex = k.tex[style.image];
	if (!tex) return;

	const float h = kKeyHeight * style.scale;
	const float w = h * k.aspect[style.image];

	const float drawY = y + kKeyHeight * style.offsetY;

	Math::Color color = { style.brightness, style.brightness, style.brightness, alpha };

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		tex.get(),
		static_cast<int>(x), static_cast<int>(drawY),
		static_cast<int>(w), static_cast<int>(h),
		nullptr, &color);
}

int SkillKeyUI::ToVirtualKey(SkillKey key)
{
	switch (key)
	{
	case SkillKey::MouseLeft: return VK_LBUTTON;
	case SkillKey::W:         return 'W';
	case SkillKey::A:         return 'A';
	case SkillKey::S:         return 'S';
	case SkillKey::D:         return 'D';
	default:                  return 0;
	}
}