#pragma once

/// <summary>
/// ���\�[�X�̃C���^�[�t�F�C�X�B
/// ���[�h����������������̂Ɏg�p����B
/// </summary>
/// <remarks>
/// ��������ȂɕK�v�Ȃ��C������B
/// </remarks>
class IResource
{
public:
	virtual ~IResource()
	{
	}
	/// <summary>
	/// �h���N���X�Ŏ�������ǂݍ��݂̏����B
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X�B</param>
	virtual void  LoadImplement(const char* filePath) = 0;
	/// <summary>
	/// ���[�h�B
	/// </summary>
	/// <param name="filePath"></param>
	void Load(const char* filePath)
	{
		m_filePath = filePath;
		LoadImplement(filePath);
	}
	/// <summary>
	/// ���[�h�t���O����B
	/// </summary>
	/// <returns>�t���O�B</returns>
	bool IsLoaded() const
	{
		return m_isLoaded;
	}
protected:
	/// <summary>
	/// ���[�h�ς݂ɐݒ�B
	/// </summary>
	void SetLoadedMark()
	{
		m_isLoaded = true;
	}
private:
	std::string	m_filePath;		//�t�@�C���p�X�B
	bool		m_isLoaded = false;		//���[�h�t���O�B
};

