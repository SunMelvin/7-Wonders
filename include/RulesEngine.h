#ifndef SEVEN_WONDERS_DUEL_RULESENGINE_H
#define SEVEN_WONDERS_DUEL_RULESENGINE_H

#include "Global.h"
#include "Player.h"

namespace SevenWondersDuel {

    /**
     * @brief 胜利检查结果
     */
    struct VictoryResult {
        bool isGameOver = false;
        VictoryType type = VictoryType::NONE;
        int winnerIndex = -1; // -1: 无, 0: P1, 1: P2
    };

    /**
     * @brief 游戏规则引擎 (Pure Logic)
     * 负责处理那些"是否触发特殊奖励"或"是否获胜"的纯逻辑判断。
     */
    class RulesEngine {
    public:
        /**
         * @brief 检查玩家是否刚刚凑齐了一对新的科技符号
         * 用于判断是否应该获得"选择科技标记"的奖励。
         * @return 如果凑齐了新的一对，返回该符号；否则返回 NONE。
         */
        static ScienceSymbol getNewSciencePairSymbol(const Player& player);

        /**
         * @brief 检查即时胜利条件 (军事压制 / 科技压制)
         * 应在每次动作结束后调用。
         * @return 胜利判定结果
         */
        static VictoryResult checkInstantVictory(const Player& p1, const Player& p2, const Board& board);
    };

}

#endif // SEVEN_WONDERS_DUEL_RULESENGINE_H