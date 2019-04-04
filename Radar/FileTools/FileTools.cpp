// FileTools.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FileTools.h"
#include<stdio.h>
#include<iostream>
#include<io.h>
#include <conio.h>
#include<vector>
#include"../Radar/QuadTree.h"
#include"../Radar/DemTools.h"
#include"../Radar//PublicMethod.h"
#include<math.h>
#include<fstream>
#include <thread>
#include"../Radar/BitmapEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
using namespace NAMESPACE_DEMTOOLS;



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  更改错误代码以符合您的需要
			_tprintf(_T("错误:  MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO:  更改错误代码以符合您的需要
		_tprintf(_T("错误:  GetModuleHandle 失败\n"));
		nRetCode = 1;
	}
	MainFram_1();
	//CBitmapEx bitmap1,bitmap2;
	//bitmap1.Load(_T("C:/Users/Administrator/Desktop/4.bmp"));
	//bitmap2.Create(900 , 900);
	//bitmap2.Draw(300, 300, 300, 300, bitmap1, 0, 0, 900, 900);
	//bitmap2.Save(_T("C:/Users/Administrator/Desktop/5.bmp"));
	return nRetCode;
}

void MainFram()
{
	while (1)
	{
		printf("-----------------------\n");
		printf("1:设置配置参数；\n");	
		printf("2:分割高程文件；\n");	
		printf("3:重新生成四叉树索引文件；\n");
		printf("");
		printf("ESC:退出\n");
		printf("-----------------------\n");


		bool trueCommand = true;
		bool back = false;
		bool reconfirm = false;
		int ch = _getch();
		printf("%c\n",ch);
		do{
			switch (ch)
			{
			case 27://退出
				back = true;
				trueCommand = true;
				break;
			case 49://"1"
				SetParameters();
				trueCommand = true;
				break;
			case 50://"2"
				CutDemFileFrame();
				trueCommand = true;
				break;
			case 51://"3"
				BuildIndexFile();
				trueCommand = true;
				break;
			default:
				trueCommand = false;
				ch = _getch();
				printf("%c\n",ch);
				break;
			}		
		} while (!trueCommand);
		if (back)
			return;
		
	}
}

void MainFram_1()
{
	while (1)
	{
		printf("-----------------------\n");
		printf("1:输入分割文件；\n");
	//	printf("2:分割高程文件并建立索引；\n");
		printf("ESC:退出\n");
		printf("-----------------------\n");


		bool trueCommand = true;
		bool back = false;
		bool reconfirm = false;
		int ch = _getch();
		printf("%c\n", ch);
		do{
			switch (ch)
			{
			case 27://退出
				back = true;
				trueCommand = true;
				break;
			case 49://"1"
				CutDemFileFrame_1();
				trueCommand = true;
				break;
			//case 50://"2"
			//	CutDemFileFrame();
			//	trueCommand = true;
			//	break;
			default:
				trueCommand = false;
				ch = _getch();
				printf("%c\n", ch);
				break;
			}
		} while (!trueCommand);
		if (back)
			return;

	}
}

//再次确认选项
bool ReConfirm(const char * str)
{
	printf("%s(Y/N)\n",str);
	bool reconfirm = false;
	int ch = _getch();
	printf("%c\n",ch);
	while (ch != 89 && ch != 78 && ch != 121 && ch != 110)
	{
		ch = _getch();
		printf("%c\n",ch);
	}
	if (ch == 89 || ch == 121)
	{
		reconfirm = true;
	}
	return reconfirm;
}

////建立四叉树索引文件
void BuildIndexFile()
{
	QuadTree quadTree;
	/*if (!quadTree.Init())
	{
		printf("配置文件载入失败！请检查目录：../Radar/CONFIGURATION.txt\n");
		return;
	}*/
	vector<string> fileNames;
	CStdioFile inFile, outFile;
	outFile.Open(_T("../Radar/QUADTREE_INDEX.txt"), CFile::modeCreate | CFile::modeWrite);
	getFileNames("../Radar/DEM_CUT", fileNames);
	bool isOK = true;
	for (int i = 0; i < (int)fileNames.size(); i++)
	{
		bool format = true;
		MapRect rect;
		CString str(fileNames[i].c_str());
		inFile.Open(str, CFile::modeRead);
		inFile.ReadString(str);
		if (strcmp(_CStringToCharArray(str), "NSDTF-DEM") != 0)
		{
			format = false;
			isOK = false;
		}
		else
		{
			inFile.ReadString(str);
			inFile.ReadString(str);
			inFile.ReadString(str);
			inFile.ReadString(str);
			inFile.ReadString(str);	//大地X
			double minX = atof(_CStringToCharArray(str));
			inFile.ReadString(str);//大地Y
			double maxY = atof(_CStringToCharArray(str));
			inFile.ReadString(str);//间隔X
			double stepX = atof(_CStringToCharArray(str));
			inFile.ReadString(str);//间隔Y
			double stepY = atof(_CStringToCharArray(str));
			inFile.ReadString(str);//X坐标数
			int numX = atoi(_CStringToCharArray(str));
			inFile.ReadString(str);//Y坐标数
			int numY = atoi(_CStringToCharArray(str));

			rect.minX = minX;
			rect.maxY = maxY;
			rect.maxX = minX + stepX*(numX-1);
			rect.minY = maxY - stepY*(numY-1);
			CString id;
			quadTree.GetID(rect, quadTree.m_root, id);
			outFile.WriteString(id);
			outFile.Write("\n",1);
			outFile.WriteString(CString(fileNames[i].c_str()));
			outFile.Write("\n", 1);
		}
		if (!format)
		{
			printf("%s文件格式错误\n",fileNames[i]);
		}
		inFile.Close();
	}
	if (isOK)
	{
		printf("生成索引成功!\n");
	}
	else
	{
		printf("生成索引失败!\n");
	}
	outFile.Close();
}

