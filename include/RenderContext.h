#ifndef SEVEN_WONDERS_DUEL_RENDERCONTEXT_H
#define SEVEN_WONDERS_DUEL_RENDERCONTEXT_H

#include "Global.h"
#include <map>
#include <string>
#include <vector>

namespace SevenWondersDuel {

    /**
     * @brief 渲染上下文
     * 用于解决 CLI 交互的 ID 映射问题。
     * 
     * 问题：游戏内部使用 UUID (如 "card_b_baths")，但这太长太难输入。
     * 解决：渲染时动态生成短 ID (如 "C1", "C2") 显示在屏幕上，
     * 并将 "1 -> card_b_baths" 的映射关系存储在此结构体中。
     * 当 InputManager 收到用户输入 "C1" 时，查表即可得到原始 ID。
     */
    struct RenderContext {
        std::map<int, std::string> cardIdMap;       // 金字塔卡牌 (C1, C2...)
        std::map<int, std::string> wonderIdMap;     // 奇迹 (W1, W2...)
        std::map<int, ProgressToken> tokenIdMap;    // 桌面科技标记 (S1, S2...)
        std::map<int, std::string> oppCardIdMap;    // 对手已建成卡牌 (用于摧毁选择 T1, T2...)
        std::map<int, std::string> discardIdMap;    // 弃牌堆卡牌 (用于陵墓选择 D1, D2...)
        std::map<int, ProgressToken> boxTokenIdMap; // 盒子里的标记 (用于图书馆选择 S1...)
        
        std::vector<std::string> draftWonderIds;    // 轮抽阶段的奇迹 ID 列表 (直接按索引 1-4 选择)

        void clear();
    };

}

#endif // SEVEN_WONDERS_DUEL_RENDERCONTEXT_H