//定义VTS记录的雷达信息

#pragma once
#include"ShipInfo.h"
#include<list>
#include<vector>
#include<string>
#include"BitmapEx.h"
#include"Host.h"
#include<mutex>

#define VTSRADAR_D_PORT 9999



class VtsRadar_s{
public:
	std::string op;
	std::string id;
	Host host;
	double lat;
	double lng;
	double height;
	double range;
	double horl; //雷达所处区域的水深

public:
	VtsRadar_s();
	~VtsRadar_s();
	VtsRadar_s& operator =(const VtsRadar_s& other);
};


class VtsRadar
{
public:
	VtsRadar();
	VtsRadar(const VtsRadar& vtsRadar);
	~VtsRadar();
	std::string getId() const;
	void setId(const std::string& id);
	double getLat() const;
	void setLat(double lat);
	double getLng() const;
	void setLng(double lng);
	double getRange() const;
	void setRange(double range);
	double getHeight() const;
	void setHeight(double height);
	double getCenterX() const;
	void setCenterX(double centerX);
	double getCenterY() const;
	void setCenterY(double centerY);
	double getMinX() const;
	double getMinY() const;
	double getMaxX() const;
	double getMaxY() const;
	void setMinX(double minX);
	void setMinY(double minY);
	void setMaxX(double maxX);
	void setMaxY(double maxY);
	double getScale() const;
	void setScale(double scale);
	std::vector<std::vector<float>> getDem() const;
	void setDem(const std::vector<std::vector<float>>& dem);
	//std::vector<std::vector<char>> getRadarMatrix() const;
	//void setRadarMatrix(const std::vector<std::vector<char>>& radarMatrix);
	std::list<Host> getHosts() const;
	double getHorl() const;
	void setHorl(double horl);
	void removeHost(Host host);
	void clearHosts();
	bool containsHost(const Host& host) const;
	void insertHostIfNotContains(const Host& host);

	std::string toString() const;

	VtsRadar& operator =(const VtsRadar& other);

	void generateRadarMatrix(double horl);//horl:水平线

	std::string radarMatrix2String(const std::vector<std::vector<char>>& matrix) const;

	//发送VTSRADAR-D消息
	void sendVtsRadarData() const;

	void sendData(const std::string& msg, const Host& host) const;

	void generateRadarPic();

	void generateRadarWithShipPic(std::vector<SHIP> ships);

	bool isShipInRadar(const SHIP& ship)const;

private:
	std::vector<float> getScanLineDem(const std::vector<std::vector<float>>& dem, double angle, int radius, int centerX, int centerY);
	std::vector<char> getScanLineMatrix(const std::vector<float>& scanLineDem, double scale, double horl, double radarHeight);
	void getScanLinePoints(PointF *p, int centerX, int centerY, int x, int y, int w, int h, int maxL);
	void getScanLinePixel(bool ** bcheck, PointF* p, const std::vector<std::vector<float>>& dem, int centerX, int centerY,
		int maxL, double horl, double radarHeight, double Rmax, double scale, double gain, CBitmapEx & bitmapEx);
	void drawShipOnBmp(CBitmapEx* bmp, const SHIP& ship, DWORD color);
private:
	std::string id; //雷达编号
	double lng = 0; //经度,单位：度
	double lat = 0; //纬度，单位：度
	double range = 0; //量程，单位：米
	double height = 0; //高度，单位：米
	double centerX = 0;//大地坐标X
	double centerY = 0;//大地坐标Y
	double minX = 0;
	double minY = 0;
	double maxX = 0;
	double maxY = 0;
	double scale = 0;//精度，单位：米
	double horl = 0; //雷达所处区域的水深, 单位：米
	std::list<Host> hosts; //目标IP集合
	std::vector<std::vector<float>> dem;//加载的高程数据
	//std::vector<std::vector<char>> radarMatrix; //雷达图像矩阵
public:
	CBitmapEx radarPicOrigin; //只包含地形的雷达图像
	CBitmapEx radarPicWithShip; //包含地形和船舶的雷达图像
	//std::mutex mtx_radarPicOrigin;
	//std::mutex mtx_radarPicWithShip;
};


class VtsRadarData{
public:
	VtsRadar vtsRadar;
	double scale; //比例尺,单位:米
	int x;
	int y;
	std::string data;
};

std::string vtsRadarMapToString(const std::vector<VtsRadar>& vtsRadars);
std::string getVtsRadarSAMsg(const std::vector<VtsRadar>& vtsRadars);

void decodeVtsRadarSMsg(const std::string& msg, std::vector<VtsRadar>& vtsRadars);
/*
* VtsRadar-S数据格式
* VTSRADAR-S;op;id;ip;port;lng;lat;height;range;#
*/

//AR操作,添加一个VtsRadar,id若存在不做任何操作
void opVtsRadarS_AR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//AT操作,给一个VtsRadar添加信号目标，若id不存在不做任何操作;不修改radar基本属性
void opVtsRadarS_AT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//AR&AT操作,添加一个VtsRadar的同时添加一个默认的信号目标，id若存在只添加信号目标
void opVtsRadarS_ARAT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//ER操作,修改一个VtsRadar的基本数据，id若不存在不做任何操作
void opVtsRadarS_ER(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//DT操作,删除一个VtsRadar的信号目标，id若不存在不做任何操作
void opVtsRadarS_DT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//DR操作,删除一个VtsRadar,id若不存在不做任何操作
void opVtsRadarS_DR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

void drawRadarImage(const std::vector<std::vector<char>>&);

std::string byteCompression(const std::vector<char>&);
size_t freCompression(const std::vector<char>& data, char* p);
void getRadarMatrixFromBitmap(std::vector<std::vector<char>>& matrix, const CBitmapEx& bmp, DWORD empty);