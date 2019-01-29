/*===============================================================
* @Author: car
* @Created Time : 2017年04月18日 星期二 14时01分08秒
*
* @File Name: ProbGen.h
* @Description: 一个简单的概率生成器
*
================================================================*/

#ifndef _PROBGEN_H_
#define _PROBGEN_H_

#include <list>

template<class T>
class ProbGen {
public:
	ProbGen() : sum(0) 
	{
		lists.clear();
	}

	ProbGen(const ProbGen& gen) : sum(0)
	{
		for(auto& v : gen.lists)
		{
			Push(v.PItem, v.Weight);
		}
	}

public:
	void Push(const T& item, uint32_t weight) 
	{
		ProbItem pItem(item, weight);
		sum += weight;
		lists.emplace_back(pItem);
	}

	const T& Get(uint32_t idx, T& def) const
	{
		if (lists.size() == 0 || idx >= lists.size()) {
			return def;
		}
		else {
			auto it = lists.begin();
			for (uint32_t i = 0; i < idx; ++i) {
				++it;
			}
			return it->PItem;
		}
	}

	const T& GetOne() const 
	{
		uint32_t chance = rand() % sum;
		uint32_t csum = 0;
		for(auto& v : lists) {
			csum += v.Weight;
			if(csum > chance) {
				return v.PItem;
			}
		}
		return lists.back().PItem;
	}

	T GetOneWithRemove() 
	{
		uint32_t chance = rand() % sum;
		uint32_t csum = 0;
		T p =  lists.back().PItem; 
		auto v = lists.begin();
		while(v != lists.end()) {
			csum += v->Weight;
			if(csum > chance) {
				p = v->PItem;
				sum -= v->Weight;
				lists.erase(v);
				return p;
			}
			v++;
		}

		return p;
	}

	void Clear() 
	{
		sum = 0;
		lists.clear();
	}
	
private:
	class ProbItem {
	public:
		ProbItem(const T& item, uint32_t weight) : PItem(item),Weight(weight) {}

	public:
		T	PItem;
		uint32_t Weight;
	};

private:
	std::list<ProbItem> lists; 
	uint32_t sum;

};


#endif //_PROBGEN_H_
