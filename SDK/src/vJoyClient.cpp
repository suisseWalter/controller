// vJoyClient.cpp : Simple feeder application with a FFB demo
//


// Monitor Force Feedback (FFB) vJoy device
#include "stdafx.h"
//#include "Devioctl.h"
#include "public.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "Serial.h"
#include "vjoyinterface.h"
#include "Math.h"
#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>
// Default device ID (Used when ID not specified)
#define DEV_ID		1;

// Prototypes
bool getComPort(char* list[10]);
void  CALLBACK FfbFunction(PVOID data);
void  CALLBACK FfbFunction1(PVOID cb, PVOID data);
void setaxis( int axis, int value);
BOOL PacketType2Str(FFBPType Type, LPTSTR Str);
BOOL EffectType2Str(FFBEType Ctrl, LPTSTR Str);
BOOL DevCtrl2Str(FFB_CTRL Type, LPTSTR Str);
BOOL EffectOpStr(FFBOP Op, LPTSTR Str);
int  Polar2Deg(BYTE Polar);
int  Byte2Percent(BYTE InByte);
int TwosCompByte2Int(BYTE in);
std::ifstream myfile,myfile2;
std::string datastring;
int ffb_direction = 0;
int ffb_strenght = 0;
int serial_result = 0;


JOYSTICK_POSITION_V2 iReport; // The structure that holds the full position data


int
__cdecl
_tmain(int argc, _TCHAR* argv[])
{
	printf("Welcome to the serial test app!\n\n");

	
	Serial* SP;
	for (int i = 1; i < 10; i++) {
		std::string str = "COM" + std::to_string(i);
		SP = new Serial(str.c_str());   // adjust as needed
		
		std::cout << "We're connected " << str << std::endl;
		if (SP->IsConnected()) {
			std::cout<<"We're connected "<< str <<std::endl;
			Sleep(20);
			char a[7];
			if (SP->ReadData(a, 2) > 0) {
				printf("we recieved data ");
				break;
			}
			else
			{
				printf( "missconnection, will try again with next one");
				
			}
			
		}
		SP->~Serial();
	}
	if (SP->IsConnected())
		printf("We're connected");
	
	int stat = 0;
	UINT DevID = DEV_ID;
	USHORT X = 0;
	USHORT Y = 0;
	USHORT Z = 0;
	LONG   Btns = 0;
	

	PVOID pPositionMessage;
	UINT	IoCode = LOAD_POSITIONS;
	UINT	IoSize = sizeof(JOYSTICK_POSITION);
	// HID_DEVICE_ATTRIBUTES attrib;
	BYTE id = 1;
	UINT iInterface = 1;

	// Define the effect names
	static FFBEType FfbEffect= (FFBEType)-1;
	LPCTSTR FfbEffectName[] =
	{"NONE", "Constant Force", "Ramp", "Square", "Sine", "Triangle", "Sawtooth Up",\
	"Sawtooth Down", "Spring", "Damper", "Inertia", "Friction", "Custom Force"};

	// Set the target Joystick - get it from the command-line 
	if (argc>1)
		DevID = _tstoi(argv[1]);

	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		_tprintf("Function vJoyEnabled Failed - make sure that vJoy is installed and enabled\n");
		int dummy = getchar();
		stat = - 2;
		goto Exit;
	}
	else
	{
		wprintf(L"Vendor: %s\nProduct :%s\nVersion Number:%s\n", static_cast<TCHAR *> (GetvJoyManufacturerString()), static_cast<TCHAR *>(GetvJoyProductString()), static_cast<TCHAR *>(GetvJoySerialNumberString()));
	};

	// Get the status of the vJoy device before trying to acquire it
	VjdStat status = GetVJDStatus(DevID);

	switch (status)
	{
	case VJD_STAT_OWN:
		_tprintf("vJoy device %d is already owned by this feeder\n", DevID);
		break;
	case VJD_STAT_FREE:
		_tprintf("vJoy device %d is free\n", DevID);
		break;
	case VJD_STAT_BUSY:
		_tprintf("vJoy device %d is already owned by another feeder\nCannot continue\n", DevID);
		return -3;
	case VJD_STAT_MISS:
		_tprintf("vJoy device %d is not installed or disabled\nCannot continue\n", DevID);
		return -4;
	default:
		_tprintf("vJoy device %d general error\nCannot continue\n", DevID);
		return -1;
	};

	// Acquire the vJoy device
	if (!AcquireVJD(DevID))
	{
		_tprintf("Failed to acquire vJoy device number %d.\n", DevID);
		int dummy = getchar();
		stat = -1;
		goto Exit;
	}
	else
		_tprintf("Acquired device number %d - OK\n", DevID);
		


	// Start FFB
