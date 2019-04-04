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
	int mmsi = 0;	//MMSI��
	double heading = 0;	//������(�Ƕ�)
	int posX = 0;	//��Ļ����X
	int posY = 0;	//��Ļ����Y
	int rotateX = 0;	//��ת�����Ļ����X
	int rotateY = 0;	//��ת�����Ļ����Y
	double longitude = 0;	//����
	double latitude = 0;	//γ��
	double dadiX = 0;
	double dadiY = 0;
	double courseThrghWater = 0;	//��ˮ����
	double courseOverGround = 0;	//�Եغ���
	double speedOverGround = 0;		//�Ե��ٶ�
	double speedThrghWather = 0;	//��ˮ�ٶ�
	long map_scale = 0;		//��ͼ�ߴ�
	bool aisSelected = false;	//��AIS����ѡ��
	Point aisPoint[5];		//AIS��ͼʱ���õ㣨0Ϊ���ĵ㣬4Ϊ���ߵ㣬1-3Ϊ�����ϸ��㣩
	//bool aisReceiveCheck = false;	//�Ƿ��յ�AIS��Ϣ
	//bool aisReceiveDoubleCheck = false;	//�Ƿ����AIS��Ϣ���μ��	�������μ��aisReceiveCheck��Ϊfalse����aisReceiveDoubleCheckΪfalse��ȷ�϶�ʧAIS��Ϣ��
	bool isBeArpaRecorded = false;		//�Ƿ�ARPA¼ȡ
	double collisionRadius = 0;			//��ײ�뾶���ף�
	unsigned int aisCheckCount = 0;
}SHIP;

std::vector<SHIP> checkAisShipsLife(std::vector<SHIP> aisShips);
std::vector<SHIP> clearDeadAisShips(std::vector<SHIP> aisShips);