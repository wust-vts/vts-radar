#pragma once

#include"stdafx.h"
#include<vector>

typedef struct SHIP_INFO{
	double length = 0;
	double width = 0;
	int lengthPixel = 0;
	int widthPixel = 0;
	double breath = 0;
	char ship_Name[10];
	int mmsi = 0;	//MMSI号
	double heading = 0;	//船艏向(角度)
	int posX = 0;	//屏幕坐标X
	int posY = 0;	//屏幕坐标Y
	int rotateX = 0;	//旋转后的屏幕坐标X
	int rotateY = 0;	//旋转后的屏幕坐标Y
	double longitude = 0;	//经度
	double latitude = 0;	//纬度
	double dadiX = 0;
	double dadiY = 0;
	double courseThrghWater = 0;	//对水航向
	double courseOverGround = 0;	//对地航向
	double speedOverGround = 0;		//对地速度
	double speedThrghWather = 0;	//对水速度
	long map_scale = 0;		//地图尺寸
	bool aisSelected = false;	//被AIS功能选中
	Point aisPoint[5];		//AIS绘图时所用点（0为中心点，4为首线点，1-3为三角上各点）
	//bool aisReceiveCheck = false;	//是否收到AIS信息
	//bool aisReceiveDoubleCheck = false;	//是否接收AIS信息二次检查	（若两次检查aisReceiveCheck都为false，则aisReceiveDoubleCheck为false，确认丢失AIS信息）
	bool isBeArpaRecorded = false;		//是否被ARPA录取
	double collisionRadius = 0;			//碰撞半径（米）
	unsigned int aisCheckCount = 0;
}SHIP;

std::vector<SHIP> checkAisShipsLife(std::vector<SHIP> aisShips);
std::vector<SHIP> clearDeadAisShips(std::vector<SHIP> aisShips);