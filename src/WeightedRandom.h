#ifndef WEIGHTEDRANDOM
#define WEIGHTEDRANDOM

#include <random>
#include <vector>

/**
 *  @brief  加权随机候选项
 *
 *  @param  value   被选中时返回的候选值
 *  @param  weight  候选值参与随机选择的权重
**/
template<typename T>
struct WeightedChoice
{
    //被选中时返回给调用方的候选值
    T value;

    //该候选值参与随机选择的权重,小于等于0时不会被选中
    float weight=0.0f;
};

//从候选项列表中按权重随机选择一个值,供波次系统选择敌机类型
template<typename T>
T chooseWeighted(const std::vector<WeightedChoice<T>>& choices,std::mt19937& gen,T fallback)
{
    //统计所有正权重的总和,忽略不参与选择的非正权重候选项
    float totalWeight=0.0f;
    for(const auto& choice:choices)
    {
        if(choice.weight>0.0f)
        {
            totalWeight+=choice.weight;
        }
    }
    //没有有效权重时返回调用方提供的兜底值
    if(totalWeight<=0.0f)
    {
        return fallback;
    }
    //在总权重区间内随机投点
    std::uniform_real_distribution<float> distribution(0.0f,totalWeight);
    float roll=distribution(gen);
    float cursor=0.0f;
    //累加权重区间,随机点落入哪个区间就返回对应候选值
    for(const auto& choice:choices)
    {
        if(choice.weight<=0.0f)
        {
            continue;
        }
        cursor+=choice.weight;
        if(roll<=cursor)
        {
            return choice.value;
        }
    }
    //浮点边界或候选项异常时返回兜底值
    return fallback;
}

#endif