void BuileIndexFile_1(CString indexPath, CString fileName)
{
	QuadTree quadtree(indexPath);
	CStdioFile indexFile;
	indexFile.Open(indexPath, CFile::modeReadWrite);
	indexFile.SeekToEnd();
	vector<string> fileNames;
	CString filePath;
	filePath.Format(_T("../Radar/DEM_CUT/%s"), fileName);
	getFileNames(_CStringToCharArray(filePath), fileNames);
	for (int i = 0; i < (int)fileNames.size(); i++)
	{
		ifstream infile(fileNames[i]);
		if (!infile.is_open())
			continue;
		char str[50];
		double minX, minY, maxX, maxY,stepX,stepY;
		int row, col;
		infile >> str >> str >> str >> str >> str >> minX >> maxY >> stepX >> stepY >> row >> col;
		infile.close();
		maxX = minX + stepX*(col - 1);
		minY = maxY - stepY*(row - 1);
		MapRect rect(minX, minY, maxX, maxY);
		CString id;
		quadtree.GetID(rect, quadtree.m_root, id);
		indexFile.WriteString(id);
		indexFile.Write("\n", 1);
		indexFile.WriteString(CString(fileNames[i].c_str()));
		indexFile.Write("\n", 1);
	}
	indexFile.Close();
	cout << "索引生成成功" << endl;
}


//设置配置参数(平面直角坐标)
void SetParameters()
{
	long minX, minY, maxX, maxY;
	cout << "请输入配置参数（minX,minY,maxX,maxY）,该参数为最外层区域的左下、右上角大地坐标值:\n";
	cin >> minX >> minY >> maxX >> maxY;
	long width = maxX - minX;
	long height = maxY - minY;
	cout << "请选择单个最小文件大小：\n";
	cout << "a:" << width << "*" << height << endl;
	cout << "b:" << (long)(width/2) << "*" << (long)(height/2) << endl;
	cout << "c:" << (long)(width / 4) << "*" << (long)(height / 4) << endl;
	cout << "d:" << (long)(width / 8) << "*" << (long)(height / 8) << endl;
	cout << "e:" << (long)(width / 16) << "*" << (long)(height / 16) << endl;
	cout << "f:" << (long)(width / 32) << "*" << (long)(height / 32) << endl;
	cout << "g:" << (long)(width / 64) << "*" << (long)(height / 64) << endl;
	cout << "h:" << (long)(width / 128) << "*" << (long)(height / 128) << endl;
	cout << "i:" << (long)(width / 256) << "*" << (long)(height / 256) << endl;
	cout << "j:" << (long)(width / 512) << "*" << (long)(height / 512) << endl;
	cout << "k:" << (long)(width / 1024) << "*" << (long)(height / 1024) << endl;
	cout << "l:" << (long)(width / 2048) << "*" << (long)(height / 2048) << endl;
	cout << "m:" << (long)(width / 4096) << "*" << (long)(height / 4096) << endl;
	int ch = _getch();
	printf("%c\n",ch);
	long minWidth = 0, minHeight = 0;
	int depth = 0;
	bool format = false;
	do
	{
		if (ch >= 65 && ch <= 77 || ch >= 97 && ch <= 109)
		{
			format = true;
			if (ch >= 65 && ch <= 77)
			{
				depth = ch - 65;
				minWidth = width / pow(2, depth);
				minHeight = height / pow(2, depth);
			}
			else
			{
				depth = ch - 97;
				minWidth = width / pow(2, depth);
				minHeight = height / pow(2, depth);
			}
		}
		else
		{
			ch = _getch();
			printf("%c\n",ch);
			format = false;
		}

	} while (!format);
	cout << "您的选择是：" << minWidth << "*" << minHeight << "，深度：" << depth << endl;

	if (ReConfirm("确认输入？"))
	{
		CStdioFile outFile;
		CString str;
		outFile.Open(_T("../Radar/CONFIGURATION.txt"), CFile::modeCreate | CFile::modeWrite);
		str.Format(_T("%ld\n"), minX);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), minY);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), maxX);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), maxY);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), minWidth);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), minHeight);
		outFile.WriteString(str);
		str.Format(_T("%ld\n"), depth);
		outFile.WriteString(str);
		outFile.Close();
		cout << "设置配置文件成功！\n";
		cout << "请务必在修改配置参数后重新分割高程文件并建立索引!\n";
	}
	else
	{
		if (ReConfirm("是否重新输入配置参数？"))
		{
			SetParameters();
		}
	}
}

