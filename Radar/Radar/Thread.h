#pragma once
#include "BitmapEx.h"
#include<vector>
#include"vtsRadar.h"

UINT ThreadReceiveVtsRadarSetting(LPVOID lpParam);
void ThreadSendVtsRadarData(std::vector<VtsRadar> vtsRadars);
UINT ThreadSendVtsRadarData(LPVOID lpParam);
UINT ThreadGenerateVtsRadarPic(LPVOID lpParam);
UINT ThreadVtsRadarBitMap2Bmp(LPVOID lpParam);

//UINT ThreadCalculateARPARecord(LPVOID lpParam);
//UINT ThreadLoadDemFile(LPVOID lpParam);
//UINT ThreadDrawDemBMP(LPVOID lpParam);
//UINT ThreadDrawDemBMP_1(LPVOID lpParam);

//UINT ThreadLogin(LPVOID lpParam);
//UINT ThreadReceive(LPVOID lpParam);