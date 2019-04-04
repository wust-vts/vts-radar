#include"stdafx.h"
#include"DemTools.h"
#include"PublicMethod.h"
#include<fstream>
#include<thread>
#include<iostream>
using namespace std;
using namespace NAMESPACE_DEMTOOLS;

#define THREAD_NUM_MAX 8	//�����̵߳��������

DemMessage::DemMessage()
{
	data = NULL;
}

DemMessage::DemMessage(const DemMessage& demMessage) :quadTree(demMessage.quadTree), fullTree(demMessage.fullTree), allFile(demMessage.allFile), loadedFile(loadedFile = demMessage.loadedFile)
{
	if (demMessage.data == NULL)
		data = NULL;
	else
	{
		data = (float*)malloc(quadTree.m_row*quadTree.m_col*sizeof(float));
		//��ֵ
		memcpy(data, demMessage.data, quadTree.m_row*quadTree.m_col*sizeof(float));
	//	allFile = demMessage.allFile;
	//	loadedFile = demMessage.loadedFile;
	}
		
}

DemMessage::~DemMessage()
{
//	quadTree.Destory();
//	fullTree.Destory();
	if (data != NULL)
		free(data);
	data = NULL;
}

DemTools::DemTools()
{
	m_pos = -1;
}

DemTools::DemTools(const DemTools& demTools)
{
	m_pos = demTools.m_pos;
	for (int i = 0; i < (int)demTools.m_dem.size(); i++)
	{
		DemMessage* demMessage = new DemMessage(*demTools.m_dem.at(i));
		m_dem.push_back(demMessage);
	}
}

DemTools::~DemTools()
{
	for (int i = 0; i < (int)m_dem.size(); i++)
	{
		(*m_dem.at(i)).~DemMessage();
		delete m_dem.at(i);
	}	
	m_dem.clear();
	m_pos = -1;
}


//��ȡ��ǰ�����������ļ�·��
bool DemMessage::GetAllFile(CString indexPath)
{
	CStdioFile indexFile;
	if (!indexFile.Open(indexPath, CFile::modeRead))
		return false;
	CString str;
	for (int i = 0; i < 11; i++)
		indexFile.ReadString(str);
	while (indexFile.ReadString(str))
	{
		indexFile.ReadString(str);
		allFile.push_back(str);
	}
	indexFile.Close();
	return true;
}


//���ݴ������Ĵ���λ���ж��Ƿ���Ҫ�����µ�DemMessage
void DemTools::ReFresh(double lon, double lat)
{
	double dadiX, dadiY;
	WebMercatorProjCal(lon, lat, dadiX, dadiY);
	for (int i = 0; i < (int)m_dem.size(); i++)
	{
		const QuadTree &quadTree = m_dem.at(i)->quadTree;
		if (PtInRect(dadiX, dadiY, quadTree.m_minX, quadTree.m_minY, quadTree.m_maxX, quadTree.m_maxY))
		{
			//���������ҵ���λ��������ͼ
			m_pos = i;
			return;
		}		
	}
	//��������δ�ҵ���λ��������ͼ�������º�ͼ
	//���������ļ�
	CString indexPath;
	if (GetIndexPath(indexPath, lon, lat))
	{
		//�������������ź�ͼ
		DemMessage* demMessage = new DemMessage;
		demMessage->quadTree.Init(indexPath);
		demMessage->quadTree.BuildQuadTree(indexPath);
		demMessage->fullTree.CreateFullQuadTree(indexPath);
		int row, col;
		row = demMessage->quadTree.m_row;
		col = demMessage->quadTree.m_col;
		//���ٶѿռ�
		demMessage->data = (float*)malloc(row*col*sizeof(float));
		memset(demMessage->data, 0.0f, row*col*sizeof(float));
		//ѹ������
		m_dem.push_back(demMessage);
		//��¼��ǰʹ�õĺ�ͼλ��
		m_pos = (int)m_dem.size() - 1;
	}
	else
	{
		m_pos = -1;
	}
}



