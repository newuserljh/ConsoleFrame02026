//#include "stdafx.h"
#include "gamecall.h"




/*
函数功能:使用背包物品
参数一:物品在背包的位置(下标)
*/
bool gamecall::useGoods(int goodsIndex)
{
	try
	{
		_asm
		{
			pushad
			push 1
			push 0
			push 0
			push goodsIndex   //背包格子下标从0开始，快捷栏1-6为60-65  有VIP包裹 60-119 快捷栏120-125  
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_USEGOODS
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:对目标(指定坐标)使用技能
参数一:技能ID
参数二:坐标x
参数三:坐标y
参数四:目标ID (可以缺省为0)
*/
bool gamecall::useSkillTo(DWORD skillId, DWORD x, DWORD y, DWORD targetId)
{
	try
	{
		_asm
		{
			pushad
			push skillId
			push y
			push x
			push targetId
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_USESKILL
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


/*
函数功能:寻路到当前地图指定坐标4格以内
参数一:坐标x
参数二:坐标y
*/
bool gamecall::CurrentMapMove(DWORD x, DWORD y)
{
	try
	{
		_asm
		{
			pushad
			push 0
			push y
			push x
			mov ecx, dword ptr ds : [CALL_ECX_1]
			mov edx, CALL_CURRENT_MAPMOVE
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:走到指定坐标，每次走一格需要连续调用
参数一:坐标x
参数二:坐标y
参数三：走跑标记，1为走，2为跑
*/
bool gamecall::Run_or_Step_To(DWORD x, DWORD y, DWORD run_step_flag)
{
	DWORD call_addr;
	if (run_step_flag == 1)call_addr = CALL_STEP_TO_MAP;
	else if (run_step_flag == 2)call_addr = CALL_RUN_TO_MAP;
	else return false;
	try
	{
		_asm
		{
			pushad
			push y
			push x
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, call_addr
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:打开NPC
参数一:NPC的ID
*/
bool gamecall::OpendNPC(DWORD npcID)
{
	try
	{
		_asm
		{
			pushad
			push npcID
			mov edx, CALL_OPENNPC
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:选择对话
参数一:对话命令
*/
bool gamecall::ChooseCmd(const char* command)
{
	try
	{
		_asm
		{
			pushad
			push 0xffff
			push 0xffff
			push 0
			push 0
			push command
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, dword ptr ds : [CALL_EDX]
			push edx
			mov edi, CALL_CHOOSE_CMD
			call edi
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:拾取地面物品
参数一:坐标x
参数二:坐标y
*/
bool gamecall::pickupGoods(DWORD x, DWORD y)
{
	try
	{
		_asm
		{
			pushad
			push y
			push x
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_PICKUP_GOODS
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:登录游戏
参数一:用户名
参数二:密码
*/
bool gamecall::loginGame(const char* user, const char* pass)
{
	try
	{
		_asm
		{
			pushad
			push pass
			push user
			push ID_LONGIN
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_LOGIN
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


/*
函数功能:模拟按键 不支持回车键,不可用
参数一:按键码
*/
bool gamecall::presskey(int vkcode)
{
	int hcode = ::MapVirtualKey(vkcode, MAPVK_VK_TO_VSC);//取硬件扫描码
	try
	{
		_asm
		{
			pushad
			push hcode
			push vkcode
			mov ecx, dword ptr ds : [CALL_ECX_2]
			mov edx, CALL_PRESSKEY
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:根据进程pid获取窗口句柄
参数一:进程PID
*/
HWND gamecall::GetHwndByPid(DWORD dwProcessID)
{
	HWND h = GetTopWindow(0);
	HWND retHwnd = NULL;
	while (h)
	{
		DWORD pid = 0;
		DWORD dwTheardId = GetWindowThreadProcessId(h, &pid);
		if (dwTheardId != 0)
		{
			if (pid == dwProcessID && GetParent(h) == NULL && ::IsWindowVisible(h))
			{
				retHwnd = h;    //会有多个相等值
			}
		}
		h = GetNextWindow(h, GW_HWNDNEXT);
	}
	return retHwnd;
}

/*
函数功能:postMessage模拟按键 支持回车键 向HWND及其所有子控件发送指定按键消息
参数一:进程PID
参数一:按键码 缺省为回车
*/
bool gamecall::presskey(DWORD pid, int vkcode)
{
	int hcode = ::MapVirtualKey(vkcode, MAPVK_VK_TO_VSC);//取硬件扫描码
	DWORD dwProcId;
	::GetWindowThreadProcessId(GetHwndByPid(pid), &dwProcId);
	HWND hWnd = GetForegroundWindow();
	//char buf[1000];
	while (hWnd)
	{
		DWORD ProcId;
		GetWindowThreadProcessId(hWnd, &ProcId);
		if (ProcId == dwProcId)
		{
			::PostMessage(hWnd, WM_KEYDOWN, vkcode, hcode);
			Sleep(10);
			::PostMessage(hWnd, WM_KEYUP, vkcode, hcode);
		}
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return true;
}

/*
函数功能:计算坐标(x1,y1)与坐标(x2,y2)的距离
参数一:x1
参数一:y1
参数三：x2
参数四：y2
返回值：距离
*/
float gamecall::caclDistance(DWORD x1, DWORD y1, DWORD x2, DWORD y2)
{
	int x, y;
	x = x1 - x2;
	y = y1 - y2;
	return (float)sqrt(abs(x * x) + abs(y * y));
}

///*
//函数功能:找到要攻击的怪物
//参数一:角色结构体
//参数二:攻击怪物列表
//返回值：返回找到的怪物对象指针
//*/
//DWORD gamecall::Find_T_Monster(role& r, std:: vector<std::string>& vec)
//{
//	std::vector<DWORD>  near_Mon_7;
//	r.Get_Envionment(near_Mon_7, 7); /*找到9格已内的怪物*/
//	if (near_Mon_7.size())
//	{
//		for (unsigned i = 0; i < near_Mon_7.size(); i++)
//		{
//			for (unsigned j = 0; j < vec.size(); j++)
//			{
//				if (strcmp((CHAR*)(near_Mon_7[i] + 0x10), vec[j].c_str()) == 0) /*strcmp返回0代表相等*/
//					return near_Mon_7[i];
//			}
//		}
//	}
//	return 0;
//}


/*
函数功能:怪物距离sort排序函数
参数一:怪物对象
参数二:怪物对象
返回值：bool
*/
bool gamecall::comp(const MONSTER_PROPERTY& a, const MONSTER_PROPERTY& b)
{
	if (a.Distance < b.Distance) {
		return true;
	}
	else if (a.Distance == b.Distance && a.ID < b.ID) {
		return true;
	}
	else {
		return false;
	}
}
/*
函数功能:刷新周围怪物按距离排序
参数一:角色结构体
参数二:攻击怪物列表
参数三:遍历范围默认15
返回值：返回按距离排序的15格以内的攻击怪物怪物列表
*/
std::vector<MONSTER_PROPERTY> gamecall::sort_aroud_monster(role& r, std::vector<std::string>& vec, DWORD e_range)
{
	std::vector<MONSTER_PROPERTY> ret;
	std::vector<DWORD>  near_Mon,npcs,players,pets;
	r.Get_Envionment(pets,npcs,near_Mon,players,e_range); /*找到7格已内的怪物*/
	if (near_Mon.size())
	{
		for (unsigned i = 0; i < near_Mon.size(); i++)
		{
			for (unsigned j = 0; j < vec.size(); j++)
			{
				if (strcmp((CHAR*)(near_Mon[i] + 0x10), vec[j].c_str()) == 0) /*strcmp返回0代表相等*/
				{
					MONSTER_PROPERTY temp((DWORD*)near_Mon[i]);
					temp.Distance = caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y, *temp.X, *temp.Y);
					ret.push_back(temp);
				}
			}
		}
	}
	std::sort(ret.begin(), ret.end(), &comp);
	return ret;
}


/*
函数功能:地面物品距离sort排序函数
参数一:地面对象
参数二:地面对象
返回值：bool
*/
bool gamecall::comp_groud(const GROUND_GOODS& a, const GROUND_GOODS& b)
{
	if (a.Distance < b.Distance) {
		return true;
	}
	else {
		return false;
	}
}
/*
函数功能:刷新地面物品按距离排序
参数一:角色结构体
参数二:拾取物品列表
返回值：返回按距离排序的15格以内的需要拾取的地面物品列表
*/
std::vector<GROUND_GOODS> gamecall::sort_groud_goods(role& r, std::vector<std::string>& vec)
{
	std::vector<GROUND_GOODS> ret;
	std::vector<DWORD>  near_groud;
	r.Get_Ground(near_groud, 15);
	if (near_groud.size())
	{
		for (unsigned i = 0; i < near_groud.size(); i++)
		{
			for (unsigned j = 0; j < vec.size(); j++)
			{
				if (strcmp((char*)(near_groud[i] + 0x18), vec[j].c_str()) == 0) /*strcmp返回0代表相等*/
				{
					GROUND_GOODS temp((DWORD*)near_groud[i]);
					temp.Distance = caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y, *temp.X, *temp.Y);
					ret.push_back(temp);
					break;
				}
			}
		}
	}
	if (ret.size() > 1)
	{
		std::sort(ret.begin(), ret.end(), &comp_groud);
	}

	return ret;
}

/*
函数功能:分割坐标字符串
参数一:坐标字符串
参数一:返回的数组指针
返回值：坐标数组指针,如果x,y都为-1 则分割出错
*/
MapXY gamecall::splitXY(std::string str)
{
	MapXY XYtemp;
	std::regex delimiters("\\s+");
	std::vector<std::string> temp(std::sregex_token_iterator(str.begin(), str.end(), delimiters, -1), std::sregex_token_iterator());
	if (temp.size() != 2)
	{
		XYtemp.x = -1;
		XYtemp.y = -1;
	}
	else
	{
		XYtemp.x = stoi(temp[0]); //x
		XYtemp.y = stoi(temp[1]); //y
	}
	return XYtemp;
}
/*
函数功能:发起组队请求
参数一:开关标志 1开 0关
返回值：bool
*/
bool gamecall::team_open_close(DWORD tflag)
{
	try
	{
		_asm
		{
			pushad
			push tflag
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_TEAM_OPEN_CLOSE
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


/*
函数功能:发起组队请求
参数一:队员名字
返回值：bool
*/
bool gamecall::maketeam(std::string pName)
{
	const char* cname = pName.c_str();
	try
	{
		_asm
		{
			pushad
			push cname
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_MAKE_TEAM
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:同意组队请求
参数一:队长名字
返回值：bool
*/
bool gamecall::allowteam(std::string pName)
{
	const char* cname = pName.c_str();
	try
	{
		_asm
		{
			pushad
			push 1
			push cname
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_ALLOW_TEAM
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:释放元神
参数一:0
返回值：bool
*/
bool  gamecall::release_Promenade(void)
{
	try
	{
		_asm
		{
			pushad
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_RELEASE_PROMENADE
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}



/*
函数功能:开关自动挂机
参数一:挂机标志，1为开，0为关
返回值：bool
*/
bool  gamecall::start_end_AutoAttack(int turn_flag)
{
	try
	{
		_asm
		{
			pushad
			push turn_flag
			mov ecx, 0x3D6B4F0
			mov edx, CALL_START_END_ATTACK
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:打开自动回收对话框
参数一:VIP等级, vip等级>=1时，使用随身包裹回收，VIP等级为0时，使用NPC回收
返回值：bool
*/
bool  gamecall::OpenRecovry(unsigned vip_level)
{
	if (vip_level > 0)
	{
		try
		{
			_asm
			{
				pushad
				push 0xFFFF
				push 0xFFFF
				push 0
				push 0
				push 0xF14DE0
				push 0xFFFFFFEC
				mov ecx, dword ptr ds : [CALL_ECX]
				mov edx, CALL_OPEN_BAG_REC
				call edx
				popad
			}
		}
		catch (...)
		{
			return false;
		}
	}
	else
	{
		if (OpendNPC(NPC_ZJ_RCV))
		{
			if ((*(DWORD*)P_CURRENT_NPC_DLG == NPC_ZJ_RCV))return true;
		}
		return false;
	}
	return true;
}

/*
函数功能:物品回收，根据对话框选择的回收经验或者元宝金币
参数一:物品ID
返回值：bool
*/
bool  gamecall::RecovryGoods(DWORD goodsId)
{
	try
	{
		_asm
		{
			pushad
			push goodsId
			mov ecx, P_CURRENT_NPC_DLG
			mov ecx, dword ptr ds : [ecx]
			push ecx
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_RCV_GOODS
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


/*
函数功能:物品回收经验
参数一:物品ID
参数二:VIP等级
返回值：bool
*/
bool  gamecall::RecovryGoods_To_Exp(DWORD goodsId, unsigned vip_level)
{
	OpenRecovry(vip_level);
	ChooseCmd("@Recovery_Exp_KaiQu");
	bool rtn = RecovryGoods(goodsId);
	ChooseCmd("@Recovery_Exp_KaiQu");
	return rtn;
}

/*
函数功能:物品回收元宝或金币
参数一:物品ID
参数二:VIP等级
返回值：bool
*/
bool  gamecall::RecovryGoods_To_Gold(DWORD goodsId, unsigned vip_level)
{
	OpenRecovry(vip_level);
	ChooseCmd("@Recovery_Gold_KaiQu");
	bool rtn = RecovryGoods(goodsId);
	ChooseCmd("@Recovery_Gold_KaiQu");
	return	rtn;
}

/*
函数功能:提交输入框内容
参数一:提交的内容
返回值：bool
*/
bool gamecall::SubmitInputbox(const char* content)
{
	try
	{
		_asm
		{
			pushad
			push 1
			push content
			mov ecx, dword ptr ds : [CALL_EDX]
			push ecx
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_SUBMIT_INPUTBOX
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:获取角色 的目标的对象指针
参数一:role对象
返回值：对象指针
*/
DWORD* gamecall::getTargetP(role &r)
{
	if (*r.m_roleproperty.p_Target_ID)
	{
		DWORD id = *r.m_roleproperty.p_Target_ID;
		std::vector<DWORD> vec,npcs,pets,players;
		r.Get_Envionment(pets,npcs,vec,players);	

		for (auto it:vec)
		{
			if (*(DWORD*)(it + 0x8) == id) return (DWORD*)it;
		}
	}
	return nullptr;
}

/*
函数功能:角色死亡后立即复活
参数一:0
返回值：bool
*/
bool  gamecall::immdia_rebirth(void)
{
	try
	{
		_asm
		{
			pushad
			push 0x2
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_IMMDIA_REBIRTH
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:小退游戏到角色选择界面
参数一:0
返回值：bool
*/
bool  gamecall::small_exit(void)
{
	try
	{
		_asm
		{
			pushad
			push 0x1
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_SMALL_EXIT
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

/*
函数功能:获取购买物品封包的物品ID，需要先选择物品之后才可以调用
参数一:索引从0开始，默认0（第一个）
返回值：物品ID
*/
DWORD  gamecall::get_buy_goods_ID(DWORD goods_index)
{
	DWORD goodsid = 0;
	try
	{
		_asm
		{
			pushad
			push goods_index
			mov ecx, 0x110D9E8
			mov edx, CALL_GET_BUY_GOODS_ID
			call edx
			mov eax, dword ptr ds : [eax + 0x20]
			mov goodsid,eax
			popad
		}
	}
	catch (...)
	{
		return false;
	}
	return goodsid;
}


/*
函数功能:买物品
参数一:物品名字
参数二:NPC——ID
参数三:是否绑定，默认不绑定，选择绑定金币购买的话，设置为1
参数四:买入物品数量,为空的话买1个
返回值：bool
*/
bool gamecall::buyGoods(const std::string& goodsName,DWORD npcId,DWORD isBind,unsigned number)
{
	/*  std::vector<std::string> drug = {"强效太阳神水", "太阳神水", "金创药(小量)", "金创药(中量)","金创药(大量)", "魔法药(小量)", "魔法药(中量)", "魔法药(大量)",
		"金创药（小）包", "金创药（中）包", "魔法药（小）包", "魔法药（中）包", "超级金创药", "超级魔法药", "特级金创药", "特级魔法药", "特级金创药包", "特级魔法药包" ,
		"万年雪霜","鹿茸","治疗药水" }; //0x64
	std::vector<std::string> books = { "初级剑法", "血影刀法", "残影刀法", "金创药(中量)","攻杀剑法", "护身真气", "刺杀剑术", "抱月刀",
	"半月弯刀", "野蛮冲撞", "战魂真悟", "雷霆剑", "烈火剑法", "破击剑法", "破盾斩", "突斩", "金刚护体", "擒龙手" ,"移形换影" ,"小火球" ,"抗拒火环" ,"诱惑之光" ,"地狱火焰" ,"雷电术" ,
	"冰箭术","瞬间移动","火炎刀" ,"风火轮" ,"爆裂火焰" ,"火墙" ,"疾光电影" ,"冰龙破" ,"玄冰刃" ,"地狱雷光" ,"法之魄" ,"魔法盾" ,"圣言术" ,"五雷轰" ,"冰咆哮" ,"风影盾" ,"兽灵术" ,"冰旋风" ,
	"狂龙紫电","化身蝙蝠" ,"魔魂术","流星火雨" ,"治疗术" ,"精神战法" ,"诅咒术" ,"赶尸" ,"隐身术" ,"替身法符" ,"集体隐身术" ,"幽灵盾" ,"神圣战甲术" ,"狮子吼" ,"困魔咒" ,"灵魂墙" ,"道心清明" ,
		"群体治愈术" ,"召唤神兽" ,"神光术" ,"解毒术" ,"幽冥火咒" ,"强化骷髅术" ,"心灵召唤" ,"遁地" };//0x32  */


		//先处理物品名字和npc，确定call的第四个参数   0x64（药）  0x32（书，地牢，随机）  0x1E(沙回，地牢包) 1（回城） 0xA（随机包，修复油）
	DWORD param_4 = 0;
	if (npcId==NPC_ZJ_YDZG) //中州药店
	{
		if ((goodsName.find("毒药") != std::string::npos) || (goodsName.find("稻草人") != std::string::npos))
		{
			param_4 = get_buy_goods_ID(); //买毒药
		}
		else
		{
			param_4 = 0x64;//买药
		}
	}
	if (npcId==NPC_ZJ_ZHP)//杂货
	{
		if (goodsName == "沙城回城卷" || goodsName == "地牢逃脱卷包")
		{
			param_4 = 0x32;
		}
		if (goodsName=="地牢卷轴"|| goodsName == "随机卷轴")
		{
			param_4 = 0x32;
		}
		if (goodsName == "回城卷轴" )
		{
			param_4 = 1;
		}
		if (goodsName == "随机传送卷包" || goodsName == "修复油")
		{
			param_4 = 0xA;
		}
		if (goodsName == "道符" || goodsName == "道符（大）")
		{
			param_4 = get_buy_goods_ID();
		}
	}
	//if (npcId == NPC_ZJ_SDZG)param_4 = 0x32; //买书，书店老板在屋里 暂时未实现
	else
	{
		param_4 = get_buy_goods_ID();
	}
	if (param_4 == 0) return false;

	const char* gname = goodsName.c_str();

	if ((param_4< 0x65) && (param_4>0)) //购买要 卷轴等 不需要物品ID
	{
		for (unsigned i = 0; i < number; i++)
		{
			try
			{
				_asm
				{
					pushad
					push 0
					push isBind
					push gname
					push param_4
					push npcId
					mov ecx, dword ptr ds : [CALL_ECX]
					mov edx, CALL_BUY_GOODS
					call edx
					popad
				}
			}
			catch (...)
			{
				return false;
			}
		}
	}
	else //需要ID
	{
		for (unsigned i = 0; i < number; i++)
		{
			param_4 = get_buy_goods_ID(i);
			try
			{
				_asm
				{
					pushad
					push 0
					push isBind
					push gname
					push param_4
					push npcId
					mov ecx, dword ptr ds : [CALL_ECX]
					mov edx, CALL_BUY_GOODS
					call edx
					popad
				}
			}
			catch (...)
			{
				return false;
			}
		}
	}
	return true;
}


/*
函数功能:卖物品
参数一:物品名字
参数二:物品ID
参数三:NPC——ID
返回值：bool
*/
bool gamecall::sellGoods(const std::string& goodsName, DWORD goodsId, DWORD npcId)
{
	const char* name = goodsName.c_str();

			try
			{
				_asm
				{
					pushad
					push name
					push goodsId
					push npcId
					mov ecx, dword ptr ds : [CALL_ECX]
					mov edx, CALL_SELL_GOODS
					call edx
					popad
				}
			}
			catch (...)
			{
				return false;
			}
}


/*
函数功能:存物品
参数一:物品名字
参数二:物品ID
参数三:NPC——ID
返回值：bool
*/
bool gamecall::storeGoods(const std::string& goodsName, DWORD goodsId, DWORD npcId)
{
	const char* name = goodsName.c_str();

	try
	{
		_asm
		{
			pushad
			push 0
			push name
			push goodsId
			push npcId
			mov ecx, dword ptr ds : [CALL_ECX]
			mov edx, CALL_STORE_GOODS
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
}

/*
函数功能:用鼠标拿起或者放下背包物品
参数一:背包索引下标从0开始
参数二:背包类型默认0：0（主体背包）,1(元神背包)，2(灵兽背包)
返回值：bool
*/
bool gamecall::takeOrPutBagGoods(DWORD bag_type,DWORD bag_index)
{
	auto call_addr = -1;
	if (bag_type == 0)call_addr = CALL_TAKE_PUT_ROLE_BAG_GOODS;
	else if (bag_type == 1)call_addr = CALL_TAKE_PUT_YS_BAG_GOODS;
	else if (bag_type == 2)call_addr = CALL_TAKE_PUT_PAT_BAG_GOODS;
	else return false;
	try
	{
		_asm
		{
			pushad
			push bag_index
			mov ecx, dword ptr ds : [CALL_ECX_2]
			add ecx,0x158
			mov edx, call_addr
			call edx
			popad
		}
	}
	catch (...)
	{
		return false;
	}
}

/*
函数功能:移动物品（从当前背包到目的背包）
参数一:当前背包代码（0（主体背包）,1(元神背包)，2(灵兽背包)）
参数二:当前物品所在背包索引下标
参数三:目的背包代码（0（主体背包）,1(元神背包)，2(灵兽背包)）
参数四:目的物品所在背包索引下标
返回值：bool
*/
bool gamecall::MoveGoods_FromBag_ToOtherBag( DWORD curruent_bag, DWORD current_index, DWORD dest_bag, DWORD dest_index)
{

	bool rtn = takeOrPutBagGoods(curruent_bag, current_index);
	if (!rtn)  return false;
	Sleep(100);
	rtn = takeOrPutBagGoods(dest_bag, dest_index);
	if (!rtn)  return false;
	return true;
}

/*
函数功能:叠加背包物品
参数一:物品名字
参数二:背包对象
参数三:当前背包代码（0（主体背包）,1(元神背包)，2(灵兽背包)）
返回值：bool
*/
bool gamecall::AddGoodsInBag(const std::string& goodsname, bag& mbag, DWORD current_bag)
{
	int firstId = mbag.getGoodsFirstIndex(goodsname);
	if (firstId == -1) return false;
	int nextId = mbag.getGoodsNextIndex(goodsname,firstId);
	if (nextId == -1)return false;
	return MoveGoods_FromBag_ToOtherBag(current_bag, nextId, current_bag, firstId);
}

/*
函数功能:在背包之间移动物品
参数一:物品名字
参数二:当前背包对象
参数三:当前背包代码（0（主体背包）,1(元神背包)，2(灵兽背包)）
参数四:目的背包对象
参数五:目的背包代码（0（主体背包）,1(元神背包)，2(灵兽背包)）
返回值：bool
*/
bool gamecall::MoveGoodsInBags(const std::string& goodsname, bag& mbag_current, DWORD current_bag, bag& mbag_dest,DWORD dest_bag)
{
	int currentId = mbag_current.getGoodsFirstIndex(goodsname);
	if (currentId == -1) return false;
	int destId = mbag_dest.getFirstSpaceIndex();
	if (destId == -1)return false;
	return MoveGoods_FromBag_ToOtherBag(current_bag, currentId, dest_bag, destId);
}