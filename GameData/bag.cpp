#include "bag.h"
#include <iostream>
bag::bag()
{
	bSpace = 0;
	maxSize = 0;
	bagBase = 0;
}

bag::~bag()
{

}

bool bag::init()
{
	bSpace = 0;
	bagBase = *(DWORD*)bagBase;
	if (!bagBase) return false;
	GOODS_PROPERTY temp;
	try
	{
		for (size_t i = 0; i < maxSize; i++)
		{
			m_bag[i].Name_Length = (BYTE*)(bagBase + i * 0x688);
			m_bag[i].pName = (char*)(bagBase + i * 0x688 + 1);
			m_bag[i].WD_low = (BYTE*)(bagBase + i * 0x688 + 0x1a);
			m_bag[i].WD_high = (BYTE*)(bagBase + i * 0x688 + 0x1b);
			m_bag[i].MD_low = (BYTE*)(bagBase + i * 0x688 + 0x1c);
			m_bag[i].MD_high = (BYTE*)(bagBase + i * 0x688 + 0x1d);
			m_bag[i].PA_low = (BYTE*)(bagBase + i * 0x688 + 0x1e);
			m_bag[i].PA_high = (BYTE*)(bagBase + i * 0x688 + 0x1f);
			m_bag[i].Magic_low = (BYTE*)(bagBase + i * 0x688 + 0x20);
			m_bag[i].Magic_high = (BYTE*)(bagBase + i * 0x688 + 0x21);
			m_bag[i].Tao_low = (BYTE*)(bagBase + i * 0x688 + 0x22);
			m_bag[i].Tao_high = (BYTE*)(bagBase + i * 0x688 + 0x23);
			m_bag[i].Need_what = (BYTE*)(bagBase + i * 0x688 + 0x24);
			m_bag[i].Need_Num = (BYTE*)(bagBase + i * 0x688 + 0x25);
			m_bag[i].ID = (DWORD*)(bagBase + i * 0x688 + 0x2c);
			m_bag[i].Use_Num = (WORD*)(bagBase + i * 0x688 + 0x30);
			m_bag[i].Use_Num_Max = (WORD*)(bagBase + i * 0x688 + 0x32);
		}
	}
	catch (...)
	{
		return false;
	}
    classifyBagItems();
	return true;
}

//分类背包物品
void bag::classifyBagItems()
{
	for (size_t i = 0; i < maxSize; i++)
	{

		if (*m_bag[i].ID != 0)
		{
			if (std::find(StoreVec.begin(), StoreVec.end(), std::string(m_bag[i].pName)) != StoreVec.end())m_bag[i].howProcess = 1;
			else if (std::find(SellWeaponVec.begin(), SellWeaponVec.end(), std::string(m_bag[i].pName)) != SellWeaponVec.end())
			{
				m_bag[i].howProcess = 2;
				m_bag[i].goods_type = 1;
			}
			else if (std::find(SellClothesVec.begin(), SellClothesVec.end(), std::string(m_bag[i].pName)) != SellClothesVec.end())
			{
				m_bag[i].howProcess = 2;
				m_bag[i].goods_type = 3;
			}
			else if (std::find(SellJewelryVec.begin(), SellJewelryVec.end(), std::string(m_bag[i].pName)) != SellJewelryVec.end())
			{
				m_bag[i].howProcess = 2;
				m_bag[i].goods_type = 2;
			}
			else if (SellMedicineVec.count(std::string(m_bag[i].pName)) > 0)
			{
				auto it = SellMedicineVec.find(std::string(m_bag[i].pName));
				m_bag[i].howProcess = 4;
				m_bag[i].goods_type = 4;
				m_bag[i].remainNumbers = it->second;
			}
			else
			{
				m_bag[i].howProcess = 5;
			}
		}
	}
}

/*
函数功能:获取物品的数量
参数一:物品名字
返回值:该物品在背包内的总数量
*/
int bag::caclGoodsNumber(std::string pName)
{
	int goodsnum = 0;
	for (size_t i = 0; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID)) continue;
		if (strcmp(pName.c_str(), m_bag[i].pName) == 0)
		{
			goodsnum++;
		}
	}
	return goodsnum;
}

/*
函数功能:获取物品的背包下标
参数一:物品名字
返回值:找到该物品的第一个下标
*/
int bag::getGoodsIndex(std::string pName)
{
	int firstIndex = -1;
	for (size_t i = 0; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID)) continue;
		if (strcmp(pName.c_str(), m_bag[i].pName) == 0)
		{
			firstIndex = i;
			break;
		}
	}
	return firstIndex;
}

