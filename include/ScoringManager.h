#ifndef SEVEN_WONDERS_DUEL_SCORINGMANAGER_H
#define SEVEN_WONDERS_DUEL_SCORINGMANAGER_H

#include "Player.h"

namespace SevenWondersDuel {

    /**
     * @brief 计分管理器
     * 负责游戏结束时的"平民胜利"分数计算。
     */
    class ScoringManager {
    public:
        /**
         * @brief 计算玩家总分
         * 包含：卡牌分数 (含行会)、奇迹分数、军事分数、金币分数 (3:1)、科技标记分数。
         */
        static int calculateScore(const Player& player, const Player& opponent, const Board& board);

        /**
         * @brief 计算蓝卡总分 (用于平局决胜)
         * 规则：如果总分相同，拥有更多蓝卡分数的玩家获胜。
         */
        static int calculateBluePoints(const Player& player, const Player& opponent);
    };

}

#endif // SEVEN_WONDERS_DUEL_SCORINGMANAGER_H