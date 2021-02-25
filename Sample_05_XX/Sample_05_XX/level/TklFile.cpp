#include "stdafx.h"
#include "TklFile.h"

void TklFile::LoadImplement(const char* filePath)
{
	auto fp = fopen(filePath, "rb");
	if (fp == nullptr) {
		MessageBoxA(nullptr, "tklFile Name is wrong", "error", MB_OK);
		return;
	}
	//バージョン読み込み。
	fread(&m_tklVersion, sizeof(m_tklVersion), 1, fp);
	//オブジェクトの数を取得。
	fread(&m_numObject, sizeof(m_numObject), 1, fp);
	m_objects.resize(m_numObject);
	for (int i = 0; i < m_numObject; i++) {
		auto& obj = m_objects.at(i);
		size_t nameCount = 0;
		//オブジェクト名取得。
		fread(&nameCount, 1, 1, fp);
		obj.name = std::make_unique<char[]>(nameCount + 1);
		fread(obj.name.get(), nameCount + 1, 1, fp);
		//親のIDを取得。
		fread(&obj.parentNo, sizeof(obj.parentNo), 1, fp);
		//バインドポーズ取得。
		fread(obj.bindPose, sizeof(obj.bindPose), 1, fp);
		//逆バインドポーズ取得。
		fread(obj.invBindPose, sizeof(obj.invBindPose), 1, fp);
		//オブジェクト番号。
		obj.no = i;
		
		//シャドウキャスターのフラグ。
		fread(&obj.isShadowCaster, sizeof(obj.isShadowCaster), 1, fp);
		//シャドウレシーバーのフラグ。
		fread(&obj.isShadowReceiver, sizeof(obj.isShadowReceiver), 1, fp);
		//intパメーターの数。
		int numIntData;
		fread(&numIntData, sizeof(numIntData), 1, fp);
		for (int i = 0; i < numIntData; i++) {
			int val = 0;
			fread(&val, sizeof(val), 1, fp);
			obj.intDatas.push_back(val);
		}
		//floatパメーターの数。
		int numFloatData;
		fread(&numFloatData, sizeof(numFloatData), 1, fp);
		for (int i = 0; i < numFloatData; i++) {
			float val = 0;
			fread(&val, sizeof(val), 1, fp);
			obj.floatDatas.push_back(val);
		}
		//stringパラメータの数。
		int numStringData;
		fread(&numStringData, sizeof(numStringData), 1, fp);
		obj.charsDatas.resize(numStringData);
		for (int i = 0; i < numStringData; i++) {
			//stringパラメータの長さ。
			int numChara;
			fread(&numChara, sizeof(numChara), 1, fp);
			//stringパラメータ。
			obj.charsDatas[i] = std::make_unique<char[]>(numChara + 1);
			fread(obj.charsDatas[i].get(), numChara + 1, 1, fp);
		}
		//vec3のパラメーター。
		int numVec3Data;
		fread(&numVec3Data, sizeof(numVec3Data), 1, fp);
		//vec3パラメーター。
		for (int i = 0; i < numVec3Data; i++) {
			float x, y, z;
			fread(&x, sizeof(x), 1, fp);
			fread(&y, sizeof(y), 1, fp);
			fread(&z, sizeof(z), 1, fp);
			obj.vec3Datas.push_back(Vector3(x, y, z));
		}
	}

	fclose(fp);
	//読み込み終了。
	SetLoadedMark();
}
