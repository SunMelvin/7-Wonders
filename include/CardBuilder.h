#ifndef SEVEN_WONDERS_DUEL_CARDBUILDER_H
#define SEVEN_WONDERS_DUEL_CARDBUILDER_H

#include "Card.h"
#include <string>
#include <vector>
#include <memory>

namespace SevenWondersDuel {

    /**
     * @brief 卡牌构建器 (Builder Pattern)
     * 提供链式调用的接口来一步步构造复杂的 Card 对象。
     * 主要用于 Factory 解析 JSON 时的对象组装。
     */
    class CardBuilder {
    private:
        Card m_card;
        std::vector<std::shared_ptr<IEffect>> m_tempEffects;

    public:
        CardBuilder() = default;

        CardBuilder& withId(const std::string& id);
        CardBuilder& withName(const std::string& name);
        CardBuilder& withAge(int age);
        CardBuilder& withType(CardType type);
        CardBuilder& withCost(const ResourceCost& cost);
        CardBuilder& withChainTag(const std::string& tag);
        CardBuilder& withRequiresChainTag(const std::string& tag);
        
        CardBuilder& addEffect(std::shared_ptr<IEffect> effect);
        CardBuilder& setEffects(std::vector<std::shared_ptr<IEffect>> effects);

        /**
         * @brief 构建最终对象
         * 将收集到的所有属性注入 Card 对象并返回。
         */
        Card build();
    };

}

#endif // SEVEN_WONDERS_DUEL_CARDBUILDER_H