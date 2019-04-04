#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include<mutex>


/* һ��������������޻��֣�:

UL(0)   |    UR(1)
--------|-----------
LL(2)   |    LR(3)
���¶Ը��������͵�ö��
*/
typedef enum
{
	UL = 0,
	UR = 1,
	LL = 2,
	LR = 3
}QuadrantEnum;

typedef struct MapRect	//�������
{
	double minX = 0;
	double minY = 0;
	double maxX = 0;
	double maxY = 0;
	MapRect(){};
	MapRect(double minx, double miny, double maxx, double maxy)
	{
		minX = minx;
		minY = miny;
		maxX = maxx;
		maxY = maxy;
	}
	bool equal(const MapRect rect)
	{
		if (rect.minX >= (minX - 1) && rect.minX <= (minX + 1) && rect.minY >= (minY - 1) && rect.minY <= (minY + 1) && rect.maxX >= (maxX - 1)
			&& rect.maxX <= (maxX + 1) && rect.maxY >= (maxY - 1) && rect.maxY <= (maxY + 1))
			return true;
		return false;
	}
}MapRect;

typedef struct FileRect	//���ļ���ԭ�ļ��е�λ��
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
	FileRect(){};
	FileRect(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}
}FileRect;

//typedef struct MapRect	//��γ��
//{
//	double left;
//	double top;
//	double right;
//	double bottom;
//}MapRect;

/*�ռ����MBR��Ϣ*/
//typedef struct SHPMBRInfo
//{
//	int nID;        //�ռ����ID��  
//	MapRect Box;    //�ռ����MBR��Χ����  
//}SHPMBRInfo;

/* �Ĳ����ڵ����ͽṹ */
typedef struct QuadNode
{
	MapRect     Box;            //�ڵ�������ľ�������  
	FileRect	filePos;		//���ļ��ڸ��ļ��е�λ�÷�Χ
	int     nChildCount;        //�ӽڵ����  
	QuadNode  *children[4];     //ָ��ڵ���ĸ�����   
	CString ID;			//ID�� ��λ�ڵ�λ��
	char* fileName;	//�ýڵ�����Ӧ�ļ�����Ҷ�ڵ㣩
}QuadNode;




class QuadTree
{
//����
public:
	QuadNode * m_root = NULL;//���ڵ�
	int m_depth = 0;	//�Ĳ������
	double m_minX = 0;
	double m_minY = 0;
	double m_maxX = 0;
	double m_maxY = 0;
	int m_row = 0;
	int m_col = 0;
	double m_stepX = 0.0;
	double m_stepY = 0.0;
	int m_minCol = 0;	//��С�ļ����
	int m_minRow = 0;	//��С�ļ��߶�
	bool m_isBuildOK = false;//��־���Ƿ񹹽���ɣ����ڶ��̼߳��
	bool m_isFullTree = false;//�Ƿ�Ϊ���Ĳ���
	CString indexFilePath;	//���ڹ�������������ļ�
//����
public:
	QuadTree(){}
	QuadTree(const QuadTree& tree);
	QuadTree(CString indexPath){ Init(indexPath); }
	~QuadTree(){ Destory(); }

	
//	bool Init();
	//��ʼ������Ϣ,�̰߳�ȫ
	bool Init(const CString indexPath);

	//��ʼ���Ĳ����ڵ�  
	QuadNode *InitQuadNode();

	//�������Ĳ�����Ҷ�ڵ㲻�����ļ���Ϣ��
	void CreateFullQuadTree(CString indexPath);

	//�����Ĳ�����ֻ��������Ϣ��Ҷ�ڵ㣩
	void BuildQuadTree(const CString indexPath);

	//��ȡĿ�귶Χ���ļ���Ϣ
	void GetFileName(const MapRect radarArea, std::vector<CString>& fileName);

	//��ȡָ���ռ�����ID  
	void GetID(const MapRect itemRect, QuadNode* root, CString& id);

	//��ȡ������ǰrect���ļ���Χ
	bool GetBoxRect(MapRect &box, const MapRect rect);

	//�ж�СBox�Ƿ�λ�ڴ�Box��
	bool _isSmallInBig(const MapRect bigBox, const MapRect smallBox);

	//�����Ĳ������ͷ�ָ��
	void Destory();

private:
	//��ȡ��Boxλ�ڸ�Box������
	QuadrantEnum _getQuadRant(const MapRect Box[4], const MapRect smallBox);



	//CStringתchar*
	char* _CStringToCharArray(CString str);

	//��һ����ηָ��4��ȴ�С����
	MapRect* _splitMap(const MapRect rect);

	MapRect* _splitMap1(const FileRect rect[4]);

	//��һ��FileRect�ֳ��ĸ�
	FileRect* _splitFile(const FileRect rect);

	//�ж��������������Ƿ����(�������)
	bool _equal(const MapRect rect1, const MapRect rect2);

	//�ж��������������Ƿ��н���
	bool _hasIntersection(const MapRect rect1, const MapRect rect2);

	//�ݹ��ͷ�ָ��
	void _destory(QuadNode* root);

	//����ڵ㣬�̰߳�ȫ
	void Insert(const char* id, QuadNode* root, const CString fileName);

	//�ݹ鹹�����Ĳ�����Ҷ�ڵ㲻�����ļ���Ϣ��
	void _createFullQuadTree(QuadNode* root, const int depth);

	//��ȡ������ǰrect���ļ���Χ
	void _getBoxRect(MapRect &box, const MapRect rect,const QuadNode* root, const int depth,bool flag);

	int _roundUp(int a, int b = 2);
	int _roundDown(int a, int b = 2);





	bool Init_unlock(const CString indexPath);
	void Insert_unlock(const char* id, QuadNode* root, const CString fileName);
	void BuildQuadTree_unlock(const CString indexPath);
	void GetFileName_unlock(const MapRect radarArea, std::vector<CString>& fileName, const QuadNode* root);
	void Destory_unlock();
	void CreateFullQuadTree_unlock(CString indexPath);
	bool GetBoxRect_unlock(MapRect &box, const MapRect rect);
};






#endif