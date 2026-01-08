#ifndef SEVEN_WONDERS_DUEL_GAMEFACTORY_H
#define SEVEN_WONDERS_DUEL_GAMEFACTORY_H

#include "Card.h"
#include "Global.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace SevenWondersDuel {

    /**
     * @brief 游戏工厂接口 (Abstract Factory)
     * 负责创建游戏中所有的初始化数据对象：卡牌、奇迹、科技标记。
     */
    class IGameFactory {
    public:
        virtual ~IGameFactory() = default;
        virtual std::vector<Card> createCards() = 0;
        virtual std::vector<Wonder> createWonders() = 0;
        virtual std::vector<ProgressToken> createAvailableTokens() = 0;
        virtual std::vector<ProgressToken> createBoxTokens() = 0;
    };

    /**
     * @brief 基础游戏工厂
     * 从 JSON 配置文件中读取并构建数据。
     */
    class BaseGameFactory : public IGameFactory {
    private:
        nlohmann::json m_jsonData;
        std::vector<ProgressToken> m_shuffledTokens; // 预先洗牌的 tokens，分发给 Available 和 Box
        
        // 辅助：解析 JSON 中的费用结构
        ResourceCost parseCost(const nlohmann::json& v);

    public:
        /**
         * @param jsonPath gamedata.json 文件的路径
         */
        BaseGameFactory(const std::string& jsonPath);
        ~BaseGameFactory() override;
        
        std::vector<Card> createCards() override;
        std::vector<Wonder> createWonders() override;
        std::vector<ProgressToken> createAvailableTokens() override;
        std::vector<ProgressToken> createBoxTokens() override;
    };

}

#endif // SEVEN_WONDERS_DUEL_GAMEFACTORY_H