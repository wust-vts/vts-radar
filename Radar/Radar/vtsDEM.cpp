#include"stdafx.h"
#include"vtsDEM.h"
#include<io.h>

using namespace std;

VtsDem vtsDem; //存储所有加载的dem文件

VtsDem::VtsDem()
{
	init();
}

VtsDem::~VtsDem()
{
}

void VtsDem::init(){
	vector<string> files;
	getFiles(VTS_DEM_BASE_PATH, files);
	for (vector<string>::iterator iter = files.begin(); iter != files.end(); iter++){
		VtsDemData vtsDemData(*iter);
		vtsDemDataList.push_back(vtsDemData);
	}
}

//大地坐标x,y,range半径
VtsDemData VtsDem::getVtsRadarUseDem(double x, double y, double range){
	vector<vector<float>> radarUseDem;
	list<VtsDemData>::iterator iter;
	VtsDemData radarUseDemData;
	for (iter = vtsDemDataList.begin(); iter != vtsDemDataList.end(); iter++){
		if (iter->containsPoint(x, y)){
			break;
		}
	}
	//未找到符合条件的DEM
	if (iter == vtsDemDataList.end()){
		return radarUseDemData;
	}
	//找到符合条件的DEM
	VtsDemData & vtsDemData = *iter;
	if (!vtsDemData.isLoadData){
		DWORD begin = GetTickCount();
		vtsDemData.loadFileFast(vtsDemData.filePath, true);
		DWORD end = GetTickCount();
		DWORD timeuse = end - begin;
		int a = 0;
	}
	int demSize = range * 2 / vtsDemData.scale;
	for (int i = 0; i < demSize; i++){
		vector<float> v(demSize);
		radarUseDem.push_back(v);
	}
	const vector<vector<float>>& originDem = vtsDemData.data;
	const int beginX = ((x - range) - vtsDemData.minX) / vtsDemData.scale;
	const int beginY = -((y + range) - vtsDemData.maxY) / vtsDemData.scale;
	int Y = beginY;

	for (int i = 0; i < demSize; i++){
		if (Y < 0){
			Y++;
			continue;
		}
		if (Y >= vtsDemData.row){
			break;
		}
		int X = beginX;
		for (int j = 0; j < demSize; j++){
			if (X >= vtsDemData.col){
				break;
			}
			if (X >= 0){
				radarUseDem[i][j] = originDem[Y][X];
			}
			X++;
		}
		Y++;
	}
	radarUseDemData.filePath = vtsDemData.filePath;
	radarUseDemData.scale = vtsDemData.scale;
	radarUseDemData.data = radarUseDem;
	return radarUseDemData;
}

//获取路径下的文件
void VtsDem::getFiles(std::string path, std::vector<std::string>& files){
	//文件句柄  
	long hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}