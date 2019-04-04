//����VTS��¼���״���Ϣ

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
	double horl; //�״����������ˮ��

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

	void generateRadarMatrix(double horl);//horl:ˮƽ��

	std::string radarMatrix2String(const std::vector<std::vector<char>>& matrix) const;

	//����VTSRADAR-D��Ϣ
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
	std::string id; //�״���
	double lng = 0; //����,��λ����
	double lat = 0; //γ�ȣ���λ����
	double range = 0; //���̣���λ����
	double height = 0; //�߶ȣ���λ����
	double centerX = 0;//�������X
	double centerY = 0;//�������Y
	double minX = 0;
	double minY = 0;
	double maxX = 0;
	double maxY = 0;
	double scale = 0;//���ȣ���λ����
	double horl = 0; //�״����������ˮ��, ��λ����
	std::list<Host> hosts; //Ŀ��IP����
	std::vector<std::vector<float>> dem;//���صĸ߳�����
	//std::vector<std::vector<char>> radarMatrix; //�״�ͼ�����
public:
	CBitmapEx radarPicOrigin; //ֻ�������ε��״�ͼ��
	CBitmapEx radarPicWithShip; //�������κʹ������״�ͼ��
	//std::mutex mtx_radarPicOrigin;
	//std::mutex mtx_radarPicWithShip;
};


class VtsRadarData{
public:
	VtsRadar vtsRadar;
	double scale; //������,��λ:��
	int x;
	int y;
	std::string data;
};

std::string vtsRadarMapToString(const std::vector<VtsRadar>& vtsRadars);
std::string getVtsRadarSAMsg(const std::vector<VtsRadar>& vtsRadars);

void decodeVtsRadarSMsg(const std::string& msg, std::vector<VtsRadar>& vtsRadars);
/*
* VtsRadar-S���ݸ�ʽ
* VTSRADAR-S;op;id;ip;port;lng;lat;height;range;#
*/

//AR����,���һ��VtsRadar,id�����ڲ����κβ���
void opVtsRadarS_AR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//AT����,��һ��VtsRadar����ź�Ŀ�꣬��id�����ڲ����κβ���;���޸�radar��������
void opVtsRadarS_AT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//AR&AT����,���һ��VtsRadar��ͬʱ���һ��Ĭ�ϵ��ź�Ŀ�꣬id������ֻ����ź�Ŀ��
void opVtsRadarS_ARAT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//ER����,�޸�һ��VtsRadar�Ļ������ݣ�id�������ڲ����κβ���
void opVtsRadarS_ER(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//DT����,ɾ��һ��VtsRadar���ź�Ŀ�꣬id�������ڲ����κβ���
void opVtsRadarS_DT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

//DR����,ɾ��һ��VtsRadar,id�������ڲ����κβ���
void opVtsRadarS_DR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars);

void drawRadarImage(const std::vector<std::vector<char>>&);

std::string byteCompression(const std::vector<char>&);
size_t freCompression(const std::vector<char>& data, char* p);
void getRadarMatrixFromBitmap(std::vector<std::vector<char>>& matrix, const CBitmapEx& bmp, DWORD empty);