//���ݴ����rect��С������data���ݣ����ظ������ļ�
void DemTools::UpdataDem(const MapRect radarRect)
{
	if (m_pos == -1)
		return;
	vector<CString> fileNamesOfRect,fileNamesTobeLoad;
	m_dem.at(m_pos)->quadTree.GetFileName(radarRect, fileNamesOfRect);
	//������Ҫ��ȡ���ļ�
	A_not_in_B(fileNamesOfRect, m_dem.at(m_pos)->loadedFile, fileNamesTobeLoad);
	if ((int)fileNamesTobeLoad.size() == 0)
		return;
	//��ȡ�ļ��������Ƿ������̷ֹ߳���ȡ��
	DWORD sysTime1 = GetTickCount();
	for (int i = 0; i < fileNamesTobeLoad.size(); i++)
	{
		LoadDemFile(fileNamesTobeLoad.at(i), m_dem.at(m_pos));
	}
	//���߳�  �Ѳ���Ҫ
	//int task_sum = (int)fileNamesTobeLoad.size();
	//int task_num = (int)fileNamesTobeLoad.size() / THREAD_NUM_MAX;
	//if (task_sum% THREAD_NUM_MAX != 0)
	//	task_num++;
	//int thread_num = task_sum / task_num;
	//if (task_sum%task_num != 0)
	//	thread_num++;
	//thread th[THREAD_NUM_MAX];
	//int c = 0;
	//int *a = &c;
	//for (int i = 0; i < thread_num; i++)
	//{
	//	int start = i*task_num;
	//	int end = start + task_num - 1;
	//	if (end > task_sum - 1)
	//		end = task_sum - 1;
	////	th[i] = thread(&DemTools::FunB, this, i, start, end);
	//	th[i] = thread(&DemTools::ThreadFunc_LoadDemFile, this, fileNamesTobeLoad, m_dem.at(m_pos), start, end);
	//	th[i].join();
	////	th[i] = thread(LoadDemFile, fileNamesTobeLoad.at(i), m_dem.at(m_pos));
	//}


	DWORD sysTime2 = GetTickCount();
	DWORD diff = sysTime2 - sysTime1;
	float* data = m_dem.at(m_pos)->data;

	int b = 0;
}

void DemTools::ThreadFunc_LoadDemFile(const vector<CString> filePaths, DemMessage* dem, int start, int end)
{
	for (int i = start; i <= end; i++)
		//	LoadDemFile(filePaths.at(i), dem);
		LoadDemFile_memoryMapping(filePaths.at(i), dem);
}