#if 1
	BOOL Ffbstarted = FfbStart(DevID);
	if (!Ffbstarted)
	{
		_tprintf("Failed to start FFB on vJoy device number %d.\n", DevID);
		int dummy = getchar();
		stat = -3;
		goto Exit;
	}
	else
		_tprintf("Started FFB on vJoy device number %d - OK\n", DevID);

#endif // 1

	// Register Generic callback function
	// At this point you instruct the Receptor which callback function to call with every FFB packet it receives
	// It is the role of the designer to register the right FFB callback function
	FfbRegisterGenCB(FfbFunction1, NULL);




	// Start endless loop
	// The loop injects position data to the vJoy device
	// If it fails it let's the user try again
	//
	// FFB Note:
	// All FFB activity is performed in a separate thread created when registered the callback function   
	char incomingData[256] = "";			// don't forget to pre-allocate memory
	//printf("%s\n",incomingData);
	int dataLength = 255;
	int readResult = 0;
	unsigned int buttonState = 0;
	short  length;
	char data[256]="",incom2[256]="",datachar[256]="";
	char* endid,* data2;
	
	myfile.open("text.txt");
	myfile2.open("data2");
	int dataint;
	getline(myfile, datastring);
	strcpy(datachar, datastring.c_str());
	myfile >> dataint;
	if (!myfile)std::cout << "unable to open file" << std::endl;
	//std::cout << datastring << " read datafile" << dataint << std::endl;
	Sleep(100);
	SP->WriteData(datachar, sizeof(datachar));
	//std::cout << datachar << std::endl;
	while (1)
		
	{  
		
		std::string axis[] = { "wAxisX","wAxisY","wAxisZ", "wThrottle","Rudder","wAileron","wAxisXRot","wAxisYRot","wAxisZRot","wSlider","wDial","wWheel","wAxisVX","wAxisVY","wAxisVZ","wAxisVBRX","wAxisVBRY","wAxisVBRZ" };

		id = (BYTE)DevID;
		iReport.bDevice = id;
		//----------arduino stuff
		readResult = SP->ReadData(incomingData, dataLength);
		
		// printf("Bytes read: (0 means no data available) %i\n",readResult);
		incomingData[readResult] = 0;
		//printf("%s\n", incomingData);
		
		strcpy(incom2, incomingData);
		endid = strtok(incom2, "<");
		data2 = strtok(incomingData, "\n");
		length=strlen(incomingData);
		//std::cout << incom2 <<" "<<incomingData<<" "<< strlen(incomingData)<< std::endl;
		
		if (incomingData[0] == '-' && incomingData[length-2] == '>') {

			std::cout << " " << " " << incomingData << " " << strlen(incomingData) << std::endl;
			//std::cout << "valid data" << std::endl;
			memcpy(data,incomingData+strlen(endid)+1,length-strlen(endid)-2);
			//printf(incomingData);
			std::cout << data<<" "<<strlen(endid)<<" "<<incomingData << std::endl;
			if (incomingData[1] == 'B'){
				iReport.lButtons = atoi(data);
				std::cout << iReport.lButtons << " set Button " << atoi(data) << std::endl;
				//printf("setbutton\n");
			}
			else if (atoi(endid)) {
				int i = atoi(&(incomingData[1]));
				std::cout << "set axis " << axis[i] << " to " << atoi(data) << std::endl;
				std::string currentaxis = axis[i];
				setaxis(i,32* atoi(data));



			}
			else std::cout <<" "<<"no correct data found "<< data << std::endl;
		}
		//-----------------
		//-------joystick stuff
		// Set destenition vJoy device
		

		// Set position data of 3 first axes
		/*if (Z>35000) Z = 0;
		Z += 200;
		iReport.wAxisZ = 1000;
		iReport.wAxisX = 32000-Z;//*/
		//iReport.wAxisY = Z/2+7000;
		//iReport.wThrottle = Z%10;
		//iReport.bHats = 0 ;
		//iReport.bHatsext1 = 0xFFFFFFFF;
		//std::cout << iReport.bHats << std::endl;

		//iReport.bHatsext1 = 3;

		//iReport.bHatsext1 = 24;
		// Set position data of first 8 buttons
		//Btns = 1<<(Z/1000);
		//iReport.lButtons = Btns+atoi( incomingData);
		//iReport.wSlider = Z;
		//std::cout << Btns<< "test" <<std::endl;

		// Send position data to vJoy device
		pPositionMessage = (PVOID)(&iReport);
		if (!UpdateVJD(DevID, pPositionMessage))
		{
			printf("Feeding vJoy device number %d failed - try to enable device then press enter\n", DevID);
			getchar();
			AcquireVJD(DevID);
		}
		//Sleep(100);
	}