//根据分割文件选择配置参数
void ChooseParameters(CString fileName)
{
	double minX, minY, maxX, maxY, stepX, stepY, row, col;
	CString filePath;
//	scanf("%s", fileName);
	filePath.Format(_T("../Radar/DEM_ORIGINAL/%s.dem"), fileName);
	ifstream inFile(filePath);
	if (!inFile.is_open())
	{
		cout <<  _CStringToCharArray(filePath)<< ",该文件不存在！\n";
		return;
	}
	char str[50];
	inFile >> str >> str >> str >> str >> str;
	inFile >> minX >> maxY >> stepX >> stepY >> row >> col;
	inFile.close();
	maxX = minX + stepX*(col-1);
	minY = maxY - stepY*(row-1);

	cout << "请选择单个最小文件大小：\n";
	cout << "a:" << row << "*" << col << endl;
	cout << "b:" << (int)(row / 2 + 0.5) << "*" << (int)(col / 2 + 0.5) << endl;
	cout << "c:" << (int)(row / 4 + 0.5) << "*" << (int)(col / 4 + 0.5) << endl;
	cout << "d:" << (int)(row / 8 + 0.5) << "*" << (int)(col / 8 + 0.5) << endl;
	cout << "e:" << (int)(row / 16 + 0.5) << "*" << (int)(col / 16 + 0.5) << endl;
	cout << "f:" << (int)(row / 32 + 0.5) << "*" << (int)(col / 32 + 0.5) << endl;
	cout << "g:" << (int)(row / 64 + 0.5) << "*" << (int)(col / 64 + 0.5) << endl;
	cout << "h:" << (int)(row / 128 + 0.5) << "*" << (int)(col / 128 + 0.5) << endl;
	cout << "i:" << (int)(row / 256 + 0.5) << "*" << (int)(col / 256 + 0.5) << endl;

	
	int ch = _getch();
	printf("%c\n", ch);
	int minRow = 0, minCol = 0;
	int depth = 0;
	bool format = false;
	do
	{
		if (ch >= 65 && ch <= 73 || ch >= 97 && ch <= 105)
		{
			format = true;
			if (ch >= 65 && ch <= 77)
			{
				depth = ch - 65;
			}
			else
			{
				depth = ch - 97;		
			}
			minRow = row / pow(2, depth) + 0.5;
			minCol = col / pow(2, depth) + 0.5;
		}
		else
		{
			ch = _getch();
			printf("%c\n", ch);
			format = false;
		}

	} while (!format);
	cout << "您的选择是：" << minRow << "*" << minCol << "，深度：" << depth << endl;

	if (ReConfirm("确认输入？"))
	{
		CStdioFile indexFile;
		CString str,indexPath;
		indexPath.Format(_T("../Radar/INDEX/%s_index.txt"), fileName);
		indexFile.Open(indexPath, CFile::modeCreate | CFile::modeWrite);
		str.Format(_T("%f\n"), minX);
		indexFile.WriteString(str);
		str.Format(_T("%f\n"), minY);
		indexFile.WriteString(str);
		str.Format(_T("%f\n"), maxX);
		indexFile.WriteString(str);
		str.Format(_T("%f\n"), maxY);
		indexFile.WriteString(str);
		str.Format(_T("%d\n"), (int)row);
		indexFile.WriteString(str);
		str.Format(_T("%d\n"), (int)col);
		indexFile.WriteString(str);
		str.Format(_T("%f\n"), stepX);
		indexFile.WriteString(str);
		str.Format(_T("%f\n"), stepY);	
		indexFile.WriteString(str);
		str.Format(_T("%d\n"), minRow);
		indexFile.WriteString(str);
		str.Format(_T("%d\n"), minCol);
		indexFile.WriteString(str);
		str.Format(_T("%d\n"), depth);
		indexFile.WriteString(str);
		indexFile.Close();
		cout << "设置索引文件参数成功！\n";


		double minLon, minLat, maxLon, maxLat;
		WebMercatorProjInvcal(minX, minY, &minLon, &minLat);
		WebMercatorProjInvcal(maxX, maxY, &maxLon, &maxLat);
		fstream mainFile("../Radar/INDEX/main.txt",ios::in);
		if (!mainFile.is_open())
		{
			mainFile.open("../Radar/INDEX/main.txt", ios::out);
			mainFile.precision(10);
			mainFile << _CStringToCharArray(fileName) << endl;
			mainFile << minLon << endl;
			mainFile << minLat << endl;
			mainFile << maxLon << endl;
			mainFile << maxLat << endl;
			mainFile << _CStringToCharArray(indexPath) << endl;
			mainFile.close();
		}
		else
		{
			fstream outTemp("../Radar/INDEX/temp.txt", ios::trunc | ios::out);
			if (!outTemp.is_open())
			{
				cout << "创建临时文件失败！\n";
				return;
			}
			char temp_fileName[50], temp_filePath[100];
			double temp_minLon, temp_minLat, temp_maxLon, temp_maxLat;
			while (1)
			{
				mainFile >> temp_fileName >> temp_minLon >> temp_minLat >> temp_maxLon >> temp_maxLat >> temp_filePath;
				if (mainFile.eof())
					break;
				if (strcmp(temp_fileName, _CStringToCharArray(fileName)) == 0)
					continue;
				outTemp.precision(10);
				outTemp << temp_fileName << endl;
				outTemp << temp_minLon << endl;
				outTemp << temp_minLat << endl;
				outTemp << temp_maxLon << endl;
				outTemp << temp_maxLat << endl;
				outTemp << temp_filePath << endl;			
			}
			outTemp.close();
			mainFile.close();
			mainFile.open("../Radar/INDEX/main.txt", ios::out | ios::trunc);
			outTemp.open("../Radar/INDEX/temp.txt", ios::in);
			if (!outTemp.is_open())
			{
				cout << "读取临时文件失败！\n";
				return;
			}
			outTemp.seekg(0);
			while (1)
			{
				outTemp >> temp_fileName >> temp_minLon >> temp_minLat >> temp_maxLon >> temp_maxLat >> temp_filePath;
				if (outTemp.eof())
					break;
				mainFile.precision(10);
				mainFile << temp_fileName << endl;
				mainFile << temp_minLon << endl;
				mainFile << temp_minLat << endl;
				mainFile << temp_maxLon << endl;
				mainFile << temp_maxLat << endl;
				mainFile << temp_filePath << endl;
			}
			mainFile.precision(10);
			mainFile << _CStringToCharArray(fileName) << endl;
			mainFile << minLon << endl;
			mainFile << minLat << endl;
			mainFile << maxLon << endl;
			mainFile << maxLat << endl;
			mainFile << _CStringToCharArray(indexPath) << endl;
			mainFile.close();
			outTemp.close();
			remove("../Radar/INDEX/temp.txt");
		}

		//分割文件
		CutDemFile_2(fileName);
		//建立索引
		BuileIndexFile_1(indexPath,fileName);
	}
	else
	{
		if (ReConfirm("是否重新选择配置参数？"))
		{
			ChooseParameters(fileName);
		}
	}
}


