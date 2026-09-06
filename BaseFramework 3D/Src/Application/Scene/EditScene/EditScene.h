#pragma once

#include"../BaseScene/BaseScene.h"

class EditScene : public BaseScene
{
public :

	EditScene() { Init(); }
	~EditScene() {}

private:

	void Event() override;
	void Init()  override;

};
