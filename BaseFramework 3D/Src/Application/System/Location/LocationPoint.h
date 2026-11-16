#pragma once

#include "LocationTag.h"

class LocationPoint
{
public:
	LocationPoint() = default;

	// 引数を指定してオブジェクトを生成できる
	LocationPoint(int id, LocationTag tag, const Math::Vector3& pos)
		: m_id(id), m_tag(tag), m_pos(pos)
	{}

	int GetID() const { return m_id; }
	void SetID(int id) { m_id = id; }

	LocationTag GetTag() const { return m_tag; }
	void SetTag(LocationTag tag) { m_tag = tag; }

	const Math::Vector3& GetPos() const { return m_pos; }
	void SetPos(const Math::Vector3& pos) { m_pos = pos; }

private:
	int m_id = -1;
	LocationTag m_tag = LocationTag::Park;
	Math::Vector3 m_pos = Math::Vector3::Zero;
};