//�÷����ٶ����
void DemTools::LoadDemFile(CString filePath, DemMessage* dem)
{
	ifstream inFile(filePath);
	if (!inFile.is_open())
		return;
	size_t size = getFileAllSize(&inFile);
	char* buff = new char[size+10];
	char* pbuff = buff;
	inFile.read(buff, size);

	char str[50];
	double minX, maxY, stepX, stepY;
	int row, col, hzoom;
	get_string(&pbuff, str, 50);	//1
	if (strcmp(str, "NSDTF-DEM") != 0)
	{
		inFile.close();
		delete buff;
		return;
	}
	get_string(&pbuff, str, 50);	//2
	get_string(&pbuff, str, 50);	//3
	get_string(&pbuff, str, 50);	//4
	get_string(&pbuff, str, 50);	//5
	get_float(&pbuff, minX);	//6
	get_float(&pbuff, maxY);	//7
	get_float(&pbuff, stepX);//8
	get_float(&pbuff, stepY);//9
	get_integer(&pbuff, row);	//10
	get_integer(&pbuff, col);	//11
	get_integer(&pbuff, hzoom);//12
	
	int startX = (minX - dem->quadTree.m_minX) / stepX + 0.5;	//��demFileData�е���ʼX
	int startY = (dem->quadTree.m_maxY - maxY) / stepY + 0.5;
	if (startX + col - 1 >= dem->quadTree.m_col)
		startX--;
	if (startX < 0)
		startX = 0;
	if (startY + row - 1 >= dem->quadTree.m_row)
		startY--;
	if (startY < 0)
		startY = 0;

	float height;
	int pDemFile, pCellFile, qDemFile, qCellFile;
	int demFileRow = dem->quadTree.m_row;
	int demFileCol = dem->quadTree.m_col;

	bool flag = false;

	for (pDemFile = startY, pCellFile = 0; pCellFile < row&&pDemFile<demFileRow; pDemFile++, pCellFile++)
	{
		for (qDemFile = startX, qCellFile = 0; qCellFile < col&&qDemFile<demFileCol; qCellFile++, qDemFile++)
		{
			int height;
			if (!get_integer(&pbuff, height))
			{
				flag = true;
				break;
			}
			if (height != 0)
				dem->data[pDemFile*demFileCol + qDemFile] = (double)height / (double)hzoom;
		}
		if (flag) break;
	}
	dem->loadedFile.push_back(filePath);
	delete[] buff;
	inFile.close();
}

void DemTools::LoadDemFile_freopen(CString filePath, DemMessage* dem)
{

}

//����֤���÷�����ȡ���ݲ�����
void DemTools::LoadDemFile_memoryMapping(CString filePath, DemMessage* dem)
{
	LPCTSTR filename = filePath;
	HANDLE hFile = ::CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "�����ļ�����ʧ��" << endl;
		TRACE("�����ļ�����ʧ��,�������:%drn", GetLastError());
		return;
	}
	// �����ļ�ӳ�����
	HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hFileMap == NULL)
	{
		cout << "�����ļ�ӳ�����ʧ��" << endl;
		TRACE("�����ļ�ӳ�����ʧ��,�������:%drn", GetLastError());
		return;
	}
	// �õ�ϵͳ��������
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;
	// �õ��ļ��ߴ�
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);
	// �ر��ļ�����
	CloseHandle(hFile);
	// ƫ�Ƶ�ַ 
	__int64 qwFileOffset = 0;
	// ���С
	DWORD dwBlockBytes = 1000 * dwGran;
	if (qwFileSize < 1000 * dwGran)
		dwBlockBytes = (DWORD)qwFileSize;

	CString c = _T("");
	CString str = _T("");
	// ӳ����ͼ
	LPBYTE lpbMapAddress = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS,
		(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF),
		dwBlockBytes);
	if (lpbMapAddress == NULL)
	{
		cout << "ӳ���ļ�ӳ��ʧ��" << endl;
		TRACE("ӳ���ļ�ӳ��ʧ��,�������:%drn", GetLastError());
		return;
	}

	// ��ӳ�����ͼ���з���
	//AfxMessageBox(_T("��ʼ����DEM�ļ�"));
	DWORD i = 0;
	//��1��
	while (*(lpbMapAddress + i) != '\r')
	{
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;
	if (str.Compare(_T("NSDTF-DEM")) != 0)
	{
		return;
	}

	//��2��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;

	//��3��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;

	//��4��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;

	//��5��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;

	//��6��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;
	double minX = _ttof(str);

	//��7��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;
	double maxY = _ttof(str);

	//��8��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;
	double stepX = _ttof(str);

	//��9��
	str = "";
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	i = i + 2;
	double stepY = _ttof(str);

	//��10�� ����row
	str = _T("");
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	int row = _ttoi(str);
	i = i + 2;

	//��11�� ����col
	str = _T("");
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	int col = _ttoi(str);
	i = i + 2;



	//��12�� ����hzoom
	str = _T("");
	while (*(lpbMapAddress + i) != '\r'){
		c.Format(_T("%c"), *(lpbMapAddress + i));
		str += c;
		i++;
	}
	int hzoom = _ttoi(str);
	i = i + 2;
	int headLength = i;

	//--------------------ע��----------------
	//���¼�������������ļ�stepX��stepY���
	//----------------------------------------

	//������ļ�������demFileData�е���ʼ��
	int startX = (minX - dem->quadTree.m_minX) / stepX + 0.5;	//��demFileData�е���ʼX
	int startY = (dem->quadTree.m_maxY - maxY) / stepY + 0.5;
	if (startX + col - 1 >= dem->quadTree.m_col)
		startX--;
	if (startX < 0)
		startX = 0;
	if (startY + row - 1 >= dem->quadTree.m_row)
		startY--;
	if (startY < 0)
		startY = 0;
	int endX = startX + col - 1;
	int endY = startY + row - 1;
	int demFileRow = dem->quadTree.m_row;
	int demFileCol = dem->quadTree.m_col;
	for (int p = startY; p <= endY; p++)
	{
		for (int q = startX; q <= endX; q++)
		{
		//	str = _T("");
			wchar_t str[10];
			int j = 0;
			while (*(lpbMapAddress + i) != ' ' && *(lpbMapAddress + i) != '\r')
			{
				str[j++] = *(lpbMapAddress + i);
			//	c.Format(_T("%c"), *(lpbMapAddress + i));
			//	str += c;
				i++;
				if (i >= dwBlockBytes)
				{
					cout << "�����ļ���Χ1" << endl;
					break;
				}
			}
			str[j] = '\0';
			float height = _ttof(str) / hzoom;
			if (height!=0)
				dem->data[p*demFileCol + q] = height;
			i++;
			if (i >= dwBlockBytes)
			{
				cout << "�����ļ���Χ2" << endl;
				break;
			}
		}
		//i++;
		if (i >= dwBlockBytes)
		{
			cout << "�����ļ���Χ3" << endl;
			break;
		}
	}

	// �����ļ�ӳ��
	UnmapViewOfFile(lpbMapAddress);
	qwFileOffset = -1;
	// �ر��ļ�ӳ�������
	CloseHandle(hFileMap);
}

