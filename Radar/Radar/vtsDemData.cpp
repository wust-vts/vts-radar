#include"stdafx.h"
#include"vtsDEM.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include"PublicMethod.h"

using namespace std;

VtsDemData::VtsDemData(){
	this->filePath = "";
	this->row = 0;
	this->col = 0;
	this->minX = 0;
	this->minY = 0;
	this->maxX = 0;
	this->maxY = 0;
	this->scale = 0;
	this->isLoadData = false;
}

VtsDemData::VtsDemData(const string& file){
	loadFileFast(file, false);
}

VtsDemData::~VtsDemData(){
	clearData();
}

void VtsDemData::clearData(){
	for (vector<vector<float>>::iterator iter = data.begin(); iter != data.end(); iter++){
		iter->clear();
	}
	data.clear();
}

//void VtsDemData::loadFile(const string& file, bool loadData){
//	this->filePath = file;
//	ifstream infile;
//	infile.open(file.data());
//	assert(infile.is_open());
//	string s;
//	getline(infile, s);
//	getline(infile, s);
//	getline(infile, s);
//	getline(infile, s);
//	getline(infile, s);
//	getline(infile, s); //minX
//	this->minX = atof(s.data());
//	getline(infile, s); //maxY
//	this->maxY = atof(s.data());
//	getline(infile, s); //scale
//	this->scale = atof(s.data());
//	getline(infile, s);
//	getline(infile, s); //width
//	this->row = atoi(s.data());
//	getline(infile, s); //height
//	this->col = atoi(s.data());
//	getline(infile, s); //
//	float hscale = atof(s.data());
//
//	//暂定这么计算
//	this->maxX = this->minX + this->col*this->scale;
//	this->minY = this->maxY - this->row*this->scale;
//	this->isLoadData = false;
//
//	if (loadData){
//		clearData();
//		while (getline(infile, s))
//		{
//			vector<string> cols = split(s, " ");
//			assert(this->col == cols.size());
//			data.push_back(vertorSToF(cols, hscale));
//		}
//		assert(this->row == data.size());
//		this->isLoadData = true;
//	}
//	infile.close();
//}

void VtsDemData::loadFileFast(const std::string& file, bool loadData){
	using namespace std;

	//string转wchar_t
	wchar_t * filePath = new wchar_t[file.size() + 1];
	MultiByteToWideChar(CP_ACP, 0, file.c_str(), file.size(), filePath, file.size() * sizeof(wchar_t));
	filePath[file.size()] = 0;

	HANDLE hFile = ::CreateFile(filePath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	// 创建文件映射对象
	HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	assert(hFileMap != NULL);
	// 得到系统分配粒度
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;
	// 得到文件尺寸
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);
	// 关闭文件对象
	CloseHandle(hFile);
	// 偏移地址 
	__int64 qwFileOffset = 0;
	// 块大小
	DWORD dwBlockBytes = 1000 * dwGran;
	if (qwFileSize < 1000 * dwGran){
		dwBlockBytes = (DWORD)qwFileSize;
	}
	LPBYTE lpbMapAddress = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS,
		(DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF),
		dwBlockBytes);
	assert(lpbMapAddress != NULL);

	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sMinX = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sMaxY = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sScale = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sRow = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sCol = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);
	string sHscale = getLine(&lpbMapAddress);
	toNextLetter(&lpbMapAddress);

	this->filePath = file;
	this->minX = atof(sMinX.data());
	this->maxY = atof(sMaxY.data());
	this->scale = atof(sScale.data());
	this->row = atoi(sRow.data());
	this->col = atoi(sCol.data());
	float hscale = atof(sHscale.data());
	//暂定这么计算
	this->maxX = this->minX + this->col*this->scale;
	this->minY = this->maxY - this->row*this->scale;
	this->isLoadData = false;
	if (loadData){
		clearData();
		while (true)
		{
			string s = getLine(&lpbMapAddress);
			if (s.size() == 0){
				break;
			}
			vector<float> cols = getRowData(s, hscale, this->col);
			assert(this->col == cols.size());
			data.push_back(cols);
			toNextLetter(&lpbMapAddress);
		}
		int count = data.size();
		for (; count < this->row; count++){
			data.push_back(vector<float>(this->col));
		}
		assert(this->row == data.size());
		this->isLoadData = true;
	}
	// 撤消文件映像
	UnmapViewOfFile(lpbMapAddress);
	// 关闭文件映射对象句柄
	CloseHandle(hFileMap);
}

std::vector<float> getRowData(std::string sRow, float hscale, size_t size){
	assert(hscale > 0);
	using namespace std;
	const char* cRow = sRow.data();
	char* buffer = new char[10];
	vector<float> result;
	size_t count = 0;
	while ((*cRow) != 0){
		while (((*cRow) < 48 || (*cRow) > 57) && (*cRow) != 0 &&(*cRow)!='-'){
			cRow++;
		}
		if ((*cRow) == 0){
			break;
		}

		const char* p = cRow;
		//0~9
		while (((*p) >= 48 && (*p) <= 57)||(*p=='-')){
			p++;
		}
		if (p == cRow){
			break;
		}
		size_t sz = p - cRow;
		memcpy(buffer, cRow, sz);
		buffer[sz] = 0;
		float height = atof(buffer) / hscale;
		result.push_back(height);
		count++;
		cRow = p;
	}
	for (; count < size; count++){
		result.push_back(0);
	}
	return result;
}

std::string getLine(BYTE** p){
	const BYTE* head = *p;
	while ((**p) != 0 && (**p) != '\r' && (**p) != '\n'){
		(*p)++;
	}
	size_t size = *p - head;
	char* cstr = new char[size + 1];
	memcpy(cstr, head, size);
	cstr[size] = 0;
	std::string str(cstr);
	delete cstr;
	return str;
}

void toNextLetter(BYTE** p){
	while (((**p) < 33 || (**p) > 122) && (**p) != 0){
		(*p)++;
	}
}

bool VtsDemData::containsPoint(double x, double y){
	return x >= minX&&x <= maxX&&y >= minY&&y <= maxY;
}

std::vector<float> vertorSToF(const std::vector<std::string>& vs, float hscale){
	vector<float> vf;
	for (vector<string>::const_iterator iter = vs.begin(); iter != vs.end(); iter++){
		vf.push_back(atof(iter->data()) / hscale);
	}
	return vf;
}