void CutDemFileFrame()
{
	printf("-----------------------\n");
	printf("1:分割单个文件；\n");
	printf("2:分割全部文件；\n");
	printf("ESC:返回\n");
	printf("-----------------------\n");

	bool trueCommand = true;
	bool back = false;
	int ch = _getch();
	printf("%c\n", ch);
	do{
		if (ch == 27)
		{
			back = true;
			trueCommand = true;
		}
		else if (ch == 49)//1
		{
			trueCommand = true;
			cout << "输入文件名(不含后缀名):\n";
			CString fileName;
			scanf("%s", fileName);	
			CString path;
			path.Format(_T("../Radar/DEM_ORIGINAL\\%s.dem"), fileName);
			CutDemFile(path);
		}
		else if (ch == 50)//2
		{
			trueCommand = true;
			if (ReConfirm("确认分割目录../Radar/DEM_ORIGINAL下所有文件？"))
			{
				vector<string> fileNames;
				getFileNames("../Radar/DEM_ORIGINAL", fileNames);
				for (int i = 0; i < fileNames.size(); i++)
				{
					CutDemFile(CString(fileNames[i].c_str()));
				}
			}
		}			
		else
		{
			trueCommand = false;
			ch = _getch();
			printf("%c\n",ch);
		}			
	} while (!trueCommand);
	
}

void CutDemFileFrame_1()
{
	printf("-----------------------\n");
	printf("1:分割单个文件；\n");
	printf("2:分割全部文件；\n");
	printf("ESC:返回\n");
	printf("-----------------------\n");

	bool trueCommand = true;
	bool back = false;
	int ch = _getch();
	printf("%c\n", ch);
	do{
		if (ch == 27)
		{
			back = true;
			trueCommand = true;
		}
		else if (ch == 49)//1
		{
			trueCommand = true;
			cout << "输入文件名(不含后缀名):\n";
			char str[50];
			cin >> str;
			CString fileName(str);
			
			ChooseParameters(fileName);
		}
		else if (ch == 50)//2
		{
			trueCommand = true;
			if (ReConfirm("确认分割目录../Radar/DEM_ORIGINAL下所有文件？"))
			{
				vector<string> fileNames,indexNames;
				getFileNames("../Radar/DEM_ORIGINAL", fileNames);
				getFileNames("../Radar/INDEX", indexNames);
				for (int i = 0; i < indexNames.size(); i++)
					remove(indexNames[i].c_str());
				for (int i = 0; i < fileNames.size(); i++)
				{
					ChooseParameters(_parseFileName(CString(fileNames[i].c_str())));
				}
			}
		}
		else
		{
			trueCommand = false;
			ch = _getch();
			printf("%c\n", ch);
		}
	} while (!trueCommand);
}


//从路径中提取出不带后缀的文件名
CString _parseFileName(CString filePath)
{
	int nPos = filePath.Find('/');
	CString sSubStr = filePath;
	while (nPos)
	{
		sSubStr = sSubStr.Mid(nPos + 1, sSubStr.GetLength() - nPos);  //取'/'右边字符串
		nPos = sSubStr.Find('/');   //函数值返回-1 
		if (nPos == -1)
		{
			nPos = 0;
		}
	}
	CString fileName = sSubStr.Left(sSubStr.Find('.'));
	return fileName;
}

//从路径中提取最右端'/'和‘_’之间的子串
CString _parseFileName_1(CString filePath)
{
	int nPos = filePath.Find('/');
	CString sSubStr = filePath;
	while (nPos)
	{
		sSubStr = sSubStr.Mid(nPos + 1, sSubStr.GetLength() - nPos);  //取'/'右边字符串
		nPos = sSubStr.Find('/');   //函数值返回-1 
		if (nPos == -1)
		{
			nPos = 0;
		}
	}
	CString fileName = sSubStr.Left(sSubStr.Find('_'));
	return fileName;
}


//返回字符first-last之间的子字符串('-'与'.'之间的字符串)
CString _subString(CString str, char first = '-', char last = '.')
{
	int first_pos = str.Find(first);
	int last_pos = str.Find(last);
	CString subStr = str.Mid(first_pos, last_pos);
	return subStr;
}

