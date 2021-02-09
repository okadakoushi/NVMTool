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
	//���x���̃��[�h�B
	m_tklFile.Load(filePath);
	//�{�[���̍s����\�z�B
	BuildBoneMatrices();

	////���x������f�[�^�̎擾�B
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

	//0�̓��[�g�{�[���Ȃ̂ł���Ȃ��B
	for (int i = 1; i < m_bones.size(); i++) {
		auto bone = m_bones[i].get();
		if (bone->GetParentBoneNo() == 0) {//�e�����[�g�Ȃ�}�b�v�`�b�v���쐬�B
			Vector3 scale;
			bone->CalcWorldTRS(objData[i].position, objData[i].rotatatin, objData[i].scale);
			//Max�Ǝ����Ⴄ�̂ŕ␳�B
			//�c�[���ł�AABB�Ƃ�Ƃ��ɍs���Ă�̂ł��Ȃ��B
			//float fix = objData[i].position.y;
			//objData[i].position.y = objData[i].position.z;
			//objData[i].position.z = -fix;

			//fix = objData[i].rotatatin.y;
			//objData[i].rotatatin.y = objData[i].rotatatin.z;
			//objData[i].rotatatin.z = -fix;

			////����ւ��B
			//std::swap(objData[i].scale.y, objData[i].scale.z);
			//�p�����[�^�[�B
			objData[i].name = bone->GetName();
			//objData.isShadowCaster = Params.at(i).isShadowCaster;
			//objData.isShadowReceiver = Params.at(i).isShadowReceiver;

			////�t�b�N�B
			//bool isHook = false;
			//if (hookFunc != nullptr) {
			//	//�t�b�N����p�̊֐��I�u�W�F�N�g������B
			//	isHook = hookFunc(objData);
			//}
			//if (isHook == false) {
			//	//�}�b�v�`�b�v�����_�[���쐬����B
			//	CteateMapChipRenderOrAddRenderObject(objData);
			//}
		}
	}

	////�}�b�v�`�b�v�����_�[�̏������B
	//for (auto& mapChipRender : m_mapChipRenderPtrs) {
	//	mapChipRender.second->InitRenderObject();
	//	mapChipRender.second->QueryRenderObjDatas([&](const LevelObjectData& objData) {
	//		//�t�b�N����Ȃ������̂ŁA�}�b�v�`�b�v���쐬����B
	//		MapChipPtr mapChip = std::make_unique<MapChip>(objData, mapChipRender.second);
	//		//�|�C���^���ڏ��B
	//		m_mapChipPtrs.push_back(std::move(mapChip));
	//	});
	//}
}

//MapChipRender* Level::CteateMapChipRenderOrAddRenderObject(const LevelObjectData& objData)
//{
//	WNameKey nameKey(objData.name);
//	//���łɓo�^����Ă���I�u�W�F�N�g���������B
//	auto itFind = m_mapChipRenderPtrs.find(nameKey.GetHashCode());
//	MapChipRender* pMapChipRender = nullptr;
//	if (itFind == m_mapChipRenderPtrs.end()) {
//		//�o�^����ĂȂ��B
//		auto mapChipRender = NewGO<MapChipRender>(EnPriority_3DModel);
//		pMapChipRender = mapChipRender;
//		//�o�^�o�^�B
//		m_mapChipRenderPtrs.insert({ nameKey.GetHashCode(), mapChipRender });
//	}
//	else {
//		//�o�^����Ă��B
//		//�`�悷��I�u�W�F�N�g�𑝂₷�B
//		pMapChipRender = itFind->second;
//	}
//	pMapChipRender->AddRenderObject(objData);
//
//	return pMapChipRender;
//}

void Level::BuildBoneMatrices()
{
	m_tklFile.QueryObject([&](TklFile::SObject& tklObj) {
		//�o�C���h�|�[�Y�B
		Matrix bindPoseMatrix;
		memcpy(bindPoseMatrix.m[0], &tklObj.bindPose[0], sizeof(tklObj.bindPose[0]));
		memcpy(bindPoseMatrix.m[1], &tklObj.bindPose[1], sizeof(tklObj.bindPose[1]));
		memcpy(bindPoseMatrix.m[2], &tklObj.bindPose[2], sizeof(tklObj.bindPose[2]));
		memcpy(bindPoseMatrix.m[3], &tklObj.bindPose[3], sizeof(tklObj.bindPose[3]));
		bindPoseMatrix.m[0][3] = 0.0f;
		bindPoseMatrix.m[1][3] = 0.0f;
		bindPoseMatrix.m[2][3] = 0.0f;
		bindPoseMatrix.m[3][3] = 1.0f;

		//�o�C���h�|�[�Y�̋t�s��B
		Matrix invBindPoseMatirx;
		memcpy(invBindPoseMatirx.m[0], &tklObj.invBindPose[0], sizeof(tklObj.invBindPose[0]));
		memcpy(invBindPoseMatirx.m[1], &tklObj.invBindPose[1], sizeof(tklObj.invBindPose[1]));
		memcpy(invBindPoseMatirx.m[2], &tklObj.invBindPose[2], sizeof(tklObj.invBindPose[2]));
		memcpy(invBindPoseMatirx.m[3], &tklObj.invBindPose[3], sizeof(tklObj.invBindPose[3]));
		invBindPoseMatirx.m[0][3] = 0.0f;
		invBindPoseMatirx.m[1][3] = 0.0f;
		invBindPoseMatirx.m[2][3] = 0.0f;
		invBindPoseMatirx.m[3][3] = 1.0f;

		//�{�[�����B
		wchar_t boneName[256];
		//tkl�̃I�u�W�F�N�g���̌^�����C�h�����ɕϊ��B
		mbstowcs(boneName, tklObj.name.get(), 256);
		BonePtr bone = std::make_unique<Bone>(
			boneName,
			bindPoseMatrix,
			invBindPoseMatirx,
			tklObj.parentNo,
			tklObj.no
			);
		//�{�[�������ʁB
		
			//�{�[����ςށB
			m_bones.push_back(std::move(bone));
		});
	
	for (auto& bone : m_bones) {
		if (bone->GetParentBoneNo() != -1) {
			m_bones.at(bone->GetParentBoneNo())->AddChild(bone.get());
			//���[�J���}�g���N�X�v�Z�B
			const Matrix& parentMatrix = m_bones.at(bone->GetParentBoneNo())->GetInvBindPoseMatrix();
			Matrix localMatrix;
			//���s�ړ��s��͂��������B
			localMatrix = bone->GetBindPoseMatrix() * parentMatrix;
			bone->SetLocalMatrix(localMatrix);
		}
		else {
			//����ȏ�e���Ȃ��B
			bone->SetLocalMatrix(bone->GetBindPoseMatrix());
		}
	}

	//�{�[���s����m�ہB
	m_boneMatirxs = std::make_unique<Matrix[]>(m_bones.size());
	//�������I���B
	m_isInited = true;
}
