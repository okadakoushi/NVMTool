#pragma once
#define _CRT_SECURE_NO_WARNINGS

const float DELTA_TIME = 1.0f / 60.0f;	//更新時間。後でGameTimeクラス作ること。
const int NUM_DIRECTIONAL_LIGHT = 4;	//ディレクションライトの数。
static const int NUM_SHADOW_MAP = 3;			//カスケードシャドウマップの数。


#include "MiniEngine.h"
#include "Skeleton.h"

using namespace std;