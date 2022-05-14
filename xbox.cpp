#include <windows.h>
#pragma comment(lib, "xinput.lib")
#include <xinput.h>
#include <iostream>
#include <ctime>
using namespace std;
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) 
#define MAX_PLAYERS 4	//maximum of 4 players

void ProcessFaceButtons(const XINPUT_STATE&);
void ProcessPadButtons(const XINPUT_STATE&);
void ProcessShoulderButtons(const XINPUT_STATE&);
void ProcessThumbButtons(const XINPUT_STATE&);
void ProcessTriggers(const XINPUT_STATE&, const int& PlayerID);
void ProcessThumbs(const XINPUT_STATE&, const int& PlayerID);
void ProcessBackAndStartButtons(const XINPUT_STATE&);
void sendchar(char a);
void printstate();
HANDLE hCom;
time_t moveLast = 0;
time_t now = 0;
time_t shootlast = 0;
bool shootingmode = 1;//1 半自动 0全自动
short int shootinterval = 100;//间隔
short int moveinterval = 100;
short direction = 0;//1w 2a 3s 4d 5q 6e 
bool isfire = 0;
bool ishighspeed = 1;
char c = 0;

int main()
{
	cout << "操作说明： " << endl;
	cout << "左摇杆位移   右摇杆云台   肩键旋转" << endl;
	cout << "发射LT   点射b     爬楼梯a" << endl;
	cout << endl;
	hCom = CreateFile("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); // 改COM口
	if (hCom == INVALID_HANDLE_VALUE)
	{
		/*std::cout << "Port unavailable!" << std::endl;
		return 0;*/
	}
	DCB dcb;
	GetCommState(hCom, &dcb);
	dcb.BaudRate = 9600;
	dcb.ByteSize = DATABITS_8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	BOOL br = SetCommState(hCom, &dcb);
	COMMTIMEOUTS cto = { MAXDWORD, MAXDWORD, MAXDWORD, MAXDWORD, MAXDWORD };
	br = SetCommTimeouts(hCom, &cto);
	XINPUT_STATE State;
	int PlayerID = 0;	//0 - first user
	if (XInputGetState(PlayerID, &State) != ERROR_SUCCESS) {	//device not found
		std::cout << "Error: a device is not found for this user: " << PlayerID << " !\n";
	}
	else {	//device is found
		cout << "当前状态：" << endl;
		while (1) {
			if (XInputGetState(PlayerID, &State) == ERROR_SUCCESS) {	//make sure device is connected
				now = clock();
				if (now - moveLast > moveinterval && moveLast != 0 && direction)
				{
					direction = 0;
					moveLast = now;
					sendchar('p');
				}
				if (now - shootlast > shootinterval && shootlast != 0 && shootingmode && isfire)
				{
					isfire = 0;
					shootlast = now;
					sendchar('o');
				}
				ProcessFaceButtons(State);
				//ProcessPadButtons(State);
				ProcessShoulderButtons(State);
				//ProcessThumbButtons(State);
				ProcessTriggers(State, PlayerID);
				ProcessThumbs(State, PlayerID);
				//ProcessBackAndStartButtons(State);
				//printstate();
			}
		}
	}
	return 0;
}
void sendchar(char a)
{
	c = a;
	BYTE byVal = (BYTE)c;
	DWORD dwTransmitted;
	WriteFile(hCom, &byVal, sizeof(byVal), &dwTransmitted, NULL);
}
void printstate()
{
	cout << "行进状态： ";
	switch (direction)
	{
	case 0:
		cout << "停止";
		break;
	case 1:
		cout << "前进";
		break;
	case 2:
		cout << "左";
		break;
	case 3:
		cout << "后退";
		break;
	case 4:
		cout << "右";
		break;
	case 5:
		cout << "左转";
		break;
	case 6:
		cout << "右转";
		break;
	default:
		break;
	}
	cout << " 发射状态： ";
	if (isfire) cout << "开火";
	else  cout << "停火";
	cout << " 速度： ";
	if (ishighspeed) cout << "快";
	else  cout << "慢";
	cout << " 射击模式： ";
	if (shootingmode) cout << "点射";
	else  cout << "连射";
	cout << "            \r";
	Sleep(20);
}


//face buttons
void ProcessFaceButtons(const XINPUT_STATE& State)
{
	//process the face buttons
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
		sendchar('j');
	}

	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
		isfire = 1;
		shootingmode = 1;
		sendchar('k');
	}

	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_X) {
	}

	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
	}
}
//dpad buttons
void ProcessPadButtons(const XINPUT_STATE& State)
{
	//process the dpad buttons
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
		std::cout << "pressing: DPAD_UP\n";
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
		std::cout << "pressing: DPAD_DOWN\n";
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
		std::cout << "pressing: DPAD_LEFT\n";
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
		std::cout << "pressing: DPAD_RIGHT\n";
	}
}
//shoulder buttons
void ProcessShoulderButtons(const XINPUT_STATE& State)
{
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
		moveLast = clock();
		if (direction != 5 )
		{
			sendchar('g');
			direction = 5;
		}
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
		moveLast = clock();
		if (direction != 6 )
		{
			sendchar('h');
			direction = 6;
		}
	}
}
//thumb buttons
void ProcessThumbButtons(const XINPUT_STATE& State)
{
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
		std::cout << "pressing: LEFT_THUMB\n";
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
		std::cout << "pressing: RIGHT_THUMB\n";
	}
}
//process the triggers
void ProcessTriggers(const XINPUT_STATE& State, const int& PlayerID)
{
	XINPUT_VIBRATION Vibration;
	Vibration.wLeftMotorSpeed = 0;
	Vibration.wRightMotorSpeed = 0;

	//left trigger
	if (State.Gamepad.bLeftTrigger > 0) {
		Vibration.wLeftMotorSpeed = (WORD)(65535.0f * (State.Gamepad.bLeftTrigger / 200.0f));	//trigger values: 255 is full pressure, 0 no pressure.		
	}
	//right trigger
	if (State.Gamepad.bRightTrigger > 0) {
		Vibration.wRightMotorSpeed = (WORD)(65535.0f * (State.Gamepad.bRightTrigger / 200.0f));
		isfire = 1;
		shootingmode = 0;
		sendchar('k');
	}
	else if (State.Gamepad.bRightTrigger <= 0 && shootingmode == 0)
	{
		isfire = 0;
		sendchar('o');
	}

	XInputSetState(PlayerID, &Vibration);
}
//process the thumbsticks/joysticks
void ProcessThumbs(const XINPUT_STATE& State, const int& PlayerID )
{

	short LthumbPosX = State.Gamepad.sThumbLX / 10; 
	short LthumbPosY = State.Gamepad.sThumbLY / 10;
	short RthumbPosX = State.Gamepad.sThumbRX / 10;
	short RthumbPosY = State.Gamepad.sThumbRY / 10;
	int Ldistance = LthumbPosX * LthumbPosX + LthumbPosY * LthumbPosY;
	int Rdistance = RthumbPosX * RthumbPosX + RthumbPosY * RthumbPosY;
	//std::cout << "Ldistance: " << Ldistance << std::endl;
	if (Ldistance < 100000) {}
	else if (Ldistance < 6000000 && Ldistance > 100000)
	{
		moveLast = clock();
		double ratio = abs(LthumbPosY / (LthumbPosX != 0 ? LthumbPosX : 1));
		if (ratio > 1)
		{
			if( LthumbPosY > 0 )
			{
				if (direction != 1 || ishighspeed != 0)
				{
					sendchar('q');
					direction = 1;
					ishighspeed = 0;
				}
			}
			else
			{
				if (direction != 3 || ishighspeed != 0)
				{
					sendchar('w');
					direction = 3;
					ishighspeed = 0;
				}
			}
		}
		else if (ratio <=1)
		{
			if( LthumbPosX < 0 )
			{
				if (direction != 2 || ishighspeed != 0)
				{
					sendchar('e');
					direction = 2;
					ishighspeed = 0;
				}
			}
			else 
			{
				if (direction != 4 || ishighspeed != 0)
				{
					sendchar('r');
					direction = 4;
					ishighspeed = 0;
				}
			}

		}
	}
	else if (Ldistance > 6000000)
	{
		moveLast = clock();
		double ratio = abs(LthumbPosY / (LthumbPosX != 0 ? LthumbPosX : 1));
		if (ratio > 1)
		{
			if (LthumbPosY > 0)
			{
				if (direction != 1 || ishighspeed != 1)
				{
					sendchar('t');
					direction = 1;
					ishighspeed = 1;
				}
			}
			else
			{
				if (direction != 3 || ishighspeed != 1)
				{
					sendchar('y');
					direction = 3;
					ishighspeed = 1;
				}
			}
		}
		else if (ratio <= 1)
		{
			if (LthumbPosX < 0)
			{
				if (direction != 2 || ishighspeed != 1)
				{
					sendchar('u');
					direction = 2;
					ishighspeed = 1;
				}
			}
			else 
			{
				if (direction != 4 || ishighspeed != 1)
				{
					sendchar('i');
					direction = 4;
					ishighspeed = 1;
				}
			}

		}
	}
	if (Rdistance < 200000) {}
	else if (Rdistance < 6000000 && Rdistance > 200000)//slow
	{
		double ratio = abs(RthumbPosY / (RthumbPosX != 0 ? RthumbPosX : 1));
		if (ratio > 1)
		{
			if (RthumbPosY > 0)
			{
				sendchar('a');
			}
			else
			{
				sendchar('s');
			}
		}
		else if (ratio <= 1)
		{
			if (RthumbPosX < 0)
			{
				sendchar('d');
			}
			else
			{
				sendchar('f');
			}
		}
	}
	else if (Rdistance > 6000000)
	{
		double ratio = abs(RthumbPosY / (RthumbPosX != 0 ? RthumbPosX : 1));
		if (ratio > 1)
		{
			if (RthumbPosY > 0)
			{
				sendchar('a');
				sendchar('a');
			}
			else
			{
				sendchar('s');
				sendchar('s');
			}
		}
		else if (ratio <= 1)
		{
			if (RthumbPosX < 0)
			{
				sendchar('d');
				sendchar('d');
			}
			else
			{
				sendchar('f');
				sendchar('f');
			}
		}
	}
}
//process back and start button
void ProcessBackAndStartButtons(const XINPUT_STATE& State)
{
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
		std::cout << "pressing: BACK\n";
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_START) {
		std::cout << "pressing: START\n";
	}
}