void CutDemFile_2(CString fileName)
{
	CString indexPath, Ori_filePath, cut_filePath;
	indexPath.Format(_T("../Radar/INDEX/%s_index.txt"), fileName);
	Ori_filePath.Format(_T("../Radar/DEM_ORIGINAL/%s.dem"), fileName);
	cut_filePath.Format(_T("../Radar/DEM_CUT/%s.dem"), fileName);
	ifstream indexFile(indexPath);
	ifstream Ori_file(Ori_filePath);
	if (!Ori_file.is_open())
	{
		cout << "源文件打开失败！\n";
		return;
	}
	if (!indexFile.is_open())
	{
		cout << "索引文件打开失败！\n";
		return;
	}
	MapRect mapRect;
	FileRect fileRect;
	int minRow, minCol, depth;
	double stepX, stepY;
	DemHead head;

	indexFile >> mapRect.minX >> mapRect.minY >> mapRect.maxX >> mapRect.maxY >> fileRect.bottom >> fileRect.right >> stepX >> stepY >> minRow >> minCol >> depth;
	fileRect.bottom--;
	fileRect.right--;

	size_t Ori_size = getFileAllSize(&Ori_file);
	char* buff = new char[Ori_size + 10];
	char* pbuff = buff;
	Ori_file.read(buff, Ori_size);

	get_string(&pbuff, head.dataMark, 20);
	get_string(&pbuff, head.version, 20);
	get_string(&pbuff, head.unit, 20);
	get_string(&pbuff, head.alpha, 20);
	get_string(&pbuff, head.compress, 20);
	get_float(&pbuff, head.minX);
	get_float(&pbuff, head.maxY);
	get_float(&pbuff, head.stepX);
	get_float(&pbuff, head.stepY);
	get_integer(&pbuff, head.row);
	get_integer(&pbuff, head.col);
	get_integer(&pbuff, head.hzoom);
	//分配空间,载入数据

	int *data = (int *)malloc(head.row*head.col*sizeof(int));
	memset(data, 0, head.row*head.col*sizeof(int));
	bool flag = false;
	for (int i = 0; i < head.row; i++)
	{
		for (int j = 0; j < head.col; j++)
		{
			int height;
			if (!get_integer(&pbuff, height))
			{
				flag = true;
				break;
			}
				data[i*head.col + j] = height;
		}
		if (flag)
		{
			cout << "源数据文件不完整!\n";
			break;
		}
	}
	

	//删除同名文件夹
	vector<string> fileNames;
	CString str;
	str.Format(_T("../Radar/DEM_CUT/%s"), fileName);
	getFileNames(_CStringToCharArray(str), fileNames);
	for (int j = 0; j < fileNames.size(); j++)
	{
		CString sameStr1;
		sameStr1 = _parseFileName_1(CString(fileNames[j].c_str()));
		if (sameStr1.Compare(fileName) == 0)
		{
			remove(fileNames[j].c_str());
			printf("删除文件%s成功!\n", fileNames[j].c_str());
		}
	}
	CString removePath;
	removePath.Format(_T("../Radar/DEM_CUT/%s"), fileName);
	_wrmdir(removePath);
	_wmkdir(removePath);

	DWORD d1 = GetTickCount();
	int suffix = 0;
	_findRect(mapRect, fileRect, fileName, data, depth, head, suffix);
	DWORD d2 = GetTickCount();
	DWORD diff = d2 - d1;

	//释放内存
	free(data);
	//关闭文件
	indexFile.close();
	Ori_file.close();
}

void CutDemFile_1(CString fileName)
{
	CString indexPath, Ori_filePath,cut_filePath;
	indexPath.Format(_T("../Radar/INDEX/%s_index.txt"), fileName);
	Ori_filePath.Format(_T("../Radar/DEM_ORIGINAL/%s.dem"), fileName);
	cut_filePath.Format(_T("../Radar/DEM_CUT/%s.dem"), fileName);
	ifstream indexFile(indexPath);
	ifstream Ori_file(Ori_filePath);
	if (!Ori_file.is_open())
	{
		cout << "源文件打开失败！\n";
		return;
	}
	if (!indexFile.is_open())
	{
		cout << "索引文件打开失败！\n";
		return;
	}
	MapRect mapRect;
	FileRect fileRect;
	int minRow, minCol, depth;
	double stepX, stepY;
	DemHead head;

	indexFile >> mapRect.minX >> mapRect.minY >> mapRect.maxX >> mapRect.maxY >> fileRect.bottom >> fileRect.right >> stepX >> stepY >> minRow >> minCol >> depth;	
	Ori_file >> head.dataMark >> head.version >> head.unit >> head.alpha >> head.compress >> head.minX >> head.maxY >> head.stepX >> head.stepY >> head.row >> head.col >> head.hzoom;
	fileRect.bottom--;
	fileRect.right--;

	
	//分配空间,载入数据
	DWORD d1 = GetTickCount();
	int *data = (int *)malloc(head.row*head.col*sizeof(int));
	memset(data, 0, head.row*head.col*sizeof(int));
	for (int i = 0; i < head.row; i++)
	{
		for (int j = 0; j < head.col; j++)
		{
			Ori_file >> data[i*head.col + j];
		}
	}
	DWORD d2 = GetTickCount();
	DWORD diff = d2 - d1;

	//删除同名文件夹
	vector<string> fileNames;
	CString str;
	str.Format(_T("../Radar/DEM_CUT/%s"), fileName);
	getFileNames(_CStringToCharArray(str), fileNames);
	for (int j = 0; j < fileNames.size(); j++)
	{
		CString sameStr1;
		sameStr1 = _parseFileName_1(CString(fileNames[j].c_str()));
		if (sameStr1.Compare(fileName) == 0)
		{
			remove(fileNames[j].c_str());
			printf("删除文件%s成功!\n", fileNames[j].c_str());
		}
	}
	CString removePath;
	removePath.Format(_T("../Radar/DEM_CUT/%s"), fileName);
	_wrmdir(removePath);
	_wmkdir(removePath);


	int suffix = 0;
	_findRect(mapRect, fileRect, fileName, data, depth, head, suffix);

	//释放内存
	free(data);
	//关闭文件
	indexFile.close();
	Ori_file.close();
}

