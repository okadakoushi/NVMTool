/// <summary>
/// NavMesh�쐬�p�c�[���B
/// </summary>

#include "stdafx.h"
#include <iostream>
#include "tkFile/tkmFile.h"
#include "level/TklFile.h"
#include "level/Level.h"

//�����̂��\������W���_�B
enum Rectangular {
	//�ق�Ƃ�EnRectangular~~���ď����Ȃ��Ƃ����Ȃ����ǒ����E�E�E�B
	//�c�[�������炻��Ȃɕϐ��Ȃ������v���ȁBtodo:EnRectangular??
	//�����̂��\�����钸�_�ԍ��̋K���Ƃ�����̂��ȁH
	EnFupperLeft,			//��O����B
	EnFlowerLeft,			//��O�����B
	EnFupperRight,			//��O�E��B
	EnFlowerRight,			//��O�E���B
	EnBupperLeft,			//������B
	EnBlowerLeft,			//�������B
	EnBupperRight,			//���E��B
	EnBlowerRight,			//���E���B
	EnRectangular_Num
};

/// <summary>
/// �Z���B
/// </summary>
struct Cell {
	Vector3 pos[3];
	Cell* linkCell[3];
};
/// <summary>
/// �i�r�Q�[�V�������b�V���B
/// </summary>
struct NaviMesh {
	std::vector<Cell>	m_cell;
};
/// <summary>
/// ��Q���B
/// </summary>
struct Obstacle {
	Vector3 v[EnRectangular_Num];	//8���_�B
	int ObjNo = 0;					//�ꉞ�I�u�W�F�N�g�i���o�[�B
};
/// <summary>
/// �����B
/// </summary>
struct Line {
	Vector3 SPoint;	//�n�_�B
	Vector3 EPoint;	//�I�_�B
};
/// <summary>
/// AABB�B
/// </summary>
struct AABB {
	Vector3 v[8];	//AABB���\������W�_�B
	Line line[12];	//AABB���\����������P�Q�{�B
};

/// <summary>
/// ��̃��b�V������i�r�Q�[�V�������b�V���̃Z�����쐬�B
/// </summary>
/// <returns></returns>
template<class TIndexBuffer> 
void BuildCellsFromOneMesh(
	NaviMesh& naviMesh,
	const TIndexBuffer& indexBuffer, 
	const TkmFile::SMesh& mesh
)
{
	for (auto& indexBuffer : indexBuffer) {
		//3�̒��_��1�|���Ȃ̂ŁA�C���f�b�N�X�� / 3�Ń|���S�����B
		int numPolygon = static_cast<int>(indexBuffer.indices.size() / 3);
		for (int i = 0; i < numPolygon; i++) {
			int no_0 = i * 3;
			int no_1 = i * 3 + 1;
			int no_2 = i * 3 + 2;
			//���_�ԍ����擾�B
			int vertNo_0 = indexBuffer.indices[no_0];
			int vertNo_1 = indexBuffer.indices[no_1];
			int vertNo_2 = indexBuffer.indices[no_2];
			//���_����Z�����쐬�B
			Cell c;
			c.pos[0] = mesh.vertexBuffer[vertNo_0].pos;
			c.pos[1] = mesh.vertexBuffer[vertNo_1].pos;
			c.pos[2] = mesh.vertexBuffer[vertNo_2].pos;
			naviMesh.m_cell.push_back(c);
		}
	}
}

