#include "skill.h"


skill::skill()
{
	skillBase = 0;
}


skill::~skill()
{
}

bool skill::init()
{
	skillBase = *(DWORD*)skillBase;
	if (!skillBase)return false;
	m_skillList.clear();
	try
	{
		int i = 0;
		SKILL_PROPERTY temp;
		do 
		{
			temp.ID = (DWORD*)(skillBase + i * 0x88 + 0xe);
			temp.level = (DWORD*)(skillBase + i * 0x88 + 8);
			temp.pName = (char*)(skillBase + i * 0x88 + 0x14);
			m_skillList.push_back(temp);
			i++;
		} while (*(DWORD*)(skillBase + i * 0x88 + 0x14));
	}
	catch (...)
	{
		return false;
	}
	return true;
}


/*
函数功能:通过名字获取技能ID
参数一:技能名字
返回值：技能ID，如果为-1表示没有此技能
*/
DWORD skill::getSkillId(const char* sName)
{
	DWORD ret = -1;
	for (size_t i=0;i<m_skillList.size();i++)
	{
		if (strcmp(sName,m_skillList[i].pName)==0)
		{
			return *m_skillList[i].ID;
		}
	}
	return ret;
}
