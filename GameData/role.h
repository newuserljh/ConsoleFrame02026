#pragma once
#include "base.h"
#include <vector>
#include "baseaddr.h"
/*
人物类
一般包含具体的人物属性等
*/
class role :
	public base
{
public:
	role();
	virtual ~role();
	virtual bool init();
	bool init_equip(DWORD p_ZB);
	bool init_promenade();
	bool role::initialize_properties(DWORD baseAddress);
	bool role::initialize_PROMENAD_properties(DWORD baseAddress);
	bool Get_Envionment(std::vector<DWORD>& pets, std::vector<DWORD>& npcs, std::vector<DWORD>& monsters, std::vector<DWORD>& players, DWORD g_range=15);
	bool Get_Ground(std::vector<DWORD>& vec, DWORD g_range = 15);
	ROLE_PROPERTY m_roleproperty;
	GOODS_PROPERTY m_euip[21];
private:



};