void _findRect(MapRect cut_mapRect, FileRect cut_fileRect,  CString fileName, int *data,int depth,const DemHead head,int &suffix)
{
	if (depth == 0)
		MakeFile(fileName,data,head,cut_mapRect,cut_fileRect,suffix++);
	else
	{
		int child_right =cut_fileRect.left + _roundDown(cut_fileRect.right - cut_fileRect.left);
		int child_left = cut_fileRect.left + _roundUp(cut_fileRect.right - cut_fileRect.left);
		int child_bottom = cut_fileRect.top + _roundDown(cut_fileRect.bottom - cut_fileRect.top);
		int child_top = cut_fileRect.top + _roundUp(cut_fileRect.bottom - cut_fileRect.top);

		//左上
		_findRect(MapRect{cut_mapRect.minX,cut_mapRect.maxY-(child_bottom-cut_fileRect.top)*head.stepY,cut_mapRect.minX+(child_right-cut_fileRect.left)*head.stepX,cut_mapRect.maxY}, 
			FileRect{ cut_fileRect.left, cut_fileRect.top, child_right, child_bottom }, fileName, data, depth - 1, head, suffix);
		//右上
		_findRect(MapRect{ cut_mapRect.maxX-(cut_fileRect.right-child_left)*head.stepX, cut_mapRect.maxY - (child_bottom - cut_fileRect.top)*head.stepY, cut_mapRect.maxX, cut_mapRect.maxY },
			FileRect{ child_left, cut_fileRect.top, cut_fileRect.right, child_bottom }, fileName, data, depth - 1, head, suffix);
		//左下
		_findRect(MapRect{ cut_mapRect.minX, cut_mapRect.minY, cut_mapRect.minX + (child_right - cut_fileRect.left)*head.stepX, cut_mapRect.minY+(cut_fileRect.bottom-child_top)*head.stepY },
			FileRect{ cut_fileRect.left, child_top, child_right, cut_fileRect.bottom }, fileName, data, depth - 1, head, suffix);
		//右下
		_findRect(MapRect{ cut_mapRect.maxX - (cut_fileRect.right - child_left)*head.stepX, cut_mapRect.minY, cut_mapRect.maxX, cut_mapRect.minY + (cut_fileRect.bottom - child_top)*head.stepY },
			FileRect{ child_left, child_top, cut_fileRect.right, cut_fileRect.bottom }, fileName, data, depth - 1, head, suffix);
	}
}

//向上取整
int _roundUp(int a,int b)
{
	int result = a / b;
	if (a%b)
		result++;
	return result;
}

//向下取整
int _roundDown(int a,int b)
{
	return (a / b);
}

void MakeFile(CString fileName,int *data,const DemHead head,const MapRect mapRect,const FileRect fileRect,const int suffix)
{
	CString filePath;
	filePath.Format(_T("../Radar/DEM_CUT/%s/%s_%d.dem"), fileName,fileName, suffix);
	ofstream cutFile(filePath,ios::out|ios::trunc);
	if (!cutFile.is_open())
	{
		cout << "创建文件" << _CStringToCharArray(filePath) << "失败！" << endl;
		return;
	}
	int row = fileRect.bottom - fileRect.top + 1;
	int col = fileRect.right - fileRect.left + 1;
	//写入头部
	cutFile << head.dataMark << endl;
	cutFile << head.version << endl;
	cutFile << head.unit << endl;
	cutFile << head.alpha << endl;
	cutFile << head.compress << endl;
	cutFile <<std::fixed<< mapRect.minX << endl;
	cutFile <<std::fixed<< mapRect.maxY << endl;
	cutFile << head.stepX << endl;
	cutFile << head.stepY << endl;
	cutFile << row << endl;
	cutFile << col << endl;
	cutFile << head.hzoom << endl;
	//写入内容
	DWORD d1 = GetTickCount();
	for (int i = fileRect.top; i <= fileRect.bottom; i++)
	{
		for (int j = fileRect.left; j <= fileRect.right; j++)
		{
			char str[10];
			int size = m_itoa(str,data[i*head.col + j],10);
			cutFile.write(str, size);
			cutFile.write(" ", 1);
		}
		cutFile.write("\n", 1);
	}
	cout << _CStringToCharArray(filePath) << "创建成功!" << endl;
	cutFile.close();
	DWORD d2 = GetTickCount();
	DWORD diff = d2 - d1;
}