/// <summary>
/// AABB���v�Z�B
/// <para>�����o�łȂ��l��ԋp���邽�ߎQ�Ƃɂ��Ȃ����ƁB</para>
/// </summary>
/// <param name="aabb">AABB�i�[�p�B</param>
/// <param name="vMax">�ő咸�_�B</param>
/// <param name="vMin">�ŏ����_�B</param>
/// <returns>AABB�B</returns>
void CalcAABB(AABB& aabb, Vector3& vMax, Vector3& vMin)
{
	//8���_���v�Z��������Ă����B
	//ZUP�Ȃ��Ƃɒ��ӁI�I
	aabb.v[EnFupperLeft] = { vMin.x, vMin.y, vMax.z };
	aabb.v[EnFlowerLeft] = { vMin.x, vMin.y, vMin.z };
	aabb.v[EnFupperRight] = { vMax.x, vMin.y, vMax.z };
	aabb.v[EnFlowerRight] = { vMax.x, vMin.y, vMin.z };
	aabb.v[EnBupperLeft] = { vMin.x, vMax.y, vMax.z };
	aabb.v[EnBlowerLeft] = { vMin.x, vMax.y, vMin.z };
	aabb.v[EnBupperRight] = { vMax.x, vMax.y, vMax.z };
	aabb.v[EnBlowerRight] = { vMax.x, vMax.y, vMin.z };
}