bool DemTools::PtInRect(double dadiX,double dadiY, double minX,double minY,double maxX,double maxY)
{
	if (dadiX >= minX&&dadiX <= maxX&&dadiY >= minY&&dadiY <= maxY)
		return true;
	return false;
}

bool DemTools::PtInRect(double lon, double lat, MapRect rect)
{
	if (lon >= rect.minX&&lon <= rect.maxX&&lat >= rect.minY&&lat <= rect.maxY)
		return true;
	return false;
}

bool DemTools::GetIndexPath(CString &indexPath, const double longitude, const double latitude)
{
	ifstream mainIndex("../Radar/INDEX/main.txt");
	if (!mainIndex.is_open())
		return false;
	char name[20], index[50];
	MapRect rect;
	indexPath = "";
	while (1)
	{
		mainIndex >> name;
		mainIndex >> rect.minX >> rect.minY >> rect.maxX >> rect.maxY;
		mainIndex >> index;
		if (mainIndex.eof())
			break;
		if (PtInRect(longitude, latitude, rect))
		{
			indexPath = index;
			break;
		}
	}
	mainIndex.close();
	if (indexPath == "")
		return false;
	return true;
}

//������A�в�����B��Ԫ�أ���ŵ�C��
void DemTools::A_not_in_B(const vector<CString> A, const vector<CString> B, vector<CString>& C)
{
	C.clear();
	for (int i = 0; i < (int)A.size(); i++)
	{
		bool flag = true;
		for (int j = 0; j < (int)B.size(); j++)
		{
			if (A.at(i).Compare(B.at(j)) == 0)
			{
				flag = false;
				break;
			}			
		}
		if (flag)
			C.push_back(A.at(i));
	}
}