void CutDemFile(CString filePath)
{
	char * c_filePath = _CStringToCharArray(filePath);
	ifstream inFile(filePath);
	if (inFile.is_open())
	{
		//载入原文件参数	
		char line1[50], line2[50], line3[50], line4[50], line5[50];
		double fileMinX,fileMinY,fileMaxX,fileMaxY;
		double stepX, stepY;
		int col, row;
		int unit;
		inFile.getline(line1, 100);
		if (strcmp(line1, "NSDTF-DEM") != 0)
		{
			inFile.close();
			printf("%s文件信息有误!\n", c_filePath);
			return;
		}
		inFile.getline(line2, 100);
		inFile.getline(line3, 100);
		inFile.getline(line4, 100);
		inFile.getline(line5, 100);
		inFile >> fileMinX >> fileMaxY >> stepX >> stepY >> row >> col;
		inFile >> unit;
		fileMaxX = fileMinX + stepX*(col-1);
		fileMinY = fileMaxY - stepY*(row-1);
		//打开配置文件
		ifstream configurFile("../Radar/CONFIGURATION.txt");
		if (!configurFile.is_open())
		{
			inFile.close();
			cout << "配置文件载入错误！请检查目录（../Radar/CONFIGURATION.txt）\n";
			return;
		}
		//载入配置参数
		double mainMinX, mainMinY, mainMaxX, mainMaxY;
		int cellFileWidth, cellFileHeight,depth;
		configurFile >> mainMinX >> mainMinY >> mainMaxX >> mainMaxY >> cellFileWidth >> cellFileHeight >> depth;
		configurFile.close();

		//分配内存
		int *data;
		data = (int *)malloc(row*col*sizeof(int));

		//载入原文件数据
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				if (inFile.eof())
				{
					inFile.close();
					cout << c_filePath << "文件已损坏，分割未成功。\n";
					return;
				}
				inFile >> data[i*col+j];
			}
		}

		//删除同名文件
		vector<string> fileNames;
		getFileNames("../Radar/DEM_CUT", fileNames);
		char sameStr2[50];
		m_strcpy2(sameStr2, filePath);
		for (int j = 0; j < fileNames.size(); j++)
		{
			char sameStr1[50];
			m_strcpy1(sameStr1, fileNames[j]);
			if (strcmp(sameStr1, sameStr2) == 0)
			{
				remove(fileNames[j].c_str());
				printf("删除文件%s成功!\n", fileNames[j].c_str());
			}
		}

		vector<MapRect> rects;
		GetRects(MapRect{ mainMinX, mainMinY, mainMaxX, mainMaxY }, MapRect{ fileMinX, fileMinY, fileMaxX, fileMaxY }, rects, depth);
		bool buidFileOK = true;

		for (int i = 0; i < rects.size(); i++)
		{
			int cellFileRow = (double)cellFileHeight / stepY + 1.5;
			int cellFileCol = (double)cellFileWidth / stepX + 1.5;

			//分配小块内存
			int *cellData = (int *)malloc(cellFileRow*cellFileCol*sizeof(int));
			memset(cellData, 0, sizeof(int)*cellFileRow*cellFileCol);


			//计算需要拷贝的文件范围
			int cellStartX, cellStartY, cellEndX, cellEndY;
			int regionStartX, regionStartY, regionEndX, regionEndY;
			if (fileMinX <= rects[i].minX)
			{
				cellStartX = 0;
				regionStartX = (double)(rects[i].minX - fileMinX) / stepX + 0.5;
			}
			else
			{
				regionStartX = 0;
				cellStartX = (double)(fileMinX - rects[i].minX) / stepX + 0.5;
			}
			if (fileMaxY >= rects[i].maxY)
			{
				cellStartY = 0;
				regionStartY = (double)(fileMaxY - rects[i].maxY) / stepY + 0.5;
			}
			else
			{
				regionStartY = 0;
				cellStartY = (double)(rects[i].maxY - fileMaxY) / stepY + 0.5;
			}
			if (fileMaxX >= rects[i].maxX)
			{
				cellEndX = cellFileCol-1;
				regionEndX = col - 1 - (int)((double)(fileMaxX - rects[i].maxX) / stepX + 0.5);
			}
			else
			{
				regionEndX = col - 1;
				cellEndX = cellFileCol - 1 - (int)((double)(rects[i].maxX - fileMaxX) / stepX + 0.5);
			}
			if (fileMinY <= rects[i].minY)
			{
				cellEndY = cellFileRow - 1;
				regionEndY = row - 1 - (int)((double)(rects[i].minY - fileMinY) / stepY + 0.5);
			}
			else
			{
				regionEndY = row - 1;
				cellEndY = cellFileRow - 1 - (int)((double)(fileMinY - rects[i].minY) / stepY + 0.5);
			}
			//将原始文件的部分拷贝到单元文件中
			int csx, csy, rsx, rsy;
			for (csy = cellStartY,rsy = regionStartY; csy <= cellEndY&&rsy<=regionEndY; csy++,rsy++)
			{
				for (csx = cellStartX,rsx = regionStartX; csx <= cellEndX&&rsx<=regionEndX; csx++,rsx++)
				{
					cellData[csy*cellFileCol+csx] = data[rsy*col+rsx];
				}
			}

			

			//写入文件
			char newFilePath[50];
			m_getCutFilePath(newFilePath, sameStr2, i);
			ofstream outFile(newFilePath);
			if (!outFile.is_open())
			{
				printf("创建文件%s失败\n", newFilePath);
				buidFileOK = false;
				break;
			}
			outFile << line1 << endl;
			outFile << line2 << endl;
			outFile << line3 << endl;
			outFile << line4 << endl;
			outFile << line5 << endl;
			outFile <<std::fixed<< rects[i].minX << endl;
			outFile <<std::fixed<< rects[i].maxY << endl;
			outFile << stepX << endl;
			outFile << stepY << endl;
			outFile << cellFileRow << endl;
			outFile << cellFileCol << endl;
			outFile << unit << endl;
			for (int m = 0; m < cellFileRow; m++)
			{
				for (int n = 0; n < cellFileCol; n++)
				{
					outFile << cellData[m*cellFileCol + n];
					if (n == cellFileCol - 1)
						outFile << endl;
					else
						outFile << ' ';
				}
			}
			outFile.close();
			//释放cellData
			free(cellData);
			printf("创建文件%s成功!\n", newFilePath);
		}


		//释放内存
		free(data);

		inFile.close();

		if (buidFileOK)
		{
			cout << "文件" << c_filePath << "分割成功!\n";
		}
		else
		{
			//删除同名文件
			vector<string> fileNames;
			getFileNames("../Radar/DEM_CUT", fileNames);
			char sameStr2[50];
			m_strcpy2(sameStr2, filePath);
			for (int j = 0; j < fileNames.size(); j++)
			{
				char sameStr1[50];
				m_strcpy1(sameStr1, fileNames[j]);
				if (strcmp(sameStr1, sameStr2) == 0)
				{
					remove(fileNames[j].c_str());
					printf("删除文件%s成功!\n", fileNames[j].c_str());
				}
			}
			printf("文件%s分割失败!\n", c_filePath);
		}

	}
	else
	{
		cout << c_filePath << " 文件不存在,请检查目录(../Radar/DEM_ORIGINAL/)!\n";
	}
}

