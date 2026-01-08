#ifndef SEVEN_WONDERS_DUEL_PLAYER_H
#define SEVEN_WONDERS_DUEL_PLAYER_H

#include "Global.h"
#include "Card.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <optional>

namespace SevenWondersDuel {

    /**
     * @brief 玩家类
     * 维护玩家个人的所有游戏状态，包括：
     * - 金币
     * - 已建造的建筑 (Card) 和奇迹 (Wonder)
     * - 资源产出能力 (Resource Production)
     * - 科技符号与进度标记
     * - 交易优惠状态
     */
    class Player {
    private:
        // 基础属性
        int m_id; // 0 (先手/左侧) 或 1 (后手/右侧)
        std::string m_name;
        int m_coins;

        // 持有的资产
        std::vector<Card*> m_builtCards;        // 已建建筑
        std::vector<Wonder*> m_builtWonders;    // 已建成的奇迹
        std::vector<Wonder*> m_unbuiltWonders;  // 轮抽拿到但尚未建造的奇迹

        // --- 资源统计缓存 (用于 O(1) 查询) ---
        
        // 玩家拥有的"固定"资源产量 (棕卡/灰卡)
        std::map<ResourceType, int> m_fixedResources; 
        
        // 玩家对对手可见的公开资源产量 (用于计算对手买资源的交易费)
        // 注意：某些卡牌只产资源但不增加此项 (如 Forum/Caravansery 这种多选一卡)
        std::map<ResourceType, int> m_publicProduction; 

        // "多选一"资源 (如：该卡每回合提供 1木 OR 1土)。
        // 这种资源在购买判定时需要进行递归搜索以求最优解。
        std::vector<std::vector<ResourceType>> m_choiceResources; 

        // 科技
        std::map<ScienceSymbol, int> m_scienceSymbols; // 拥有的符号计数
        std::set<ScienceSymbol> m_claimedSciencePairs; // 记录已触发过"配对奖励"的符号，避免重复触发

        // 连锁标记 (用于判定免费建造)
        std::set<std::string> m_ownedChainTags; 

        // 获得的绿色科技标记
        std::set<ProgressToken> m_progressTokens;

        // 特殊Buff: 交易优惠
        // 对应黄色卡牌：若为 true，则向银行购买该类资源固定 1 金币
        std::map<ResourceType, bool> m_tradingDiscounts; 

    public:
        Player(int pid, std::string pname);

        // --- Getters (属性查询) ---
        int getId() const { return m_id; }
        const std::string& getName() const { return m_name; }
        int getCoins() const { return m_coins; }

        const std::vector<Card*>& getBuiltCards() const { return m_builtCards; }
        const std::vector<Wonder*>& getBuiltWonders() const { return m_builtWonders; }
        const std::vector<Wonder*>& getUnbuiltWonders() const { return m_unbuiltWonders; }

        const std::map<ResourceType, int>& getFixedResources() const { return m_fixedResources; }
        const std::map<ResourceType, int>& getPublicProduction() const { return m_publicProduction; }
        const std::vector<std::vector<ResourceType>>& getChoiceResources() const { return m_choiceResources; }

        const std::map<ScienceSymbol, int>& getScienceSymbols() const { return m_scienceSymbols; }
        const std::set<ScienceSymbol>& getClaimedSciencePairs() const { return m_claimedSciencePairs; }
        
        const std::set<std::string>& getOwnedChainTags() const { return m_ownedChainTags; }
        const std::set<ProgressToken>& getProgressTokens() const { return m_progressTokens; }
        const std::map<ResourceType, bool>& getTradingDiscounts() const { return m_tradingDiscounts; }

        // --- 状态辅助查询 ---

        /**
         * @brief 获取已建造的某颜色卡牌数量
         * 常用于公会卡或黄色卡牌的按颜色计分。
         */
        int getCardCount(CardType type) const;

        // --- 资源与购买逻辑核心 ---

