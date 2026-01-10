#include "role.h"
#include "baseaddr.h"
#include "../Common/utils.h"

// 宏定义，用于简化属性初始化
#define INIT_PROPERTY(base, offset, type, member) \
    m_roleproperty.member = reinterpret_cast<type*>((*(DWORD*)base) + offset)

role::role()
{
}

role::~role()
{
}

// 初始化角色属性
bool role::init()
{
	// 初始化角色基本属性
	if (!initialize_properties(RoleBase)) return false;
	// 初始化角色装备
	if (!init_equip((DWORD)m_roleproperty.p_ZB)) return false;
	return true;
}

// 初始化元神属性
bool role::init_promenade()
{
	// 初始化元神基本属性
	if (!initialize_PROMENAD_properties(PROMENADE_Base)) return false;
	// 初始化元神装备
	if (!init_equip((DWORD)m_roleproperty.p_ZB))return false;
	return true;
}

// 初始化属性的通用函数
bool role::initialize_properties(DWORD baseAddress)
{
	try
	{
		// 使用宏定义初始化各个属性
		INIT_PROPERTY(baseAddress, 8, DWORD, Object.ID);
		INIT_PROPERTY(baseAddress, 0x10, char, Object.pName);
		INIT_PROPERTY(baseAddress, 0x80, DWORD, Object.HP);
		INIT_PROPERTY(baseAddress, 0x84, DWORD, Object.HP_MAX);
		INIT_PROPERTY(baseAddress, 0x88, DWORD, Object.MP);
		INIT_PROPERTY(baseAddress, 0x8c, DWORD, Object.MP_MAX);
		INIT_PROPERTY(baseAddress, 0xac, DWORD, Object.X);
		INIT_PROPERTY(baseAddress, 0xb0, DWORD, Object.Y);
		INIT_PROPERTY(baseAddress, 0x34b, BYTE, Object.IsPosion);
		INIT_PROPERTY(baseAddress, 0x5cc, DWORD, VIP_Level);
		INIT_PROPERTY(baseAddress, 0xe4, DWORD, Job);
		INIT_PROPERTY(baseAddress, 0xec, DWORD, Level);
		INIT_PROPERTY(baseAddress, 0xdc, DWORD, Sex);
		INIT_PROPERTY(baseAddress, 0xA5C, DWORD, GJL_L);
		INIT_PROPERTY(baseAddress, 0xA60, DWORD, GJL_H);
		INIT_PROPERTY(baseAddress, 0xA64, DWORD, MFL_L);
		INIT_PROPERTY(baseAddress, 0xA68, DWORD, MFL_H);
		INIT_PROPERTY(baseAddress, 0xA6c, DWORD, DSL_L);
		INIT_PROPERTY(baseAddress, 0xA70, DWORD, DSL_H);
		INIT_PROPERTY(baseAddress, 0xA7C, DWORD, FY_L);
		INIT_PROPERTY(baseAddress, 0xA80, DWORD, FY_H);
		INIT_PROPERTY(baseAddress, 0xA84, DWORD, MF_L);
		INIT_PROPERTY(baseAddress, 0xA88, DWORD, MF_H);
		INIT_PROPERTY(baseAddress, 0xAA8, WORD, BAG_W);
		INIT_PROPERTY(baseAddress, 0xAAA, WORD, BAG_W_MAX);
		INIT_PROPERTY(baseAddress, 0xAD0, DWORD, SW);
		INIT_PROPERTY(baseAddress, 0xC54, DWORD, p_Bag_Base);
		INIT_PROPERTY(baseAddress, 0xc58, DWORD, Bag_Size);
		INIT_PROPERTY(baseAddress, 0xc60, DWORD, p_LR_Bag_Base);
		INIT_PROPERTY(baseAddress, 0xc64, DWORD, LR_Bag_Size);
		INIT_PROPERTY(baseAddress, 0x458, DWORD, p_Target_ID);
		INIT_PROPERTY(baseAddress, 0x1358, DWORD, p_Skill_Base);
		INIT_PROPERTY(baseAddress, 0xc48, DWORD, p_ZB);
		INIT_PROPERTY(baseAddress, 0x139C, DWORD, LL);
		INIT_PROPERTY(baseAddress, 0x8299C8, char, p_Current_Map);
		INIT_PROPERTY(baseAddress, 0xA24, DWORD, PatCount);
		INIT_PROPERTY(baseAddress, 0x1054550, DWORD, Team_is_allow);
		INIT_PROPERTY(baseAddress, 0x1054550 + 8, DWORD, Team_pointer);
		INIT_PROPERTY(baseAddress, 0x13A0, DWORD, Is_has_Promenade);
		INIT_PROPERTY(baseAddress, 0x3D8, DWORD, Is_Promenade_Release);
		INIT_PROPERTY(baseAddress, 0xA08, DWORD, Promenade_Mode);

		INIT_PROPERTY(baseAddress, 0xA84, DWORD, p_Role_EXP);
		INIT_PROPERTY(baseAddress, 0xAA0, DWORD, p_Role_EXP_MAX);
		INIT_PROPERTY(baseAddress, 0xC08, double, p_Role_YB_Bind);
		INIT_PROPERTY(baseAddress, 0xC10, DWORD, p_Role_Gold);
		INIT_PROPERTY(baseAddress, 0xC14, DWORD, p_Role_GoldBind);
		INIT_PROPERTY(baseAddress, 0xBEC, DWORD, p_Role_YB);

		// 检查 HP_MAX 是否为 0
		if (!*m_roleproperty.Object.HP_MAX)
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

// 初始化元神属性的通用函数
bool role::initialize_PROMENAD_properties(DWORD baseAddress)
{
	try
	{
		// 使用宏定义初始化各个属性
		INIT_PROPERTY(baseAddress, 8, DWORD, Object.ID);
		INIT_PROPERTY(baseAddress, 0x10, char, Object.pName);
		INIT_PROPERTY(baseAddress, 0x80, DWORD, Object.HP);
		INIT_PROPERTY(baseAddress, 0x84, DWORD, Object.HP_MAX);
		INIT_PROPERTY(baseAddress, 0x88, DWORD, Object.MP);
		INIT_PROPERTY(baseAddress, 0x8c, DWORD, Object.MP_MAX);
		INIT_PROPERTY(baseAddress, 0xac, DWORD, Object.X);
		INIT_PROPERTY(baseAddress, 0xb0, DWORD, Object.Y);
		INIT_PROPERTY(baseAddress, 0x34b, BYTE, Object.IsPosion);
		//INIT_PROPERTY(baseAddress, 0x5cc, DWORD, VIP_Level);
		INIT_PROPERTY(baseAddress, 0xe4, DWORD, Job);
		INIT_PROPERTY(baseAddress, 0xec, DWORD, Level);
		INIT_PROPERTY(baseAddress, 0xdc, DWORD, Sex);
		INIT_PROPERTY(baseAddress, 0xA5C, DWORD, GJL_L);
		INIT_PROPERTY(baseAddress, 0xA60, DWORD, GJL_H);
		INIT_PROPERTY(baseAddress, 0xA64, DWORD, MFL_L);
		INIT_PROPERTY(baseAddress, 0xA68, DWORD, MFL_H);
		INIT_PROPERTY(baseAddress, 0xA6c, DWORD, DSL_L);
		INIT_PROPERTY(baseAddress, 0xA70, DWORD, DSL_H);
		INIT_PROPERTY(baseAddress, 0xA7C, DWORD, FY_L);
		INIT_PROPERTY(baseAddress, 0xA80, DWORD, FY_H);
		INIT_PROPERTY(baseAddress, 0xA84, DWORD, MF_L);
		INIT_PROPERTY(baseAddress, 0xA88, DWORD, MF_H);
		INIT_PROPERTY(baseAddress, 0xAA8, WORD, BAG_W);
		INIT_PROPERTY(baseAddress, 0xAAA, WORD, BAG_W_MAX);
		INIT_PROPERTY(baseAddress, 0xAD0, DWORD, SW);
		INIT_PROPERTY(baseAddress, 0xC54, DWORD, p_Bag_Base);
		INIT_PROPERTY(baseAddress, 0xc58, DWORD, Bag_Size);
		//INIT_PROPERTY(baseAddress, 0xc60, DWORD, p_LR_Bag_Base);
		//INIT_PROPERTY(baseAddress, 0xc64, DWORD, LR_Bag_Size);
		INIT_PROPERTY(baseAddress, 0x458, DWORD, p_Target_ID);
		INIT_PROPERTY(baseAddress, 0x1358, DWORD, p_Skill_Base);
		INIT_PROPERTY(baseAddress, 0xc48, DWORD, p_ZB);
		//INIT_PROPERTY(baseAddress, 0x139C, DWORD, LL);
		INIT_PROPERTY(baseAddress, 0x8299C8, char, p_Current_Map);
		INIT_PROPERTY(baseAddress, 0xA24, DWORD, PatCount);
		//INIT_PROPERTY(baseAddress, 0x1054550, DWORD, Team_is_allow);
		//INIT_PROPERTY(baseAddress, 0x1054550 + 8, DWORD, Team_pointer);
		//INIT_PROPERTY(baseAddress, 0x13A0, DWORD, Is_has_Promenade);
		//INIT_PROPERTY(baseAddress, 0x3D8, DWORD, Is_Promenade_Release);
		//INIT_PROPERTY(baseAddress, 0xA08, DWORD, Promenade_Mode);

		INIT_PROPERTY(baseAddress, 0xA84, DWORD, p_Role_EXP);
		INIT_PROPERTY(baseAddress, 0xAA0, DWORD, p_Role_EXP_MAX);
		INIT_PROPERTY(baseAddress, 0xC08, double, p_Role_YB_Bind);
		INIT_PROPERTY(baseAddress, 0xC10, DWORD, p_Role_Gold);
		INIT_PROPERTY(baseAddress, 0xC14, DWORD, p_Role_GoldBind);
		INIT_PROPERTY(baseAddress, 0xBEC, DWORD, p_Role_YB);

		// 检查 HP_MAX 是否为 0
		if (!*m_roleproperty.Object.HP_MAX)
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


// 初始化装备
bool role::init_equip(DWORD p_ZB)
{
	if (nullptr== (DWORD*)p_ZB) return false;
	DWORD zbBase = *(DWORD*)p_ZB;
	try
	{
		for (auto i = 0; i < 21; i++)
		{
			m_euip[i].pName = reinterpret_cast<char*>(zbBase + i * 0x688 + 1);
			m_euip[i].WD_low = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1a);
			m_euip[i].WD_high = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1b);
			m_euip[i].MD_low = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1c);
			m_euip[i].WD_high = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1d);
			m_euip[i].PA_low = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1e);
			m_euip[i].PA_high = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x1f);
			m_euip[i].Magic_low = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x20);
			m_euip[i].Magic_high = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x21);
			m_euip[i].Tao_low = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x22);
			m_euip[i].Tao_high = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x23);
			m_euip[i].Need_what = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x24);
			m_euip[i].Need_Num = reinterpret_cast<BYTE*>(zbBase + i * 0x688 + 0x25);
			m_euip[i].ID = reinterpret_cast<DWORD*>(zbBase + i * 0x688 + 0x2c);
			m_euip[i].Use_Num = reinterpret_cast<WORD*>(zbBase + i * 0x688 + 0x30);
			m_euip[i].Use_Num_Max = reinterpret_cast<WORD*>(zbBase + i * 0x688 + 0x32);
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

