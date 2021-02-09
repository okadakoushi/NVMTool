/// <summary>
/// ��@�\���x���B
/// NavMesh�p�̃Z���쐬�̂��߁A�ʒu��񓙂̂ݎZ�o����N���X�B
/// �����_���[�Ƃ��Ă̔\�͎͂����ĂȂ��B
/// todo:���x�������include�֌W��������ƓK���Ȃ̂Œ����B
/// </summary>

#pragma once

class MapChip;
class MapChipRender;
class Bone;

#include "TklFile.h"
#include "Noncopyable.h"

/// <summary>
/// �I�u�W�F�N�g�f�[�^�[�B
/// �t�b�N�ł���͈́B
/// </summary>
struct LevelObjectData {
	Vector3 position;		//���W�B
	Quaternion rotatatin;	//��]�B
	Vector3 scale;			//�g��B
	const wchar_t* name = nullptr;	//���O�B
	bool isShadowCaster;	//�V���h�E�L���X�^�[�B
	bool isShadowReceiver;	//�V���h�E���V�[�o�[�B
	/// <summary>
	/// �����œn�����I�u�W�F�N�g���̃I�u�W�F�N�g�����肷��B
	/// </summary>
	/// <param name="objName">���ׂ�I�u�W�F�N�g���B</param>
	/// <returns>���O����v������true�B</returns>
	bool EqualObjectName(const wchar_t* objName)
	{
		return wcscmp(objName, name) == 0;
	}
};

/// <summary>
/// ���x���B
/// </summary>
class Level : Noncopyable
{
private:
public:
	~Level();
	/// <summary>
	/// ���x�����������B
	/// <para>�t�b�N����ꍇ(MapCihp�Ȃ�)True�B�ʂ̌`��Draw���ĂˁB</para>
	/// <para>�t�b�N���Ȃ����`��͑��点��(MapChip����)false�B</para>
	/// <para>�t�b�N�����Ȃ����`������Ȃ�(MapChip�Ȃ�)nullptr�B</para>
	/// </summary>
	/// <remarks>
	/// �t�b�N�֐����ŁA�I�u�W�F�N�g��`�悷��N���X�̃C���X�^���X���쐬�����Ƃ��ɁAfalse���w�肷���
	/// MapChip�ƃN���X��Draw������A2�d�`��ɂȂ�̂ŁAnullptr��false�͎g�������邱�ƁB
	/// </remarks>
	/// <param name="filePath">�t�@�C���p�X�B</param>
	/// <param name="hookFunc">�I�u�W�F�N�g�쐬���Ƀt�b�N����֐��I�u�W�F�N�g�B</param>
	void Init(const char* filePath, std::function<bool(LevelObjectData& obj)> hookFunc = nullptr);
	/// <summary>
	/// ���x���I�u�W�F�N�g�f�[�^���擾�B
	/// </summary>
	/// <param name="ObjNo">�I�u�W�F�N�g�ԍ��B</param>
	/// <returns>�I�u�W�F�N�g�B</returns>
	const LevelObjectData& GetLevelObj(int ObjNo) const
	{
		return objData[ObjNo];
	}
	/// <summary>
	/// tkl�t�@�C�����擾�B
	/// </summary>
	/// <returns></returns>
	const TklFile& GetTklFile() const
	{
		return m_tklFile;
	}
private:
	/// <summary>
	/// �}�b�v�`�b�v�����_�[���쐬�ł�����쐬 or �`�悷�ׂ��I�u�W�F�N�g�̃C���N�������g�B
	/// </summary>
	/// <remarks>
	/// ���o�^�I�u�W�F�N�g�������_�[���[���쐬�B
	/// �o�^�ς݂̏ꍇ�������_�[���[���`�悷��I�u�W�F�N�g���C���N�������g�B
	/// </remarks>
	/// <param name="objData">�I�u�W�F�N�g�f�[�^�[�B</param>
	/// <returns></returns>
	MapChipRender* CteateMapChipRenderOrAddRenderObject(const LevelObjectData& objData);
	/// <summary>
	/// �{�[���s��̍\�z�B
	/// </summary>
	void BuildBoneMatrices();
private:
	TklFile m_tklFile;							//tkl�t�@�C���B
	static const int BONE_MAX;					//�{�[���̍ő吔�B
	using BonePtr = std::unique_ptr<Bone>;		//�{�[���̃|�C���^�B
	std::vector<BonePtr> m_bones;				//�{�[���z��B
	std::unique_ptr<Matrix[]> m_boneMatirxs;	//�{�[���s��B
	bool m_isInited = false;					//�����������H
	LevelObjectData objData[64];				//�I�u�W�F�N�g�f�[�^�[�B
};

