#include <Wire.h>        //libs
#include <TFLI2C.h>

TFLI2C tflI2C;
int TfL_Succ = 0;

// New
//int16_t TfL_Addr1 = 0x10;	//TfL Lib - first address (usualy left sensor)
//nt16_t TfL_Addr2 = 0x12;	//TfL Lib - second address (usualy mid sensor)
//int16_t TfL_Addr3 = 0x13;	//TfL Lib - third address (usualy right sensor)

// Old
//int16_t TfL_Addr1 = 0x10;	//TfL Lib - first address (usualy left sensor)
int16_t TfL_Addr2 = 0x10;	//TfL Lib - second address (usualy mid sensor)
//int16_t TfL_Addr3 = 0x10;	//TfL Lib - third address (usualy right sensor)

int16_t TfL_AddrDefault = 0x10;

/**
 * @param sumecVer Version of the pcb. Values can be: 3, 4, 5.
 * @brief
 */
void TfL_Setup(uint8_t sumecVer) {
	if (sumecVer < 3 || sumecVer > 5)
	{
		return;
	}
	switch (sumecVer)
	{
	case 3:
		Wire.begin(18, 17, 400000);
		break;
	case 4:
		Wire.begin(17, 18, 400000);
		break;
	case 5:
		Wire.begin(42, 41, 400000);
		break;
	}
}

int TfL_Get(int TfLAddr) {
	int16_t TfL_Dist = 0;
	bool res = tflI2C.getData(TfL_Dist, TfLAddr);
	if (res == 0) TfL_Dist = 666;
	return TfL_Dist;
}

int TfL_IsOnline(int addrToCheck) {
	byte error, address;
	int A1;
	A1=0;

	for(address = 1; address < 127; address++ ) {
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		if(error == 0) {
			if(address==addrToCheck) {
				return 0;
			} else {
				return 1;
			}
		}
	}
}