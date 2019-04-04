#pragma once

#include<vector>
#include<list>

const std::string VTS_DEM_BASE_PATH = ".\\vtsDem";

class VtsDemData
{
public:
	VtsDemData(const std::string& file);
	VtsDemData();
	~VtsDemData();
	void loadFile(const std::string& file, bool loadData);
	void loadFileFast(const std::string& file, bool loadData);
	void clearData();
	bool containsPoint(double x, double y);

public:
	std::string filePath;
	double minX;
	double minY;
	double maxX;
	double maxY;
	double scale;
	int row;
	int col;
	bool isLoadData;
	std::vector<std::vector<float>> data;
};


class VtsDem
{
public:
	VtsDem();
	~VtsDem();
	VtsDemData getVtsRadarUseDem(double x, double y, double range);

private:
	void init();
	void getFiles(std::string path, std::vector<std::string>& files);

private:
	std::list<VtsDemData> vtsDemDataList;
};

std::vector<float> vertorSToF(const std::vector<std::string>& vs, float hscale);

std::string getLine(BYTE**);

void toNextLetter(BYTE**);

std::vector<float> getRowData(std::string sRow, float hscale, size_t size);