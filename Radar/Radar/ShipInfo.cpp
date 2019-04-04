#include "stdafx.h"
#include "ShipInfo.h"

const unsigned int MAX_AIS_SHIP_CHECK_COUNT = 10; //ais´¬²°¶ªÊ§Ê±¼äÎª10s

std::vector<SHIP> checkAisShipsLife(std::vector<SHIP> aisShips){
	for (std::vector<SHIP>::iterator it = aisShips.begin(); it != aisShips.end(); it++){
		it->aisCheckCount++;
	}
	return aisShips;
}

std::vector<SHIP> clearDeadAisShips(std::vector<SHIP> aisShips){
	for (std::vector<SHIP>::iterator it = aisShips.begin(); it != aisShips.end(); it++){
		if (it->aisCheckCount > MAX_AIS_SHIP_CHECK_COUNT){
			it = aisShips.erase(it);
			break;
		}
	}
	return aisShips;
}