Exit:
	RelinquishVJD(DevID);
	return 0;
}

//find all available com port in a char array
bool getComPort(char* list[10]) {
	char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	bool gotPort = false; // in case the port is not found

	for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
	{
		std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
		DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		// Test the return value and error if any
		if (test != 0) //QueryDosDevice returns zero if it didn't find an object
		{
			std::cout << str << ": " << lpTargetPath << std::endl;
			gotPort = true;
		}

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
		}
	}

	return gotPort;
}

//sets the axis #axis to value
void setaxis(int axis, int value) {
	switch (axis) {
		case 0:
			iReport.wAxisX = value;
			break;
		case 1:
			iReport.wAxisY = value;
			break;
		case 2:
			iReport.wAxisZ = value;
			break;
		case 3:
			iReport.wAxisX = value;
			break;
		case 4:
			iReport.wAxisX = value;
			break;
		case 5:
			iReport.wAxisX = value;
			break;
		case 6:
			iReport.wAxisX = value;
			break;
		case 7:
			iReport.wAxisX = value;
			break;
		case 8:
			iReport.wAxisX = value;
			break;
		


	}

}


// Generic callback function
void CALLBACK FfbFunction(PVOID data)
{
	FFB_DATA * FfbData = (FFB_DATA *)data;
	int size = FfbData->size;
	_tprintf("\nFFB Size %d\n", size);

	_tprintf("Cmd:%08.8X ", FfbData->cmd);
	_tprintf("ID:%02.2X ", FfbData->data[0]);
	_tprintf("Size:%02.2d ", static_cast<int>(FfbData->size - 8));
	_tprintf(" - ");
	for (UINT i = 0; i < FfbData->size - 8; i++)
		_tprintf(" %02.2X", (UINT)FfbData->data);
	_tprintf("\n");
}

