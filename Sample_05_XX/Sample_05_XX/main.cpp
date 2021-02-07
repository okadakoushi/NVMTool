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
class NaviMesh {
public:
	std::vector<Cell>	m_cell;
	void Save(const char* filePath)
	{
		//�t�@�C�����J��
		FILE* fp = fopen(filePath, "wb");
		if (fp) {
			std::intptr_t topAddress = (std::intptr_t)(&m_cell[0]);
			//�Z���̐����������ށB
			int numCell = m_cell.size();
			printf("Save����Z���̐���%d�ł��B", numCell);
			fwrite(&numCell, sizeof(numCell), 1, fp);
			for (auto& cell : m_cell) {
				//���_�f�[�^��ۑ�����B
				fwrite(&cell.pos[0], sizeof(cell.pos[0]), 1, fp);
				fwrite(&cell.pos[1], sizeof(cell.pos[1]), 1, fp);
				fwrite(&cell.pos[2], sizeof(cell.pos[2]), 1, fp);
				//�אڃZ���̃t�@�C�������΃A�h���X���L�^���Ă����B
				for (int i = 0; i < 3; i++) {
					std::intptr_t address;
					if (cell.linkCell[i]) {
						//�אڃZ��������̂ŁA�t�@�C�������΃A�h���X���������ށB
						address = (std::intptr_t)(&cell.linkCell[i]);
						address -= topAddress;
						fwrite(&address, sizeof(address), 1, fp);

					}
					else {
						//�אڃZ�����Ȃ��B
						address = UINT64_MAX;
					}
				}
			}
			fclose(fp);
		}
		
	}
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
	//�����̖��O�B
	enum LineName {
		EnFrontLeft,
		EnBuckLeft,
		EnFrontRight,
		EnBuckRight,
		EnLineCount
	};
	Line line[EnLineCount];	//AABB���\���������4�{�B
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
	//v0����v1�B
	Vector3 nom = v1 - v0;
	//�@���B
	Vector3 v2tov1 = v2 - v1;
	nom.Cross(v2tov1);
	nom.Normalize();

	//�Z�����܂ޖ������ʂƐ����̌�������B
	//�O�p�`�̂ǂ����꒸�_��������̏I�_�ɐL�т�����B
	Vector3 v2ToEnd = epoint - v2;
	//�O�p�`�̂ǂ����꒸�_��������̎n�_�ɐL�т�����B
	Vector3 v2ToStart = spoint - v2;
	//�@���Ɠ��ς��Ƃ邱�ƂŐ��ˉe�x�N�g��������B
	float EndDotN = v2ToEnd.Dot(nom);
	float StartDonN= v2ToStart.Dot(nom);
	//2�̎ˉe�x�N�g���̓��ς����߂�B
	//�������Ă���ꍇ�͎ˉe�x�N�g�����Ⴄ�����������Ă���͂��B
	float v1v2 = EndDotN * StartDonN;
	if (v1v2 < 0.0f) {
		//�������ʂƐ������������Ă���B
		//��_�����߂�B
		//�ӂ̐�Βl���߂�B
		float EndLen = fabsf(EndDotN);
		float StartLen = fabsf(StartDonN);
		//�ӂ̊��������߂�B
		if ((StartLen + EndLen) > 0.0f) {
			float t = EndLen / (StartLen + EndLen);
			//��_�����߂�B
			Vector3 v1v2Vec = epoint - spoint;
			v1v2Vec *= t;
			Vector3 hitPos = epoint + v1v2Vec;

			//��_���|���S�����ɂ��邩�ǂ����B
			//�e���_����@���Ɍ������x�N�g���ƁA���̒��_�Ɍ������x�N�g���Ƃ̊O�ς��v�Z����
			//�O�ό��ʂ�����ς��Ƃ��Đ��̏ꍇ�͏Փ˂��Ă���B
			//�O�ς̓����Ƃ���v1.Cross(v2)�Ƃ������ꍇ��v2��v1�̎��v���̈ʒu�ɂ���ΐ��̒l�B�t�͕��̒l�ɂȂ�B
			//P0
			Vector3 P0toP1 = v1 - v0;
			Vector3 P0toH = hitPos - v0;
			//�O�ρB
			P0toP1.Cross(P0toH);
			//P1
			Vector3 P1toP2 = v2 - v1;
			Vector3 P1toH = hitPos - v1;
			P1toP2.Cross(P0toH);
			//P2
			Vector3 P2toP0 = v0 - v2;
			Vector3 P2toH = hitPos - v2;
			P2toP0.Cross(P0toH);
			//���ς��v�Z�B
			float P0toP1Dot = P0toP1.Dot(P1toP2);
			float P0toP2Dot = P0toP1.Dot(P2toP0);
			//�Փ˔���B
			if (P0toP1Dot * P0toP2Dot > 0) {
				//�Փ˂��Ă��B
				return true;
			}
		}
	}

	return false;

	//���ʂ̕�������d(�@���̒����H)�����߂�B
	/*float d = nom.Length();
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
	}*/
}

