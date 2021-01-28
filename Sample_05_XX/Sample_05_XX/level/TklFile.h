#pragma once

#include "Resource/IResource.h"
class TklFile : public IResource
{
public:
	struct SObject {
		std::unique_ptr<char[]> name;	//���f�����B
		int parentNo;					//�e�̔ԍ��B
		float bindPose[4][3];			//�o�C���h�|�[�Y�B
		float invBindPose[4][3];		//�o�C���h�|�[�Y�̋t�s��B
		int no;							//�I�u�W�F�N�g�ԍ��B
		bool isShadowCaster;			//�V���h�E�L���X�^�[�t���O�B
		bool isShadowReceiver;			//�V���h�E���V�[�o�[�t���O�B
		std::vector<int> intDatas;		//int�p�����[�^�B
		std::vector<float> floatDatas;	//float�p�����[�^�B
		std::vector<std::unique_ptr<char[]>> charsDatas;	//������B
		std::vector<Vector3> vec3Datas;	//vec3�p�����[�^�[�B
	};
	/// <summary>
	/// �ǂݍ��ݏ����B
	/// </summary>
	/// <param name="filePath"></param>
	void LoadImplement(const char* filePath) override final;
	/// <summary>
	/// �{�[���ɑ΂��ăN�G�����s���B
	/// <para>�N�G�����s���A�I�u�W�F�N�g�ƃN�G�������������B</para>
	/// <para>�����̊֐��I�u�W�F�N�g��ǂݍ��񂾃I�u�W�F�N�g���Ăяo���B</para>
	/// </summary>
	/// <param name="query">�N�G���֐��B</param>
	void QueryObject(std::function<void(SObject& obj)> query)
	{
		for (auto& obj : m_objects) {
			query(obj);
		}
	}
	/// <summary>
	/// �I�u�W�F�N�g�̐����擾�B
	/// </summary>
	/// <returns></returns>
	const int& GetObjectCount() const
	{
		return m_numObject;
	}
	/// <summary>
	/// �I�u�W�F�N�g���擾�B
	/// </summary>
	/// <param name="i">���Ԗڂ̃I�u�W�F�N�g���B</param>
	/// <returns>�I�u�W�F�N�g�B</returns>
	const SObject& GetObj(int& i) const
	{
		return m_objects[i];
	}
private:
	int m_tklVersion = 100;			//tkl�t�@�C���̃o�[�W�����B
	int m_numObject = 0;			//�I�u�W�F�N�g�̐��B
	std::vector<SObject> m_objects;	//�I�u�W�F�N�g�̃��X�g�B
};

