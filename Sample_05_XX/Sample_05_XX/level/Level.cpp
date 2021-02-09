#include "stdafx.h"
#include "Level.h"
#include "Resource/IResource.h"
#include "Skeleton.h"
#include <comdef.h>

Level::~Level()
{
}

void Level::Init(const char* filePath, std::function<bool(LevelObjectData& obj)> hookFunc)
{
	//レベルのロード。
	m_tklFile.Load(filePath);
	//ボーンの行列を構築。
	BuildBoneMatrices();

	////レベルからデータの取得。
	//struct SParams {
	//	bool isShadowCaster;
	//	bool isShadowReceiver;
	//};
	//vector<SParams> Params;
	//m_tklFile.QueryObject([&](TklFile::SObject& tklObj)
	//	{
	//		SParams objParam;
	//		objParam.isShadowCaster = tklObj.isShadowCaster;
	//		objParam.isShadowReceiver = tklObj.isShadowReceiver;
	//		Params.push_back(objParam);
	//	});

	//0はルートボーンなのでいらない。
	for (int i = 1; i < m_bones.size(); i++) {
		auto bone = m_bones[i].get();
		if (bone->GetParentBoneNo() == 0) {//親がルートならマップチップを作成。
			Vector3 scale;
			bone->CalcWorldTRS(objData[i].position, objData[i].rotatatin, objData[i].scale);
			//Maxと軸が違うので補正。
			//ツールではAABBとるときに行ってるのでやらない。
			//float fix = objData[i].position.y;
			//objData[i].position.y = objData[i].position.z;
			//objData[i].position.z = -fix;

			//fix = objData[i].rotatatin.y;
			//objData[i].rotatatin.y = objData[i].rotatatin.z;
			//objData[i].rotatatin.z = -fix;

			////入れ替え。
			//std::swap(objData[i].scale.y, objData[i].scale.z);
			//パラメーター。
			objData[i].name = bone->GetName();
			//objData.isShadowCaster = Params.at(i).isShadowCaster;
			//objData.isShadowReceiver = Params.at(i).isShadowReceiver;

			////フック。
			//bool isHook = false;
			//if (hookFunc != nullptr) {
			//	//フックする用の関数オブジェクトがある。
			//	isHook = hookFunc(objData);
			//}
			//if (isHook == false) {
			//	//マップチップレンダーを作成する。
			//	CteateMapChipRenderOrAddRenderObject(objData);
			//}
		}
	}

	////マップチップレンダーの初期化。
	//for (auto& mapChipRender : m_mapChipRenderPtrs) {
	//	mapChipRender.second->InitRenderObject();
	//	mapChipRender.second->QueryRenderObjDatas([&](const LevelObjectData& objData) {
	//		//フックされなかったので、マップチップを作成する。
	//		MapChipPtr mapChip = std::make_unique<MapChip>(objData, mapChipRender.second);
	//		//ポインタを移譲。
	//		m_mapChipPtrs.push_back(std::move(mapChip));
	//	});
	//}
}

//MapChipRender* Level::CteateMapChipRenderOrAddRenderObject(const LevelObjectData& objData)
//{
//	WNameKey nameKey(objData.name);
//	//すでに登録されているオブジェクトかを検索。
//	auto itFind = m_mapChipRenderPtrs.find(nameKey.GetHashCode());
//	MapChipRender* pMapChipRender = nullptr;
//	if (itFind == m_mapChipRenderPtrs.end()) {
//		//登録されてない。
//		auto mapChipRender = NewGO<MapChipRender>(EnPriority_3DModel);
//		pMapChipRender = mapChipRender;
//		//登録登録。
//		m_mapChipRenderPtrs.insert({ nameKey.GetHashCode(), mapChipRender });
//	}
//	else {
//		//登録されてた。
//		//描画するオブジェクトを増やす。
//		pMapChipRender = itFind->second;
//	}
//	pMapChipRender->AddRenderObject(objData);
//
//	return pMapChipRender;
//}

void Level::BuildBoneMatrices()
{
	m_tklFile.QueryObject([&](TklFile::SObject& tklObj) {
		//バインドポーズ。
		Matrix bindPoseMatrix;
		memcpy(bindPoseMatrix.m[0], &tklObj.bindPose[0], sizeof(tklObj.bindPose[0]));
		memcpy(bindPoseMatrix.m[1], &tklObj.bindPose[1], sizeof(tklObj.bindPose[1]));
		memcpy(bindPoseMatrix.m[2], &tklObj.bindPose[2], sizeof(tklObj.bindPose[2]));
		memcpy(bindPoseMatrix.m[3], &tklObj.bindPose[3], sizeof(tklObj.bindPose[3]));
		bindPoseMatrix.m[0][3] = 0.0f;
		bindPoseMatrix.m[1][3] = 0.0f;
		bindPoseMatrix.m[2][3] = 0.0f;
		bindPoseMatrix.m[3][3] = 1.0f;

		//バインドポーズの逆行列。
		Matrix invBindPoseMatirx;
		memcpy(invBindPoseMatirx.m[0], &tklObj.invBindPose[0], sizeof(tklObj.invBindPose[0]));
		memcpy(invBindPoseMatirx.m[1], &tklObj.invBindPose[1], sizeof(tklObj.invBindPose[1]));
		memcpy(invBindPoseMatirx.m[2], &tklObj.invBindPose[2], sizeof(tklObj.invBindPose[2]));
		memcpy(invBindPoseMatirx.m[3], &tklObj.invBindPose[3], sizeof(tklObj.invBindPose[3]));
		invBindPoseMatirx.m[0][3] = 0.0f;
		invBindPoseMatirx.m[1][3] = 0.0f;
		invBindPoseMatirx.m[2][3] = 0.0f;
		invBindPoseMatirx.m[3][3] = 1.0f;

		//ボーン名。
		wchar_t boneName[256];
		//tklのオブジェクト名の型をワイド文字に変換。
		mbstowcs(boneName, tklObj.name.get(), 256);
		BonePtr bone = std::make_unique<Bone>(
			boneName,
			bindPoseMatrix,
			invBindPoseMatirx,
			tklObj.parentNo,
			tklObj.no
			);
		//ボーン名判別。
		
			//ボーンを積む。
			m_bones.push_back(std::move(bone));
		});
	
	for (auto& bone : m_bones) {
		if (bone->GetParentBoneNo() != -1) {
			m_bones.at(bone->GetParentBoneNo())->AddChild(bone.get());
			//ローカルマトリクス計算。
			const Matrix& parentMatrix = m_bones.at(bone->GetParentBoneNo())->GetInvBindPoseMatrix();
			Matrix localMatrix;
			//平行移動行列はかき消す。
			localMatrix = bone->GetBindPoseMatrix() * parentMatrix;
			bone->SetLocalMatrix(localMatrix);
		}
		else {
			//これ以上親いない。
			bone->SetLocalMatrix(bone->GetBindPoseMatrix());
		}
	}

	//ボーン行列を確保。
	m_boneMatirxs = std::make_unique<Matrix[]>(m_bones.size());
	//初期化終わり。
	m_isInited = true;
}