/*
函数功能:获取物品的背包下标
参数一:物品名字
返回值:找到该物品的第一个下标
*/
int bag::getGoodsFirstIndex(std::string pName)
{
	int firstIndex = -1;
	for (size_t i = 0; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID)) continue;
		if (strcmp(pName.c_str(), m_bag[i].pName) == 0)
		{
			firstIndex = i;
			break;
		}
	}
	return firstIndex;
}

/*
函数功能:获取物品的背包下标
参数一:物品名字
参数二:上一个物品的下标
返回值:找到该物品的下一个下标
*/
int bag::getGoodsNextIndex(std::string pName,int first_i)
{
	int nextIndex = -1;
	for (size_t i = first_i+1; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID)) continue;
		if (strcmp(pName.c_str(), m_bag[i].pName) == 0)
		{nextIndex = i;
			break;
		}
	}
	return nextIndex;
}

/*
函数功能:获取背包第一个空格子下标,从前找
返回值:背包第一个空格子下标
*/
int bag::getFirstSpaceIndex()
{
	int firstSpaceIndex = -1;

	for (size_t i = 0; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID))
		{
			firstSpaceIndex = i;
			break;
		}
	}

	return firstSpaceIndex;
}

/*
函数功能:获取背包最后面的空格子下标,从后往前找
返回值:背包最后面的空格子下标
*/
int bag::getLastSpaceIndex()
{
	int lastSpaceIndex = -1;
	for (size_t i = maxSize -1; i >-1; i--)
	{
		if (!(*m_bag[i].ID)) 
		{
			lastSpaceIndex = i;
			break;
		}
	}
	return lastSpaceIndex;
}

/*
函数功能:背包是否有毒药
返回值:0没有毒药，1有红毒，2有绿毒，3有红绿毒
*/
int bag::ifHasPoison()
{
	int a = caclGoodsNumber("绿色毒药（大量）") +
		    caclGoodsNumber("绿色毒药（中量）") +
			caclGoodsNumber("绿色毒药（小量）") +
			caclGoodsNumber("道尊灵毒(绿)");
	int b = caclGoodsNumber("红色毒药（大量）") +
			caclGoodsNumber("红色毒药（中量）") +
			caclGoodsNumber("红色毒药（小量）") +
			caclGoodsNumber("道尊灵毒(红)");
	(a > 0) ? (a = 2) : (a = 0);
	(b > 0) ? (b = 1) : (b = 0);
	return a + b;
}

/*
函数功能:计算背包蓝药，只计算大量蓝药和特级蓝药
返回值:背包蓝药数量
*/
int bag::caclMpGoodsNumer()
{
	return caclGoodsNumber("魔法药（大量）") +
			caclGoodsNumber("特级魔法药") +
			caclGoodsNumber("特级魔法药包") *6+
			caclGoodsNumber("超级魔法药")*6;
}


/*
函数功能:计算背包红药，只计算大量红药和特级红药
返回值:背包蓝药数量
*/
int bag::caclHpGoodsNumer()
{
	return caclGoodsNumber("金创药（大量）") +
		caclGoodsNumber("特级金创药") +
		caclGoodsNumber("特级金创药包") *6+
		caclGoodsNumber("超级金创药")*6;
}


/*
函数功能:计算背包空格子数量
返回值:背包空格子数量
*/
int bag::getBagSpace()
{
	int space = 0;
	for (size_t i = 0; i < maxSize; i++)
	{
		if (!(*m_bag[i].ID)) space++;
	}
	return space;
}


void  bag::getGoodsProcessIndex()
{
	// 清空各个容器
	index_vec_store.clear();
	index_vec_sell_weapon.clear();
	index_vec_sell_cloth.clear();
	index_vec_sell_jewel.clear();
	index_vec_sell_medci.clear();
	classifyBagItems();
    for (size_t i = 0; i < maxSize; i++)
    {
        if ((m_bag[i].ID && *m_bag[i].ID != 0))
        {
            if (m_bag[i].howProcess == 1)
            {
				index_vec_store.push_back(i);
            }
            else if (m_bag[i].howProcess == 2&& m_bag[i].goods_type == 1)
            {
				index_vec_sell_weapon.push_back(i);
            }
            else if (m_bag[i].howProcess == 2 && m_bag[i].goods_type == 3)
            {
				index_vec_sell_cloth.push_back(i);
            }
			else if (m_bag[i].howProcess == 2 && m_bag[i].goods_type == 2)
			{
				index_vec_sell_jewel.push_back(i);
			}
			else if (m_bag[i].howProcess == 2 && m_bag[i].goods_type == 4)
			{
				index_vec_sell_medci.push_back(i);
			}
        }
    }
}