/// <summary>
/// �����ƃZ���̏Փ˔���B
/// </summary>
/// <param name="spoint">�����n�_�B</param>
/// <param name="epoint">�����I�_�B</param>
/// <param name="cell">�Z���B</param>
/// <returns>�Փ˂��Ă���Ȃ�True/���Ă��Ȃ��Ȃ�False�B</returns>
bool IntersectPlaneAndLine(
	Vector3& spoint,
	Vector3& epoint,
	Cell& cell
)
{
	//�Z���̖@�������߂Ă����B
	Vector3 v0 = cell.pos[0];
	Vector3 v1 = cell.pos[1];
	Vector3 v2 = cell.pos[2];
	//AB,BC�B
	Vector3 nom = v1 - v0;
	Vector3 v1Tov2 = v2 - v1;
	//�O�ς��v�Z�B
	nom.Cross(v1Tov2);
	//���ʂ̕�������d(�@���̒����H)�����߂�B
	float d = nom.Length();
	//nom.Normalize();

	//���ʏ�̓_P�����߂�B�����Ă邩�Ȃ��H
	Vector3 P = { nom.x * d, nom.y * d, nom.z * d };

	//PA,PB�����߂Ă����B
	Vector3 PA, PB;
	PA = P - spoint;
	PB = P - epoint;

	//PA PB ���ꂼ��̕��ʖ@���Ɠ���
	float dot_PA, dot_PB;
	dot_PA = PA.Dot(nom);
	dot_PB = PB.Dot(nom);

	//�덷�����Btodo:�������Ȃ����B
	if (abs(dot_PA) < 0.000001) { dot_PA = 0.0; }
	if (abs(dot_PB) < 0.000001) { dot_PB = 0.0; }

	//�����𔻒肵�Ă����B
	if (dot_PA == 0.0f && dot_PB == 0.0f) {
		//���[�����ʏ�Ȃ̂Ō�_�v�Z�s�\�B
		return false;
	}
	else {
		if ((dot_PA >= 0.0f && dot_PB <= 0.0f) ||
			(dot_PA <= 0.0f && dot_PB >= 0.0f)) 
		{
			//���ϕЕ�����,�Е������Ȃ̂Ō������Ă���B
			return true;
		}
		else {
			//�������Ă��Ȃ��B
			return false;
		}
	}
}
/// <summary>
/// �i�r�Q�[�V�����쐬�c�[���B
/// </summary>
/// <param name="argc">�����̐��B</param>
/// <param name="argv">���̓t�@�C���B</param>
/// <returns></returns>
int main(int argc, char* argv[] )
{
	if (argc < 2) {
		//����������Ȃ��̂Ńw���v��\������B
		std::cout << "�i�r�Q�[�V�������b�V�������c�[��\n";
		std::cout << "mknvm.exe tkmFilePath nvmFilePath\n";
		std::cout << "tkmFilePath�E�E�E�i�r�Q�[�V�������b�V���̐�������tk�t�@�C��\n";
		std::cout << "nvmFilePath�E�E�E���������i�r�Q�[�V�������b�V���̃t�@�C���p�X\n";
		return 0;
	}

	//���x���������Btkl�����[�h����Ă�B
	Level level;
	level.Init(argv[1]);
	//�i�r���b�V��
	NaviMesh naviMesh;

	int eraseCount = 0;

	//tkl�t�@�C���̏������Ƃ�tkm�t�@�C����ǂݍ��ށB
	for (int i = 1; i < level.GetTklFile().GetObjectCount(); i++) {
		//��ڂ̓��x���̃��[�g�{�[���Ȃ̂ŏ��O�B
		//�I�u�W�F�N�g���񂷁B
		TkmFile tkmFile;
		//�t�@�C���p�X�`���B
		char filePath[256];
		sprintf_s(filePath, "Assets/modelData/NavSample/%s.tkm", level.GetTklFile().GetObj(i).name.get());
		//tkm�̃��[�h�B
		//���[�h���邱�ƂŒ��_�o�b�t�@�����������B
		tkmFile.Load(filePath);

		//���b�V���̌`�������Ă����B
		//���b�V�����`������̂�NaviMesh���쐬���鏰�Ƃ��݂̂ł����͂��B
		//todo:���܋����I�ɂP�Ԗڂ̃I�u�W�F�N�g��Nav���쐬����悤�ɂ��Ă�̂Œ����B
		if (level.GetTklFile().GetObj(i).no == 1) {
			tkmFile.QueryMeshParts([&](const TkmFile::SMesh& mesh) {
				//16�r�b�g��
				//NavMesh����ł�,
				BuildCellsFromOneMesh(naviMesh, mesh.indexBuffer16Array, mesh);
				BuildCellsFromOneMesh(naviMesh, mesh.indexBuffer32Array, mesh);
				});
		}
		//���_�̍ő�B
		Vector3 vMax = tkmFile.GetMaxVertex();
		//���_�̍ŏ��B
		Vector3 vMin = tkmFile.GetMinVertex();
		//���[�J��AABB�����߂�B
		AABB aabb;
		CalcAABB(aabb, vMax, vMin);

		//AABB�����[���h���W�n�ɂ��邽�߂ɁA
		//���x������ϊ����W�������Ă���B
		Vector3 levelPos = level.GetLevelObj(i).position;
		Quaternion levelRot = level.GetLevelObj(i).rotatatin;
		Vector3 scale = level.GetLevelObj(i).scale;	
		//�ϊ����W���s�񉻂��Ă����B
		Matrix mTrans, mRot, mScale;
		mTrans.MakeTranslation(levelPos);
		mRot.MakeRotationFromQuaternion(levelRot);
		mScale.MakeScaling(scale);
		//���[���h�s������߂�B
		Matrix world = mScale * mRot * mTrans;
		
		//�������烍�[�J��AABB�Ƀ��[���h�s�����Z���Ă����B
		for(int vCount = 0; vCount < EnRectangular_Num; vCount++) {
			//���[���h���W���ɕϊ��B
			aabb.v[vCount].TransformCoord(world);
		}

		//AABB�̐����P�Q�{�����߂Ă����B
		//todo:���̃R�[�h�ɋ~�ς��B
		//��O���B
		aabb.line[0] = { aabb.v[EnFupperLeft] ,aabb.v[EnFlowerLeft] };
		//��O��B
		aabb.line[1] = { aabb.v[EnFupperLeft] ,aabb.v[EnFupperRight] };
		//�����B
		aabb.line[2] = { aabb.v[EnFupperLeft] ,aabb.v[EnBupperLeft] };
		//��O���B
		aabb.line[3] = { aabb.v[EnFlowerLeft] , aabb.v[EnFlowerRight] };
		//��O�E�B
		aabb.line[4] = { aabb.v[EnFupperRight] , aabb.v[EnFlowerRight] };
		//����B
		aabb.line[5] = { aabb.v[EnBupperLeft] , aabb.v[EnBupperRight] };
		//���E
		aabb.line[6] = { aabb.v[EnFupperRight] , aabb.v[EnBupperRight] };
		//
		aabb.line[7] = { aabb.v[EnBupperRight] , aabb.v[EnBlowerRight] };
		//
		aabb.line[8] = { aabb.v[EnFlowerRight] , aabb.v[EnBlowerRight] };

		aabb.line[9] = { aabb.v[EnFlowerLeft] , aabb.v[EnBlowerLeft] };

		aabb.line[10] = { aabb.v[EnBlowerLeft] , aabb.v[EnBlowerRight] };

		aabb.line[11] = { aabb.v[EnBupperLeft] , aabb.v[EnBlowerLeft] };

		for (int cellCount = 0; cellCount < naviMesh.m_cell.size(); cellCount++) {
			//AABB�̐����ƃZ���ɑ΂��āA�Փ˔�����s���B
			for (int lineCount = 0; lineCount < 12; lineCount++) {
				//�����{�����A�Փ˔�����s���B
				//todo:���line��enum��`���ă}�W�b�N�i���o�[�����܂��B
				//�Փ˔���(collision detection)�B
				bool CD = IntersectPlaneAndLine(aabb.line[lineCount].SPoint, aabb.line[lineCount].EPoint, naviMesh.m_cell[cellCount]);
				if (CD == true) {
					//�Փ˂��Ă�����A���̃Z���͍폜����B
					naviMesh.m_cell.erase(naviMesh.m_cell.begin() + lineCount);
					eraseCount++;
				}
			}
		}
	}

	printf("�����ꂽ�Z���̐���%d�ł��B", eraseCount);


	//////////////////////////////////////////////////////////////////////////////
	//�@�����ɂ��邽�߂ɁE�E�E
	//�A�i�r�Q�[�V�������b�V�����m�[�h�Ƃ���BVH�\�z����B
	//�B�����ƎO�p�`�Ƃ̓����蔻��Ƃ̑O�ɁABVH�𗘗p�����啝�ȑ��؂���s���B
	//////////////////////////////////////////////////////////////////////////////
	


	//////////////////////////////////////////////////////////////////////////////
	//�z�u����Ă���I�u�W�F�N�g�ƃZ���̓����蔻����s���āA
	//�Փ˂��Ă���Z���͏�������B
	//�@�z�u����Ă���I�u�W�F�N�g�̃��[�J��AABB������8���_�����߂�B
	//�A���[�J��AABB��8���_�Ƀ��[���h�s�����Z����B
	//�B8���_�̃G�b�W(����)�ƎO�p�`�Ƃ̏Փ˔�����s���āA�Ԃ����Ă���Z���������B
	//�C�����ƎO�p�`�̏Փ˔���́A���ʂ̕��������Ȃ�Ƃ�������ł��܂��B
	//////////////////////////////////////////////////////////////////////////////
	
	////navimesh��AABB����8���_�v�Z���Ă����B
	//Vector3 v[EnRectangular_Num];
	////�ő�A�ŏ����_�i�[�p�B
	//Vector3 vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	//Vector3 vMin = { FLT_MAX,FLT_MAX ,FLT_MAX };
	//for (int cellCount = 0; cellCount < naviMesh.m_cell.size(); cellCount++) {
	//	for (int vecCount = 0; vecCount < 3; vecCount++) {
	//		//navimesh�̐��� + 3�ӕ��񂷁B todo:�}�W�b�N�i���o�[�B�B�B
	//		vMax.Max(naviMesh.m_cell[cellCount].pos[vecCount]);
	//		vMin.Min(naviMesh.m_cell[cellCount].pos[vecCount]);
	//	}
	//}

	//�Z���̗אڏ��̍\�z�B


	//�i�r�Q�[�V�������b�V���̃f�[�^��ۑ��B
	FILE* fp = fopen(argv[2], "wb");
	int i = 0;
	fwrite(&i, sizeof(i), 1, fp);
	fclose(fp);
}