//获取一个文件夹下的所有文件名
void getFileNames(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFileNames(p.assign(path).append("/").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//CString转char *
char* _CStringToCharArray(CString str)
{
	char *ptr;
#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	ptr = new char[len + 1];
	memset(ptr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ptr, len + 1, NULL, NULL);
#else
	ptr = new char[str.GetAllocLength() + 1];
	sprintf(ptr, _T("%s"), str);
#endif
	return ptr;
}

//判断两个MapRect是否有交集
bool m_hasIntersection(MapRect rect1, MapRect rect2)
{
	if (rect1.minX >= rect2.maxX || rect1.maxX <= rect2.minX || rect1.minY >= rect2.maxY || rect1.maxY <= rect2.minY)
		return false;
	else
		return true;
}

//将一块矩形分割成4块等大小矩形
MapRect* _splitMap(MapRect rect)
{
	struct MapRect* box;
	box = (MapRect*)malloc(sizeof(struct MapRect) * 4);
	box[LL].minX = rect.minX;
	box[LL].minY = rect.minY;
	box[LL].maxX = rect.minX + (rect.maxX - rect.minX) / 2;
	box[LL].maxY = rect.minY + (rect.maxY - rect.minY) / 2;
	box[LR].minX = box[LL].maxX;
	box[LR].maxX = rect.maxX;
	box[LR].minY = rect.minY;
	box[LR].maxY = box[LL].maxY;
	box[UL].minX = rect.minX;
	box[UL].maxX = box[LL].maxX;
	box[UL].minY = box[LL].maxY;
	box[UL].maxY = rect.maxY;
	box[UR].minX = box[UL].maxX;
	box[UR].maxX = rect.maxX;
	box[UR].minY = box[UL].minY;
	box[UR].maxY = rect.maxY;
	return box;
}

void GetRects(MapRect mainRect, MapRect fileRect, vector<MapRect>& rects, int depth)
{
	if (depth == 0)
	{
		if (m_hasIntersection(mainRect, fileRect))
		{
			rects.push_back(mainRect);
		}
		return;
	}
	else
	{
		MapRect* sonRects = _splitMap(mainRect);
		if (m_hasIntersection(sonRects[0], fileRect))
			GetRects(sonRects[0], fileRect, rects, depth - 1);
		if (m_hasIntersection(sonRects[1], fileRect))
			GetRects(sonRects[1], fileRect, rects, depth - 1);
		if (m_hasIntersection(sonRects[2], fileRect))
			GetRects(sonRects[2], fileRect, rects, depth - 1);
		if (m_hasIntersection(sonRects[3], fileRect))
			GetRects(sonRects[3], fileRect, rects, depth - 1);
		free(sonRects);
	}
}

//从strSrc中拷贝除-**.dem以前的部分
char * m_strcpy1(char * strDest, const string strSrc)
{
	if (NULL == strDest)
		throw "Invalid argument(s)";
	char * strDestCopy = strDest;
	int i = 0;
	while (strSrc[i++] != '/');
	for (; strSrc[i] != '\0';)
	{
		*strDest = strSrc[i];
		strDest++;
		i++;
		if (strSrc[i] == '-')
			break;
	}
	*strDest = '\0';
	return strDestCopy;
}

//从strSrc中拷贝除.dem以前的部分
char * m_strcpy2(char * strDest, const CString strSrc)
{
	if (NULL == strDest)
		throw "Invalid argument(s)";
	char * strDestCopy = strDest;
	int i = 0;
	while (strSrc[i++] != '/');
	for (; strSrc[i] != '\0';)
	{
		*strDest = strSrc[i];
		strDest++;
		i++;
		if (strSrc[i] == '.')
			break;
	}
	*strDest = '\0';
	return strDestCopy;
}

char* m_getCutFilePath(char * strDest, const char strSrc[], int n)
{
	if (NULL == strDest)
		throw "Invalid argument(s)";
	char * strDestCopy = strDest;
	char headpath[20] = "../Radar/DEM_CUT\\";
	for (int i = 0; headpath[i] != '\0';i++)
	{
		*strDest++ = headpath[i];
	}
	for (int i = 0; strSrc[i] != '\0'; i++)
	{
		*strDest++ = strSrc[i];
	}
	*strDest++ = '-';
	char numStr[10];
	_itoa(n, numStr, 10);
	for (int i = 0; numStr[i] != '\0';i++)
		*strDest++ = numStr[i];
	*strDest++ = '.';
	*strDest++ = 'd';
	*strDest++ = 'e';
	*strDest++ = 'm';
	*strDest = '\0';
	return strDestCopy;
}