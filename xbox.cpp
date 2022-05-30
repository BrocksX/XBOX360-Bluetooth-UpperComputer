#include <windows.h>
#pragma comment(lib, "xinput.lib")
#include <xinput.h>
#include <iostream>
#include <ctime>
using namespace std;
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) 
#define MAX_PLAYERS 4	//maximum of 4 players
void set_motorframe(const XINPUT_STATE&, const int& PlayerID);
void set_steeringframe(const XINPUT_STATE&, const int& PlayerID);
void set_otherframe(const XINPUT_STATE&, const int& PlayerID);
void sendchar(int a);
HANDLE hCom;
int main()
{
	cout << "操作说明： " << endl;
	cout << "左摇杆位移   右摇杆云台   扳机键旋转" << endl;
	cout << "发射RB   反转LB   撑杆 B X" << endl;
	cout << endl;
	hCom = CreateFile("\\\\.\\COM10", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); // 改COM口
	if (hCom == INVALID_HANDLE_VALUE) {
		std::cout << "Port unavailable!" << std::endl;
	}
	DCB dcb;
	GetCommState(hCom, &dcb);
	dcb.BaudRate = 115200;
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
			if (XInputGetState(PlayerID, &State) == ERROR_SUCCESS) {//make sure device is connected
				set_motorframe(State, PlayerID);
				set_otherframe(State, PlayerID);
				Sleep(10);
				set_steeringframe(State, PlayerID);
			}
		}
	}
	return 0;
}
void sendchar(int a)
{
	char c = a;
	BYTE byVal = (BYTE)c;
	DWORD dwTransmitted;
	WriteFile(hCom, &byVal, sizeof(byVal), &dwTransmitted, NULL);
}
void set_otherframe(const XINPUT_STATE& State, const int& PlayerID)
{
	XINPUT_VIBRATION Vibration;
	Vibration.wLeftMotorSpeed = 0;
	Vibration.wRightMotorSpeed = 0;
	int otherframe = 64;
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) //开火
	{
		otherframe += 48;
		Vibration.wRightMotorSpeed = (WORD)(65535.0f);	//trigger values: 255 is full pressure, 0 no pressure.	
	}
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)     otherframe += 32; //反转
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_B) 		otherframe += 12; //撑杆正传
	if (State.Gamepad.wButtons & XINPUT_GAMEPAD_X)  otherframe += 8; //撑杆反传
	if ((otherframe & 48) == 48) cout << "  开火";
	else if ((otherframe & 48) == 32) cout << "  反转";
	else cout << "  停火";
	if ((otherframe & 12) == 12) cout << "  撑杆正传";
	else if ((otherframe & 12) == 8) cout << "  撑杆反传";
	sendchar(otherframe);
	XInputSetState(PlayerID, &Vibration);
	cout << "            \r";
}
void set_motorframe(const XINPUT_STATE& State, const int& PlayerID)
{
	short LPosX = State.Gamepad.sThumbLX / 400;//死区+-10   75 40 20
	short LPosY = State.Gamepad.sThumbLY / 400;//死区+-10   1000一档
	short LeftTrigger = State.Gamepad.bLeftTrigger;
	short RightTrigger = State.Gamepad.bRightTrigger;
	int Ldistance = (LPosX * LPosX + LPosY * LPosY);//死区100   3000 6000
	//std::cout << "LPosX: " << LPosX << "  Ldistance: "<< Ldistance <<std::endl;
	int motorframe1 = 128;
	int motorframe2 = 192;
	int vx = 0, vy = 0, rotation = 0;
	int ru = 0, ld = 0, lu = 0, rd = 0;
	if (LeftTrigger < 5 && RightTrigger < 5)
	{
		if (abs(LPosX) > 75)
		{
			vx = LPosX > 0 ? 3 : -3;
			vy = 0;
		}
		else if (abs(LPosX) > 40)
		{
			vx = LPosX > 0 ? 2 : -2;
			if (Ldistance > 6000) vy = LPosY > 0 ? 1 : -1;
		}
		else if (abs(LPosX) > 25)
		{
			vx = LPosX > 0 ? 1 : -1;
			if (Ldistance > 6000) vy = LPosY > 0 ? 2 : -2;
			else if (Ldistance > 3000) vy = LPosY > 0 ? 1 : -1;
		}
		else
		{
			vx = 0;
			if (Ldistance > 6000) vy = LPosY > 0 ? 3 : -3;
			else if (Ldistance > 3000) vy = LPosY > 0 ? 2 : -2;
			else if (Ldistance > 100) vy = LPosY > 0 ? 1 : -1;
			else vy = 0;
		}
		ru = vx + vy; ld = vx + vy;
		lu = vy - vx; rd = vy - vx;
		//cout << "lu: " << lu << " ru: " << ru << " ld: " << ld << " rd: " << rd << endl;
	}
	else if (LeftTrigger > 5 && RightTrigger < 5)//240 120 5
	{
		if (LeftTrigger > 240) { lu = 3; ld = 3; ru = -3; rd = -3; rotation = 3; }
		else if (LeftTrigger > 120) { lu = 2; ld = 2; ru = -2; rd = -2;  rotation = 2; }
		else { lu = 1; ld = 1; ru = -1; rd = -1; rotation = 1; }
	}
	else if (LeftTrigger < 5 && RightTrigger > 5)
	{
		if (RightTrigger > 240) { lu = -3; ld = -3; ru = 3;  rd = 3; rotation = -3; }
		else if (RightTrigger > 120) { lu = -2; ld = -2; ru = 2; rd = 2; rotation = -2; }
		else { lu = -1; ld = -1; ru = 1; rd = 1; rotation = -1; }
	}
	if (lu > 0)
		motorframe1 += 32;
	if (rd > 0)
		motorframe2 += 4;
	if (ld > 0)
		motorframe1 += 4;
	if (ru > 0)
		motorframe2 += 32;
	motorframe1 += (abs(lu) << 3);
	motorframe1 += abs(ld);
	motorframe2 += (abs(ru) << 3);
	motorframe2 += abs(rd);
	sendchar(motorframe1);
	sendchar(motorframe2);
	cout << "  Vx:  " << vx << "  Vy:  " << vy << "  Ro:  " << rotation;
}
void set_steeringframe(const XINPUT_STATE& State, const int& PlayerID)
{
	short RPosX = State.Gamepad.sThumbRX / 400;//死区+-10   75 40 25
	short RPosY = State.Gamepad.sThumbRY / 400;//死区+-10   75 40 25
	int Rdistance = (RPosX * RPosX + RPosY * RPosY);//死区50   3000  6000
	//std::cout << "RPosY: " << RPosY << "  Rdistance: "<< Rdistance <<std::endl;
	int vx = 0, vy = 0;
	if (abs(RPosX) > 75)
	{
		vx = RPosX > 0 ? 3 : -3;
	}
	else if (abs(RPosX) > 40)
	{
		vx = RPosX > 0 ? 2 : -2;
	}
	else if (abs(RPosX) > 15)
	{
		vx = RPosX > 0 ? 1 : -1;
	}
	else
	{
		vx = 0;
	}
	if (abs(RPosY) > 75)
	{
		vy = RPosY > 0 ? 3 : -3;
	}
	else if (abs(RPosY) > 40)
	{
		vy = RPosY > 0 ? 2 : -2;
	}
	else if (abs(RPosY) > 15)
	{
		vy = RPosY > 0 ? 1 : -1;
	}
	else
	{
		vy = 0;
	}
	std::cout << "  Sx:  " << vx << "  Sy:  " << vy;
	if (vx != 0 || vy != 0)
	{
		int steeringframe = 0;
		if (vx > 0)  steeringframe += 32;
		if (vy > 0)  steeringframe += 4;
		steeringframe += (abs(vx) << 3);
		steeringframe += abs(vy);
		sendchar(steeringframe);
	}
}
