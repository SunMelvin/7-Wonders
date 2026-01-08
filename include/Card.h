#ifndef SEVEN_WONDERS_DUEL_CARD_H
#define SEVEN_WONDERS_DUEL_CARD_H

#include "Global.h"
#include "EffectSystem.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace SevenWondersDuel {

    class Player;
    class Card;

    /**
     * @brief 统一资源费用结构
     * 描述建造卡牌或奇迹所需的成本，包括金币和多种资源组合。
     */
    class ResourceCost {
    private:
        int m_coins = 0;
        std::map<ResourceType, int> m_resources;

    public:
        ResourceCost() = default;
        
        int getCoins() const { return m_coins; }
        const std::map<ResourceType, int>& getResources() const { return m_resources; }
        
        void setCoins(int coins) { m_coins = coins; }
        void setResources(const std::map<ResourceType, int>& res) { m_resources = res; }
        
        /**
         * @brief 增加一种资源需求
         */
        void addResource(ResourceType type, int count) { m_resources[type] += count; }

        /**
         * @brief 检查是否免费 (无金币且无资源需求)
         */
        bool isFree() const { return m_coins == 0 && m_resources.empty(); }
    };

    /**
     * @brief 金字塔卡槽节点
     * 用于构建游戏桌面上的卡牌金字塔结构。每个 Slot 包含一张卡，并记录其位置状态和依赖关系。
     */
    class CardSlot {
    private:
        std::string m_id;             // 对应 Card 的 ID (缓存，方便查找)
        Card* m_cardPtr = nullptr;    // 指向实际 Card 数据的指针
        bool m_isFaceUp = false;      // 是否正面朝上 (可见)
        bool m_isRemoved = false;     // 是否已被玩家拿走

        int m_row = 0;                // 在金字塔中的行号 (从上往下)
        int m_index = 0;              // 行内索引 (从左往右)

        std::vector<int> m_coveredBy; // 压着当前牌的 Slot 索引列表 (依赖关系)

    public:
        CardSlot() = default;

        const std::string& getId() const { return m_id; }
        Card* getCardPtr() const { return m_cardPtr; }
        bool isFaceUp() const { return m_isFaceUp; }
        bool isRemoved() const { return m_isRemoved; }
        int getRow() const { return m_row; }
        int getIndex() const { return m_index; }
        const std::vector<int>& getCoveredBy() const { return m_coveredBy; }

        void setId(const std::string& id) { m_id = id; }
        void setCardPtr(Card* ptr) { m_cardPtr = ptr; }
        void setFaceUp(bool val) { m_isFaceUp = val; }
        void setRemoved(bool val) { m_isRemoved = val; }
        void setRow(int r) { m_row = r; }
        void setIndex(int i) { m_index = i; }
        
        /**
         * @brief 添加一个压在上面的卡槽索引
         */
        void addCoveredBy(int index) { m_coveredBy.push_back(index); }

        /**
         * @brief 通知有一个压在上面的卡被移除了
         * 当所有压在上面的卡都被移除后，此卡会自动翻面 (FaceUp = true)。
         * @return 如果此操作导致卡牌翻面，返回 true
         */
        bool notifyCoveringRemoved(int index);
    };

    /**
     * @brief 卡牌实体类
     * 存储卡牌的静态数据：名称、类型、费用、效果等。
     */
    class Card {
    private:
        std::string m_id;
        std::string m_name;
        int m_age = 0;             // 所属时代 (1, 2, 3)
        CardType m_type = CardType::CIVILIAN;

        ResourceCost m_cost;       // 建造费用

        std::string m_chainTag;          // 此卡提供的连锁标记 (如 "MOON")
        std::string m_requiresChainTag;  // 此卡需要的连锁标记 (如有此标记则免费)

        std::vector<std::shared_ptr<IEffect>> m_effects; // 获取此卡后的即时或被动效果

    public:
        Card() = default;

        const std::string& getId() const { return m_id; }
        const std::string& getName() const { return m_name; }
        int getAge() const { return m_age; }
        CardType getType() const { return m_type; }
        const ResourceCost& getCost() const { return m_cost; }
        const std::string& getChainTag() const { return m_chainTag; }
        const std::string& getRequiresChainTag() const { return m_requiresChainTag; }
        const std::vector<std::shared_ptr<IEffect>>& getEffects() const { return m_effects; }

        void setId(const std::string& id) { m_id = id; }
        void setName(const std::string& name) { m_name = name; }
        void setAge(int age) { m_age = age; }
        void setType(CardType type) { m_type = type; }
        void setCost(const ResourceCost& cost) { m_cost = cost; }
        void setChainTag(const std::string& tag) { m_chainTag = tag; }
        void setRequiresChainTag(const std::string& tag) { m_requiresChainTag = tag; }
        void setEffects(std::vector<std::shared_ptr<IEffect>> effects) { m_effects = std::move(effects); }

        /**
         * @brief 计算此卡提供的胜利点数
         * 部分卡牌分数依赖于玩家状态 (如公会卡)。
         */
        int getVictoryPoints(const Player* self, const Player* opponent) const;
    };

    /**
     * @brief 奇迹实体类
     * 奇迹是一种特殊的“卡牌”，在游戏开始时轮抽获得，建造后提供强力效果。
     */
    class Wonder {
    private:
        std::string m_id;
        std::string m_name;
        ResourceCost m_cost;

        std::vector<std::shared_ptr<IEffect>> m_effects;

        bool m_isBuilt = false;                  // 是否已建造
        const Card* m_builtOverlayCard = nullptr; // 用于建造该奇迹所垫在下面的卡牌 (仅作记录)

    public:
        Wonder() = default;

        const std::string& getId() const { return m_id; }
        const std::string& getName() const { return m_name; }
        const ResourceCost& getCost() const { return m_cost; }
        const std::vector<std::shared_ptr<IEffect>>& getEffects() const { return m_effects; }
        bool isBuilt() const { return m_isBuilt; }
        const Card* getBuiltOverlayCard() const { return m_builtOverlayCard; }

        void setId(const std::string& id) { m_id = id; }
        void setName(const std::string& name) { m_name = name; }
        void setCost(const ResourceCost& cost) { m_cost = cost; }
        void setEffects(std::vector<std::shared_ptr<IEffect>> effects) { m_effects = std::move(effects); }

        /**
         * @brief 标记奇迹为已建造
         * @param overlay 垫在奇迹下面的那张牌 (通常来自于金字塔)
         */
        void build(const Card* overlay);
        
        /**
         * @brief 重置奇迹状态 (用于新游戏初始化)
         */
        void reset();

        /**
         * @brief 计算奇迹提供的胜利点数
         */
        int getVictoryPoints(const Player* self, const Player* opponent) const;
    };

}

#endif // SEVEN_WONDERS_DUEL_CARD_H