        /**
         * @brief 计算向银行购买资源的单价
         * 基础价格 2 + 对手该资源的产量。如果有优惠卡则固定 1。
         */
        int getTradingPrice(ResourceType type, const Player& opponent) const;

        /**
         * @brief 检查是否能负担某项费用
         * 会综合计算：自有资源、多选一资源的最优分配、科技减免(砌体/建筑学)、以及缺口资源的购买成本。
         * @param cost 原始费用
         * @param opponent 对手玩家 (用于计算交易费)
         * @param targetType 目标卡牌类型 (用于判定是否适用科技减免)
         * @return pair<是否买得起, 实际需要支付的总金币>
         */
        std::pair<bool, int> calculateCost(const ResourceCost& cost, const Player& opponent, CardType targetType) const;

        // --- 状态修改 (Mutators) ---

        void payCoins(int amount);
        void gainCoins(int amount);
        
        void setTradingDiscount(ResourceType r, bool active);
        void addClaimedSciencePair(ScienceSymbol s);

        /**
         * @brief 增加资源产量
         * @param isTradable 是否增加对手可见的产量 (影响对手交易费)
         */
        void addResource(ResourceType type, int count, bool isTradable);
        
        void addProductionChoice(const std::vector<ResourceType>& choices);
        void addScienceSymbol(ScienceSymbol s);
        void addChainTag(const std::string& tag);
        
        /**
         * @brief 获得科技标记
         * 并立即应用某些即时生效的 Token 效果 (如 LAW)。
         */
        void addProgressToken(ProgressToken token);

        // --- 建造与管理 ---

        void constructCard(Card* card);

        /**
         * @brief 移除已建造的卡牌 (用于被对手摧毁)
         * @return 被移除的卡牌指针，若无对应颜色卡牌则返回 nullptr
         */
        Card* removeCardByType(CardType type);

        // 奇迹管理
        void addUnbuiltWonder(Wonder* w);
        void removeUnbuiltWonder(const std::string& wonderId);
        void clearUnbuiltWonders();
        
        /**
         * @brief 建造手中的奇迹
         * @param overlayCard 用于垫在奇迹下的卡牌 (通常是刚从金字塔拿的)
         */
        void constructWonder(std::string wonderId, Card* overlayCard);

        // --- 迭代器实现 (方便遍历特定颜色的已建卡牌) ---
        class BuiltCardIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = Card*;
            using pointer           = Card**;
            using reference         = Card*&;

            BuiltCardIterator(const std::vector<Card*>* cards, int index, std::optional<CardType> filter)
                : m_cards(cards), m_index(index), m_filter(filter) {
                advanceToNextValid();
            }

            Card* operator*() const { return (*m_cards)[m_index]; }
            
            BuiltCardIterator& operator++() {
                m_index++;
                advanceToNextValid();
                return *this;
            }

            BuiltCardIterator operator++(int) {
                BuiltCardIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const BuiltCardIterator& a, const BuiltCardIterator& b) {
                return a.m_index == b.m_index && a.m_cards == b.m_cards;
            }

            friend bool operator!=(const BuiltCardIterator& a, const BuiltCardIterator& b) {
                return !(a == b);
            }

        private:
            const std::vector<Card*>* m_cards;
            int m_index;
            std::optional<CardType> m_filter;

            void advanceToNextValid() {
                while (m_index < (int)m_cards->size()) {
                    if (!m_filter.has_value()) break; // No filter, current is valid
                    if ((*m_cards)[m_index]->getType() == m_filter.value()) break; // Match
                    m_index++;
                }
            }
        };

        struct CardRange {
            BuiltCardIterator m_begin;
            BuiltCardIterator m_end;
            BuiltCardIterator begin() const { return m_begin; }
            BuiltCardIterator end() const { return m_end; }
        };

        CardRange getCardsByType(CardType type) const;
        CardRange getAllCards() const;
    };
}

#endif // SEVEN_WONDERS_DUEL_PLAYER_H
