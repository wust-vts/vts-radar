//CutDemFile_1()和MakeFile()函数中的文件读取和写入方式需要改进  >>和<<运算符太耗时


#pragma once

#include"stdafx.h"
#include "resource.h"
#include<vector>
#include<string.h>
#include<cstring>
#include"../Radar/QuadTree.h"

using  std::vector;
using std::string;

typedef struct DemHead
{
	char dataMark[20];
	char version[20];
	char unit[20];
	char alpha[20];
	char compress[20];
	double minX = 0;
	double maxY = 0;
	double stepX = 0;
	double stepY = 0;
	int row = 0;
	int col = 0;
	int hzoom = 1000;
}DemHead;



void MainFram();
bool ReConfirm(const char* str);
void BuildIndexFile();
void SetParameters();
void CutDemFile(CString fileName);
void getFileNames(string path, vector<string>& files);
char* _CStringToCharArray(CString str);
void CutDemFileFrame();
bool m_hasIntersection(MapRect rect1, MapRect rect2);
MapRect* _splitMap(MapRect rect);
void GetRects(MapRect mainRect,MapRect sonRect, vector<MapRect>& rects, int depth);
char * m_strcpy1(char * strDest, const string strSrc);
char * m_strcpy2(char * strDest, const CString strSrc);
char* m_getCutFilePath(char * strDest,const char strSrc[],int n);
void MainFram_1();
void CutDemFileFrame_1();
void ChooseParameters(CString fileName);
void CutDemFile_1(CString fileName);
void CutDemFile_2(CString fileName);	//改进读文件
void BuileIndexFile_1(CString indexPath,CString fileName);
CString _parseFileName(CString filePath);
void _findRect(MapRect cut_mapRect, FileRect cut_fileRect, CString fileName, int *data, int depth, const DemHead head, int &suffix);
int _roundUp(int a, int b = 2);
int _roundDown(int a, int b = 2);
void MakeFile(CString fileName, int *data, const DemHead head, const MapRect mapRect, const FileRect fileRect, const int suffix);