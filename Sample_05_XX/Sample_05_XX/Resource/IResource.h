#pragma once

/// <summary>
/// リソースのインターフェイス。
/// ロード処理が発生するものに使用する。
/// </summary>
/// <remarks>
/// 正直そんなに必要ない気もする。
/// </remarks>
class IResource
{
public:
	virtual ~IResource()
	{
	}
	/// <summary>
	/// 派生クラスで実装する読み込みの処理。
	/// </summary>
	/// <param name="filePath">ファイルパス。</param>
	virtual void  LoadImplement(const char* filePath) = 0;
	/// <summary>
	/// ロード。
	/// </summary>
	/// <param name="filePath"></param>
	void Load(const char* filePath)
	{
		m_filePath = filePath;
		LoadImplement(filePath);
	}
	/// <summary>
	/// ロードフラグ入手。
	/// </summary>
	/// <returns>フラグ。</returns>
	bool IsLoaded() const
	{
		return m_isLoaded;
	}
protected:
	/// <summary>
	/// ロード済みに設定。
	/// </summary>
	void SetLoadedMark()
	{
		m_isLoaded = true;
	}
private:
	std::string	m_filePath;		//ファイルパス。
	bool		m_isLoaded = false;		//ロードフラグ。
};

