#ifndef SEVEN_WONDERS_DUEL_INPUTMANAGER_H
#define SEVEN_WONDERS_DUEL_INPUTMANAGER_H

#include "Global.h"
#include "GameController.h"
#include "RenderContext.h"
#include <string>
#include <vector>
#include <map>

namespace SevenWondersDuel {

    class GameView;

    /**
     * @brief 输入管理器
     * 负责处理人类玩家的键盘输入，解析命令，并将其转换为 Action 对象。
     * 同时维护交互过程中的错误状态。
     */
    class InputManager {
    public:
        InputManager() = default;

        /**
         * @brief 核心交互循环 (阻塞式)
         * 1. 调用 View 渲染当前帧。
         * 2. 等待用户输入一行命令。
         * 3. 解析命令 (build C1, wonder C2 W1, 等)。
         * 4. 如果解析成功，返回 Action；否则设置 Error 并重绘。
         * 
         * @param view 视图对象引用
         * @param model 游戏模型引用 (用于查询 ID 对应的实体)
         * @param state 当前游戏状态 (决定了哪些命令可用)
         * @return 解析后的有效 Action
         */
        Action promptHumanAction(GameView& view, const GameModel& model, GameState state);

        void setLastError(const std::string& msg) { m_lastError = msg; }
        void clearLastError() { m_lastError = ""; }
        const std::string& getLastError() const { return m_lastError; }

    private:
        /**
         * @brief 解析带前缀的短ID
         * 例如输入 "C12"，前缀 'C'，则返回 12。
         */
        int parseId(const std::string& input, char prefix);

        RenderContext m_ctx; // 当前帧的渲染上下文 (用于 ID 映射)
        std::string m_lastError;
    };

}

#endif // SEVEN_WONDERS_DUEL_INPUTMANAGER_H