// 遍历周围对象
bool role::Get_Envionment(std::vector<DWORD>& pets, std::vector<DWORD>& npcs, std::vector<DWORD>& monsters, std::vector<DWORD>& players, DWORD g_range)
{
	pets.clear();
	npcs.clear();
	monsters.clear();
	players.clear();
	DWORD p_temp = 0;

	// 宝宝名字的子字符串集合
	std::vector<std::string> petNames = { "级踏云豹", "级震天狮", "级傲天凤", "级麒麟","级小麒麟", "级强化骷髅", "级变异骷髅", "级尸灵",
		"级青铜兽", "级滚刀手", "级石人", "级铜锤手", "级火烈鸟", "级兽骑兵", "级兽骑统领", "级招魂使", "级烈焰使", "级猛魔统领" };

	for (int i = (int)*m_roleproperty.Object.X - (int)g_range; i < (int)*m_roleproperty.Object.X + (int)g_range; i++)
	{
		if (i <= 0) i = 1;
		for (int j = (int)*m_roleproperty.Object.Y - (int)g_range; j < (int)*m_roleproperty.Object.Y + (int)g_range; j++)
		{
			if (j <= 0) j = 1;
			try
			{
				_asm
				{
					pushad
					mov ecx, RoleBase
					mov ecx, [ecx]
					add ecx, 0x1D38
					mov edi, i
					mov esi, j
					mov eax, 0x66666667
					imul edi
					sar edx, 0x4
					mov eax, edx
					shr eax, 0x1F
					add eax, edx
					lea eax, dword ptr ds : [eax + eax * 4]
					add eax, eax
					add eax, eax
					add eax, eax
					mov ebx, eax
					mov eax, 0x99999999
					imul esi
					sar edx, 0x4
					mov eax, edx
					shr eax, 0x1F
					add eax, edx
					sub eax, ebx
					add eax, edi
					lea edx, dword ptr ds : [eax + eax * 4]
					lea eax, dword ptr ds : [esi + edx * 8]
					imul eax, eax, 0xA70
					add ecx, Envi_Offset
					mov eax, dword ptr ds : [eax + ecx]
					mov p_temp, eax
					popad
				}
				if (p_temp != 0)
				{
					if (*(DWORD*)(p_temp + 0x80) == 0) continue; //过滤死亡对象

					WORD t = *(WORD*)(p_temp + 0x68);
					if (t == 0) //玩家
					{
						players.push_back(p_temp);
					}
					else if (t == 1) //NPC
					{
						npcs.push_back(p_temp);
					}
					else if (t == 0x400) //怪物 0x400为心魔类怪物，个地图守卫
					{
						monsters.push_back(p_temp);
					}
					else if (t == 2)
					{
						std::string name = (char*)((DWORD)p_temp + 0x10);
						bool isPet = false;
						for (const auto& petName : petNames)
						{
							if (name.find(petName) != std::string::npos)
							{
								isPet = true;
								break;
							}
						}
						if (isPet)
						{
							pets.push_back(p_temp);
						}
						else
						{
							monsters.push_back(p_temp);
						}
					}
					else
					{
						//其他类型 后续输出到文件 debug
					}
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

// 获取地面物品
bool role::Get_Ground(std::vector<DWORD>& vec, DWORD g_range)
{
	vec.clear();
	DWORD p_temp = 0;
	for (int i = (int)*m_roleproperty.Object.X - (int)g_range; i < (int)*m_roleproperty.Object.X + (int)g_range; i++)
	{
		if (i <= 0) i = 1;
		for (int j = (int)*m_roleproperty.Object.Y - (int)g_range; j < (int)*m_roleproperty.Object.Y + (int)g_range; j++)
		{
			if (j <= 0) j = 1;
			try
			{
				_asm
				{
					pushad
					mov ecx, RoleBase
					mov ecx, [ecx]
					add ecx, 0x1D38
					mov edi, i
					mov esi, j
					mov eax, 0x66666667
					imul edi
					sar edx, 0x4
					mov eax, edx
					shr eax, 0x1F
					add eax, edx
					lea eax, dword ptr ds : [eax + eax * 4]
					add eax, eax
					add eax, eax
					add eax, eax
					mov ebx, eax
					mov eax, 0x99999999
					imul esi
					sar edx, 0x4
					mov eax, edx
					shr eax, 0x1F
					add eax, edx
					sub eax, ebx
					add eax, edi
					lea edx, dword ptr ds : [eax + eax * 4]
					lea eax, dword ptr ds : [esi + edx * 8]
					imul eax, eax, 0xA70
					add ecx, Ground_Offset
					mov eax, dword ptr ds : [eax + ecx]
					mov p_temp, eax
					popad
				}
				if (p_temp != 0)
				{
					vec.push_back(p_temp);/*地面*/
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

