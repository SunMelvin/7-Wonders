#ifndef SEVEN_WONDERS_DUEL_GAMECONTROLLER_H
#define SEVEN_WONDERS_DUEL_GAMECONTROLLER_H

#include "Global.h"
#include "Player.h"
#include "Board.h"
#include "Card.h"
#include <memory>
#include <vector>
#include <string>
#include <random>

namespace SevenWondersDuel {

    class IGameStateLogic;

    /**
     * @brief 游戏数据模型 (Model Layer Root)
     * 作为一个聚合根，持有游戏所有的动态数据，但不包含业务规则逻辑。
     * 作用：
     * 1. 提供只读接口供 View 层渲染。
     * 2. 提供可变接口供 Controller 层修改。
     */
    class GameModel {
    private:
        std::vector<std::unique_ptr<Player>> m_players; // 两个玩家
        std::unique_ptr<Board> m_board;               // 游戏棋盘

        int m_currentAge = 0;           // 当前时代 (1, 2, 3)
        int m_currentPlayerIndex = 0;   // 当前行动玩家索引 (0 或 1)
        int m_winnerIndex = -1;         // 获胜者索引 (-1:未结束, 0:P1胜, 1:P2胜)
        VictoryType m_victoryType = VictoryType::NONE;

        // 奇迹轮抽相关
        std::vector<Wonder*> m_draftPool;        // 当前轮抽区可见的4张奇迹
        std::vector<Wonder*> m_remainingWonders; // 剩下的奇迹（未被选入当前轮抽池）

        // 数据仓库 (所有实体对象的实际存储地)
        std::vector<Card> m_allCards;
        std::vector<Wonder> m_allWonders;

        std::vector<std::string> m_gameLog; // 游戏日志

    public:
        GameModel();

        // --- Getters (Read-Only) ---

        const Player* getCurrentPlayer() const { return m_players[m_currentPlayerIndex].get(); }
        const Player* getOpponent() const { return m_players[1 - m_currentPlayerIndex].get(); }
        
        const Board* getBoard() const { return m_board.get(); }
        const std::vector<std::unique_ptr<Player>>& getPlayers() const { return m_players; }
        
        int getCurrentAge() const { return m_currentAge; }
        int getCurrentPlayerIndex() const { return m_currentPlayerIndex; }
        int getWinnerIndex() const { return m_winnerIndex; }
        VictoryType getVictoryType() const { return m_victoryType; }

        const std::vector<Wonder*>& getDraftPool() const { return m_draftPool; }
        const std::vector<Wonder*>& getRemainingWonders() const { return m_remainingWonders; }
        const std::vector<Card>& getAllCards() const { return m_allCards; }
        const std::vector<Wonder>& getAllWonders() const { return m_allWonders; }
        const std::vector<std::string>& getGameLog() const { return m_gameLog; }

        // --- Mutators (Controlled Access) ---
        
        Player* getCurrentPlayerMut() { return m_players[m_currentPlayerIndex].get(); }
        Player* getOpponentMut() { return m_players[1 - m_currentPlayerIndex].get(); }
        Board* getBoardMut() { return m_board.get(); }

        void clearPlayers() { m_players.clear(); }
        void addPlayer(std::unique_ptr<Player> p) { m_players.push_back(std::move(p)); }
        
        void setCurrentAge(int age) { m_currentAge = age; }
        void setCurrentPlayerIndex(int index) { m_currentPlayerIndex = index; }
        void setWinnerIndex(int index) { m_winnerIndex = index; }
        void setVictoryType(VictoryType type) { m_victoryType = type; }

        // 奇迹轮抽池管理
        void clearDraftPool() { m_draftPool.clear(); }
        void addToDraftPool(Wonder* w) { m_draftPool.push_back(w); }
        void removeFromDraftPool(const std::string& wonderId);

        void clearRemainingWonders() { m_remainingWonders.clear(); }
        void addToRemainingWonders(Wonder* w) { m_remainingWonders.push_back(w); }
        void popRemainingWonder();
        Wonder* backRemainingWonder();

        // 数据填充
        void populateData(std::vector<Card> cards, std::vector<Wonder> wonders);