void hantei(Vector3& vMax, Vector3& vMin, AABB& aabb, NaviMesh& naviMesh, int cellCount) 
{
	printf("���b�V���ԍ���%d�ł��B\n", cellCount);
	//�������瓖���蔻�肪�����Ȃ��悤�ɁAAABB�����ɋK���I�Ȑ������΂���
	//�����蔻��𔲂��Ȃ��悤�ɂ���B
	//AABB�̕ӂ���Ƃ΂����_�̊Ԋu�B
	const int stride = 5;
	//�e���̐������΂����B
	int xCount = vMax.x - vMin.x / stride;
	int yCount = vMax.y - vMin.y / stride;	//������Z
	int zCount = vMax.z - vMin.z / stride;	//������Y

	//��_�Ƃ��钸�_�B
	const Vector3 baseVertex = aabb.v[EnFlowerLeft];
	//��Ղ�XYZ���̒��_���W�����߂�B
	//��Ւ��_���W���X�g�B���ʂ̔z��ł��ƁA�T�C�Y�m�ۗʂ����ߑł��ɂȂ��ċC���������̂�vector�ł�낤�B
	vector<float> Base_xValue, Base_yValue, Base_zValue;
	////���������Ƃ��B
	//Base_xValue.resize(xCount);
	//Base_yValue.resize(xCount);
	//Base_zValue.resize(xCount);
	for (int vX = 0; vX < xCount; vX++) {
		float X = baseVertex.x + vX * stride;
		Base_xValue.push_back(X);
	}
	for (int vY = 0; vY < yCount; vY++) {
		float Y = baseVertex.y + vY * stride;
		Base_yValue.push_back(Y);
	}
	for (int vZ = 0; vZ < zCount; vZ++) {
		float Z = baseVertex.z + vZ * stride;
		Base_zValue.push_back(Z);
	}
	//�n�_�ƏI�_�B
	Vector3 Start, End;
	End = baseVertex;
	//�S�����v�Z�B
	for (int y = 0; y < yCount; y++) {
		for (int x = 0; x < xCount; x++) {
			for (int z = 0; z < zCount; z++) {
				//�n�_�͑O�̏I�_�B
				Start = End;
				End = { Base_xValue[x] * stride, Base_yValue[y] * stride , Base_zValue[z] * stride };
				bool CD = IntersectPlaneAndLine(Start, End, naviMesh.m_cell[cellCount]);
				if (CD == true) {
					printf("�폜���������������������I�I\n");
					naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
					return;
				}
			}
		}
	}
}
/// <summary>
/// �i�r�Q�[�V�����쐬�c�[���B
/// </summary>
/// <param name="argc">�����̐��B</param>
/// <param name="argv">���̓t�@�C���B</param>
/// <returns></returns>
int main(int argc, char* argv[])
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
			//NavMesh���쐬�����I�u�W�F�N�g�Ƃ͓����蔻������K�v�͂Ȃ��̂ŏ������X�L�b�v�B
			continue;
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
		for (int vCount = 0; vCount < EnRectangular_Num; vCount++) {
			//���[���h���W���ɕϊ��B
			aabb.v[vCount].TransformCoord(world);
		}

		//AABB�̐����P�Q�{�����߂Ă����B
		//todo:���̃R�[�h�ɋ~�ς��B
		//��O���B
		//aabb.line[0] = { aabb.v[EnFupperLeft] ,aabb.v[EnFlowerLeft] };
		////��O��B
		//aabb.line[1] = { aabb.v[EnFupperLeft] ,aabb.v[EnFupperRight] };
		////�����B
		//aabb.line[2] = { aabb.v[EnFupperLeft] ,aabb.v[EnBupperLeft] };
		////��O���B
		//aabb.line[3] = { aabb.v[EnFlowerLeft] , aabb.v[EnFlowerRight] };
		////��O�E�B
		//aabb.line[4] = { aabb.v[EnFupperRight] , aabb.v[EnFlowerRight] };
		////����B
		//aabb.line[5] = { aabb.v[EnBupperLeft] , aabb.v[EnBupperRight] };
		////���E
		//aabb.line[6] = { aabb.v[EnFupperRight] , aabb.v[EnBupperRight] };
		////
		//aabb.line[7] = { aabb.v[EnBupperRight] , aabb.v[EnBlowerRight] };
		////
		//aabb.line[8] = { aabb.v[EnFlowerRight] , aabb.v[EnBlowerRight] };

		//aabb.line[9] = { aabb.v[EnFlowerLeft] , aabb.v[EnBlowerLeft] };

		//aabb.line[10] = { aabb.v[EnBlowerLeft] , aabb.v[EnBlowerRight] };

		//aabb.line[11] = { aabb.v[EnBupperLeft] , aabb.v[EnBlowerLeft] };

		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnFlowerLeft], aabb.v[EnFupperLeft] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnBlowerLeft], aabb.v[EnBupperLeft] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnFlowerRight], aabb.v[EnFupperRight] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnBlowerRight], aabb.v[EnBupperRight] };

		for (int cellCount = naviMesh.m_cell.size(); cellCount > 0; cellCount--) {
			hantei(vMax, vMin, aabb, naviMesh, cellCount);
		}

	//	for (int cellCount = 0; cellCount < naviMesh.m_cell.size(); cellCount++) {
	//		//�Z���̐������[�v�B
	//		for (int lineCount = 0; lineCount < AABB::EnLineCount; lineCount++) {
	//			//�����{�����A�Փ˔�����s���B
	//			//�Փ˔���(collision detection)�B
	//			bool CD = IntersectPlaneAndLine(aabb.line[lineCount].SPoint, aabb.line[lineCount].EPoint, naviMesh.m_cell[cellCount]);
	//			if (CD == true) {
	//				//�Փ˂��Ă�����A���̃Z���͍폜����B
	//				naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
	//				printf("�Z�����폜����܂����B�Z���ԍ���%d�ł��B\n", cellCount);
	//				eraseCount++;
	//			}
	//		}
	//	}
	//}

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

	}
	naviMesh.Save(argv[2]);
}