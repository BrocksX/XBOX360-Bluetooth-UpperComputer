# 微软XBOX360手柄控制的蓝牙上位机开源

###  **介绍**
由于智能车的操控手有切换观察视角的要求，考虑到电脑太笨重，所以放弃传统的键盘/键鼠操控模式，采用微软的XBOX360手柄作为信号输入源

### 说明
本程序需要用到xinput.h头文件和xinput.lib，请确保电脑上有相关环境。微软官方介绍文档 https://docs.microsoft.com/zh-cn/windows/win32/api/xinput/nf-xinput-xinputgetstate?redirectedfrom=MSDN


首先建立蓝牙串口链接，设定好通信参数

```
HANDLE hCom = CreateFile("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); // 改COM口
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
```

确保手柄正常连接和工作

```
XINPUT_STATE State;
	int PlayerID = 0;	//0 - first user
	if (XInputGetState(PlayerID, &State) != ERROR_SUCCESS) {	//device not found
		std::cout << "Error: a device is not found for this user: " << PlayerID << " !\n";
	}
	else {	//device is found
		while (1) {
			if (XInputGetState(PlayerID, &State) == ERROR_SUCCESS) {	//make sure device is connected
			
			////////
			
			}
```

接下来就是检查各个按键和遥感的输入状态，发送相应的数据帧

```
				ProcessFaceButtons(State);
				ProcessPadButtons(State);
				ProcessShoulderButtons(State);
				ProcessThumbButtons(State);
				ProcessTriggers(State, PlayerID);
				ProcessThumbs(State, PlayerID);
				ProcessBackAndStartButtons(State);
				printstate();
```

手柄摇杆的处理
不同的手柄会存在不同的漂移，所以需要根据手柄的时间参数来规划好死区。

```
short LthumbPosX = State.Gamepad.sThumbLX / 10; 
	short LthumbPosY = State.Gamepad.sThumbLY / 10;
	short RthumbPosX = State.Gamepad.sThumbRX / 10;
	short RthumbPosY = State.Gamepad.sThumbRY / 10;
	int Ldistance = LthumbPosX * LthumbPosX + LthumbPosY * LthumbPosY;
	int Rdistance = RthumbPosX * RthumbPosX + RthumbPosY * RthumbPosY;
	//std::cout << "Ldistance: " << Ldistance << std::endl;
	if (Ldistance < 100000){}//死区
	else if (Ldistance < 6000000 && Ldistance > 100000)
	{
		///
	}
	else if (Ldistance > 6000000)
	{
	    ///
	}
```