        // 查找辅助
        Card* findCardById(const std::string& id);
        const Card* findCardById(const std::string& id) const;
        Wonder* findWonderById(const std::string& id);
        const Wonder* findWonderById(const std::string& id) const;

        std::vector<Wonder*> getPointersToAllWonders();

        // 日志管理
        void addLog(const std::string& msg);
        void clearLog();

        int getRemainingCardCount() const;
    };

    /**
     * @brief 游戏核心控制器 (Controller Layer)
     * 实现了 ILogger 和 IGameActions 接口，供 EffectSystem 回调使用。
     * 
     * 职责：
     * 1. 游戏初始化 (Initialize)
     * 2. 主循环控制 (StartGame)
     * 3. 接收并处理 Action (ProcessAction)
     * 4. 维护当前游戏状态机 (GameStateLogic)
     * 5. 时代切换与结算
     */
    class GameController : public ILogger, public IGameActions {
        // 允许具体的 Command 类访问 Controller 的私有方法来修改模型
        friend class DraftWonderCommand;
        friend class BuildCardCommand;
        friend class DiscardCardCommand;
        friend class BuildWonderCommand;
        friend class SelectProgressTokenCommand;
        friend class DestructionCommand;
        friend class SelectFromDiscardCommand;
        friend class ChooseStartingPlayerCommand;

    public:
        GameController();
        ~GameController();

        /**
         * @brief 初始化游戏
         * 加载数据，创建玩家，准备初始状态。
         */
        void initializeGame(const std::string& jsonPath, const std::string& p1Name, const std::string& p2Name);
        
        /**
         * @brief 开始游戏
         * 启动第一个奇迹轮抽阶段。
         */
        void startGame();

        GameState getState() const;
        const GameModel& getModel() const;

        /**
         * @brief 验证动作是否合法
         * 委托给当前的 m_stateLogic 进行验证。
         */
        ActionResult validateAction(const Action& action);

        /**
         * @brief 执行动作
         * 1. 验证动作
         * 2. 创建对应 Command
         * 3. 执行 Command
         * 4. 触发 EffectSystem
         * 5. 检查胜利条件
         * 6. 切换玩家或状态
         * @return 成功执行返回 true
         */
        bool processAction(const Action& action);

        // --- IGameActions 实现 (供 EffectSystem 回调) ---
        void setPendingDestructionType(CardType t) override { m_pendingDestructionType = t; }
        CardType getPendingDestructionType() const { return m_pendingDestructionType; }

        void setState(GameState newState) override;
        std::vector<int> moveMilitary(int shields, int playerId) override;
        bool isDiscardPileEmpty() const override;
        void grantExtraTurn() override { m_extraTurnPending = true; }
        void addLog(const std::string& msg) override { m_model->addLog(msg); }

    private:
        std::unique_ptr<GameModel> m_model;
        std::unique_ptr<IGameStateLogic> m_stateLogic; // 当前状态逻辑处理对象
        GameState m_currentState = GameState::WONDER_DRAFT_PHASE_1;

        bool m_extraTurnPending = false; // 是否触发了再次行动 (如奇迹效果)
        int m_draftTurnCount = 0;        // 轮抽阶段计数

        std::mt19937 m_rng; // 随机数生成器

        CardType m_pendingDestructionType = CardType::CIVILIAN; // 等待摧毁的卡牌类型

        void updateStateLogic(GameState newState);

        // --- 内部流程 ---
        void setupAge(int age);
        void prepareNextAge();
        std::vector<Card*> prepareDeckForAge(int age);
        void initWondersDeck();
        void dealWondersToDraft();
        
        void onTurnEnd();
        void switchPlayer();
        void checkVictoryConditions();
        
        // --- 辅助逻辑 ---
        void resolveMilitaryLoot(const std::vector<int>& lootEvents);
        bool checkForNewSciencePairs(Player* p);
        Card* findCardInPyramid(const std::string& id);
        Wonder* findWonderInHand(const Player* p, const std::string& id);
    };
}

#endif // SEVEN_WONDERS_DUEL_GAMECONTROLLER_H