void CALLBACK FfbFunction1(PVOID data, PVOID userdata)
{
	// Packet Header
	_tprintf("\n ============= FFB Packet size Size %d =============\n", static_cast<int>(((FFB_DATA *)data)->size));

	/////// Packet Device ID, and Type Block Index (if exists)
#pragma region Packet Device ID, and Type Block Index
	int DeviceID, BlockIndex;
	FFBPType	Type;
	TCHAR	TypeStr[100];

	if (ERROR_SUCCESS == Ffb_h_DeviceID((FFB_DATA *)data, &DeviceID))
		_tprintf("\n > Device ID: %d", DeviceID);
	if (ERROR_SUCCESS == Ffb_h_Type((FFB_DATA *)data, &Type))
	{
		if (!PacketType2Str(Type, TypeStr))
			_tprintf("\n > Packet Type: %d", Type);
		else
			_tprintf("\n > Packet Type: %s", TypeStr);

	}
	if (ERROR_SUCCESS == Ffb_h_EBI((FFB_DATA *)data, &BlockIndex))
		_tprintf("\n > Effect Block Index: %d", BlockIndex);
#pragma endregion


	/////// Effect Report
#pragma region Effect Report
	FFB_EFF_CONST Effect;
	if (ERROR_SUCCESS == Ffb_h_Eff_Report((FFB_DATA *)data, &Effect))
	{
		if (!EffectType2Str(Effect.EffectType, TypeStr))
			_tprintf("\n >> Effect Report: %02x", Effect.EffectType);
		else
			_tprintf("\n >> Effect Report: %s", TypeStr);

		if (Effect.Polar)
		{
			_tprintf("\n >> Direction: %d deg (%02x)", Polar2Deg(Effect.Direction), Effect.Direction);


		}
		else
		{
			_tprintf("\n >> X Direction: %02x", Effect.DirX);
			_tprintf("\n >> Y Direction: %02x", Effect.DirY);
		};

		if (Effect.Duration == 0xFFFF)
			_tprintf("\n >> Duration: Infinit");
		else
			_tprintf("\n >> Duration: %d MilliSec", static_cast<int>(Effect.Duration));

		if (Effect.TrigerRpt == 0xFFFF)
			_tprintf("\n >> Trigger Repeat: Infinit");
		else
			_tprintf("\n >> Trigger Repeat: %d", static_cast<int>(Effect.TrigerRpt));

		if (Effect.SamplePrd == 0xFFFF)
			_tprintf("\n >> Sample Period: Infinit");
		else
			_tprintf("\n >> Sample Period: %d", static_cast<int>(Effect.SamplePrd));


		_tprintf("\n >> Gain: %d%%", Byte2Percent(Effect.Gain));

	};
#pragma endregion
#pragma region PID Device Control
	FFB_CTRL	Control;
	TCHAR	CtrlStr[100];
	if (ERROR_SUCCESS == Ffb_h_DevCtrl((FFB_DATA *)data, &Control) && DevCtrl2Str(Control, CtrlStr))
		_tprintf("\n >> PID Device Control: %s", CtrlStr);

#pragma endregion
#pragma region Effect Operation
	FFB_EFF_OP	Operation;
	TCHAR	EffOpStr[100];
	if (ERROR_SUCCESS == Ffb_h_EffOp((FFB_DATA *)data, &Operation) && EffectOpStr(Operation.EffectOp, EffOpStr))
	{
		_tprintf("\n >> Effect Operation: %s", EffOpStr);
		if (Operation.LoopCount == 0xFF)
			_tprintf("\n >> Loop until stopped");
		else
			_tprintf("\n >> Loop %d times", static_cast<int>(Operation.LoopCount));

	};
#pragma endregion
#pragma region Global Device Gain
	BYTE Gain;
	if (ERROR_SUCCESS == Ffb_h_DevGain((FFB_DATA *)data, &Gain))
		_tprintf("\n >> Global Device Gain: %d", Byte2Percent(Gain));

#pragma endregion
#pragma region Condition
	FFB_EFF_COND Condition;
	if (ERROR_SUCCESS == Ffb_h_Eff_Cond((FFB_DATA *)data, &Condition))
	{
		if (Condition.isY)
			_tprintf("\n >> Y Axis");
		else
			_tprintf("\n >> X Axis");
		_tprintf("\n >> Center Point Offset: %d", TwosCompByte2Int(Condition.CenterPointOffset)*10000/127);
		_tprintf("\n >> Positive Coefficient: %d", TwosCompByte2Int(Condition.PosCoeff)*10000/127);
		_tprintf("\n >> Negative Coefficient: %d", TwosCompByte2Int(Condition.NegCoeff)*10000/127);
		_tprintf("\n >> Positive Saturation: %d", Condition.PosSatur*10000/255);
		_tprintf("\n >> Negative Saturation: %d", Condition.NegSatur*10000/255);
		_tprintf("\n >> Dead Band: %d", Condition.DeadBand*10000/255);
	}
#pragma endregion
#pragma region Envelope
	FFB_EFF_ENVLP Envelope;
	if (ERROR_SUCCESS == Ffb_h_Eff_Envlp((FFB_DATA *)data, &Envelope))
	{
		_tprintf("\n >> Attack Level: %d", Envelope.AttackLevel*10000/255);
		_tprintf("\n >> Fade Level: %d", Envelope.FadeLevel*10000/255);
		_tprintf("\n >> Attack Time: %d", static_cast<int>(Envelope.AttackTime));
		_tprintf("\n >> Fade Time: %d", static_cast<int>(Envelope.FadeTime));
	};

#pragma endregion
#pragma region Periodic
	FFB_EFF_PERIOD EffPrd;
	if (ERROR_SUCCESS == Ffb_h_Eff_Period((FFB_DATA *)data, &EffPrd))
	{
		_tprintf("\n >> Magnitude: %d", EffPrd.Magnitude * 10000 / 255);
		_tprintf("\n >> Offset: %d", TwosCompByte2Int(EffPrd.Offset) * 10000 / 127);
		_tprintf("\n >> Phase: %d", EffPrd.Phase * 3600 / 255);
		_tprintf("\n >> Period: %d", static_cast<int>(EffPrd.Period));
	};
#pragma endregion

#pragma region Effect Type
	FFBEType EffectType;
	if (ERROR_SUCCESS == Ffb_h_EffNew((FFB_DATA *)data, &EffectType))
	{
		if (EffectType2Str(EffectType, TypeStr))
			_tprintf("\n >> Effect Type: %s", TypeStr);
		else
			_tprintf("\n >> Effect Type: Unknown");
	}

#pragma endregion

#pragma region Ramp Effect
	FFB_EFF_RAMP RampEffect;
	if (ERROR_SUCCESS == Ffb_h_Eff_Ramp((FFB_DATA *)data, &RampEffect))
	{
		_tprintf("\n >> Ramp Start: %d", TwosCompByte2Int(RampEffect.Start) * 10000 / 127);
		_tprintf("\n >> Ramp End: %d", TwosCompByte2Int(RampEffect.End) * 10000 / 127);
	};

#pragma endregion

	_tprintf("\n");
	FfbFunction(data);
	_tprintf("\n ====================================================\n");

}


