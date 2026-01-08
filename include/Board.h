#ifndef SEVEN_WONDERS_DUEL_BOARD_H
#define SEVEN_WONDERS_DUEL_BOARD_H

#include "Global.h"
#include "Card.h"
#include <vector>
#include <string>
#include <iterator>

namespace SevenWondersDuel {

    class Player;

    /**
     * @brief 军事轨道 (冲突指示条)
     * 位于棋盘顶部，记录双方的军事优势。
     * 范围从 -9 (P1胜利) 到 +9 (P2胜利)，0 为中间点。
     */
    class MilitaryTrack {
    private:
        int m_position = 0; // 当前指示物位置
        
        // 掠夺标记状态：[P0-5元, P0-2元, P1-2元, P1-5元]
        // true表示标记还在，false表示已被移除(触发过掠夺)
        bool m_lootTokens[4] = {true, true, true, true}; 

    public:
        int getPosition() const { return m_position; }
        const bool* getLootTokens() const { return m_lootTokens; }
        
        /**
         * @brief 移动冲突指示物
         * @param shields 获得的盾牌数量
         * @param currentPlayerId 当前获得盾牌的玩家 ID (0 或 1)
         * @return 触发的掠夺事件列表 (负数表示 P0 损失金币，正数表示 P1 损失金币)
         */
        std::vector<int> move(int shields, int currentPlayerId);

        /**
         * @brief 获取当前位置对应的胜利点数
         * 游戏结束时结算。
         */
        int getVictoryPoints(int playerId) const;
    };

    /**
     * @brief 卡牌金字塔结构
     * 管理每个时代桌面上卡牌的排列方式 (正三角、倒三角、蛇形)。
     * 处理卡牌的遮挡依赖关系。
     */
    class CardPyramid {
    private:
        std::vector<CardSlot> m_slots; // 所有的卡槽节点

    public:
        const std::vector<CardSlot>& getSlots() const { return m_slots; }
        
        /**
         * @brief 初始化指定时代的金字塔结构
         * @param age 时代 (1, 2, 3)
         * @param deck 该时代的卡牌堆
         */
        void init(int age, const std::vector<Card*>& deck);

        /**
         * @brief 从金字塔中移除一张卡牌
         * 会自动更新剩余卡牌的遮挡/翻面状态。
         * @param cardId 卡牌 ID
         * @return 被移除的 Card 指针，如果未找到或已移除则返回 nullptr
         */
        Card* removeCard(const std::string& cardId);

        // --- 迭代器实现 (用于遍历所有当前可见/可选的卡牌) ---
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = const CardSlot;
            using pointer           = const CardSlot*;
            using reference         = const CardSlot&;

            Iterator(const std::vector<CardSlot>* slots, int index) 
                : m_ptrSlots(slots), m_currentIndex(index) {
                advanceToNextAvailable();
            }

            reference operator*() const { return (*m_ptrSlots)[m_currentIndex]; }
            pointer operator->() const { return &(*m_ptrSlots)[m_currentIndex]; }

            Iterator& operator++() {
                m_currentIndex++;
                advanceToNextAvailable();
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Iterator& a, const Iterator& b) {
                return a.m_currentIndex == b.m_currentIndex && a.m_ptrSlots == b.m_ptrSlots;
            }

            friend bool operator!=(const Iterator& a, const Iterator& b) {
                return !(a == b);
            }

        private:
            const std::vector<CardSlot>* m_ptrSlots;
            int m_currentIndex;

            // 跳过那些已被拿走或被遮挡的卡槽
            void advanceToNextAvailable() {
                while (m_currentIndex < (int)m_ptrSlots->size()) {
                    const CardSlot& slot = (*m_ptrSlots)[m_currentIndex];
                    if (!slot.isRemoved() && slot.getCoveredBy().empty()) {
                        break;
                    }
                    m_currentIndex++;
                }
            }
        };

        Iterator begin() const { return Iterator(&m_slots, 0); }
        Iterator end() const { return Iterator(&m_slots, (int)m_slots.size()); }

    private:
        // 内部构建辅助函数
        void addSlot(int row, int count, bool faceUp, const std::vector<Card*>& deck, int& deckIdx);
        int getAbsIndex(CardSlot* ptr);
        std::vector<CardSlot*> getSlotsByRow(int r);

        // 各时代的具体依赖连接逻辑
        void setupDependenciesAge1();
        void setupDependenciesAge2();
        void setupDependenciesAge3();
    };

    /**
     * @brief 游戏棋盘
     * 整合了军事轨道、卡牌金字塔、弃牌堆以及科技标记区。
     */
    class Board {
    private:
        MilitaryTrack m_militaryTrack;
        CardPyramid m_cardStructure;
        std::vector<Card*> m_discardPile; // 弃牌堆

        // 科技标记 (绿色圆片)
        std::vector<ProgressToken> m_availableProgressTokens; // 棋盘上可选的 5 枚
        std::vector<ProgressToken> m_boxProgressTokens;       // 留在盒子里的 (某些奇迹可查看)

    public:
        Board() = default;

        const MilitaryTrack& getMilitaryTrack() const { return m_militaryTrack; }
        const CardPyramid& getCardStructure() const { return m_cardStructure; }
        const std::vector<Card*>& getDiscardPile() const { return m_discardPile; }
        const std::vector<ProgressToken>& getAvailableProgressTokens() const { return m_availableProgressTokens; }
        const std::vector<ProgressToken>& getBoxProgressTokens() const { return m_boxProgressTokens; }

        // --- 代理方法 ---
        std::vector<int> moveMilitary(int shields, int currentPlayerId);
        void initPyramid(int age, const std::vector<Card*>& deck);
        Card* removeCardFromPyramid(const std::string& cardId);
        
        // --- 弃牌堆管理 ---
        void addToDiscardPile(Card* c);
        Card* removeCardFromDiscardPile(const std::string& cardId);

        // --- 科技标记管理 ---
        void setAvailableProgressTokens(const std::vector<ProgressToken>& tokens);
        void setBoxProgressTokens(const std::vector<ProgressToken>& tokens);
        void addAvailableProgressToken(ProgressToken t);
        void addBoxProgressToken(ProgressToken t);
        bool removeAvailableProgressToken(ProgressToken t);
        bool removeBoxProgressToken(ProgressToken t);

        /**
         * @brief 摧毁玩家的卡牌 (实际上是将其移入弃牌堆)
         */
        void destroyCard(Player* target, CardType color);
    };
}

#endif // SEVEN_WONDERS_DUEL_BOARD_H