// Convert Packet type to String
BOOL PacketType2Str(FFBPType Type, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Type)
	{
	case PT_EFFREP:
		Str = "Effect Report";
		break;
	case PT_ENVREP:
		Str = "Envelope Report";
		break;
	case PT_CONDREP:
		Str = "Condition Report";
		break;
	case PT_PRIDREP:
		Str = "Periodic Report";
		break;
	case PT_CONSTREP:
		Str = "Constant Force Report";
		break;
	case PT_RAMPREP:
		Str = "Ramp Force Report";
		break;
	case PT_CSTMREP:
		Str = "Custom Force Data Report";
		break;
	case PT_SMPLREP:
		Str = "Download Force Sample";
		break;
	case PT_EFOPREP:
		Str = "Effect Operation Report";
		break;
	case PT_BLKFRREP:
		Str = "PID Block Free Report";
		break;
	case PT_CTRLREP:
		Str = "PID Device Contro";
		break;
	case PT_GAINREP:
		Str = "Device Gain Report";
		break;
	case PT_SETCREP:
		Str = "Set Custom Force Report";
		break;
	case PT_NEWEFREP:
		Str = "Create New Effect Report";
		break;
	case PT_BLKLDREP:
		Str = "Block Load Report";
		break;
	case PT_POOLREP:
		Str = "PID Pool Report";
		break;
	default:
		stat = FALSE;
		break;
	}

	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Convert Effect type to String
BOOL EffectType2Str(FFBEType Type, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Type)
	{
	case ET_NONE:
		stat = FALSE;
		break;
	case ET_CONST:
		Str="Constant Force";
		break;
	case ET_RAMP:
		Str="Ramp";
		break;
	case ET_SQR:
		Str="Square";
		break;
	case ET_SINE:
		Str="Sine";
		break;
	case ET_TRNGL:
		Str="Triangle";
		break;
	case ET_STUP:
		Str="Sawtooth Up";
		break;
	case ET_STDN:
		Str="Sawtooth Down";
		break;
	case ET_SPRNG:
		Str="Spring";
		break;
	case ET_DMPR:
		Str="Damper";
		break;
	case ET_INRT:
		Str="Inertia";
		break;
	case ET_FRCTN:
		Str="Friction";
		break;
	case ET_CSTM:
		Str="Custom Force";
		break;
	default:
		stat = FALSE;
		break;
	};

	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Convert PID Device Control to String
BOOL DevCtrl2Str(FFB_CTRL Ctrl, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Ctrl)
	{
	case CTRL_ENACT:
		Str="Enable Actuators";
		break;
	case CTRL_DISACT:
		Str="Disable Actuators";
		break;
	case CTRL_STOPALL:
		Str="Stop All Effects";
		break;
	case CTRL_DEVRST:
		Str="Device Reset";
		break;
	case CTRL_DEVPAUSE:
		Str="Device Pause";
		break;
	case CTRL_DEVCONT:
		Str="Device Continue";
		break;
	default:
		stat = FALSE;
		break;
	}
	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Convert Effect operation to string
BOOL EffectOpStr(FFBOP Op, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Op)
	{
	case EFF_START:
		Str="Effect Start";
		break;
	case EFF_SOLO:
		Str="Effect Solo Start";
		break;
	case EFF_STOP:
		Str="Effect Stop";
		break;
	default:
		stat = FALSE;
		break;
	}

	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Polar values (0x00-0xFF) to Degrees (0-360)
int Polar2Deg(BYTE Polar)
{
	return ((UINT)Polar*360)/255;
}

// Convert range 0x00-0xFF to 0%-100%
int Byte2Percent(BYTE InByte)
{
	return ((UINT)InByte*100)/255;
}

// Convert One-Byte 2's complement input to integer
int TwosCompByte2Int(BYTE in)
{
	int tmp;
	BYTE inv = ~in;
	BOOL isNeg = in>>7;
	if (isNeg)
	{
		tmp = (int)(inv);
		tmp = -1*tmp;
		return tmp;
	}
	else
		return (int)in;
}
