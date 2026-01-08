#ifndef SEVEN_WONDERS_DUEL_AGENT_H
#define SEVEN_WONDERS_DUEL_AGENT_H

#include "GameController.h"
#include "GameView.h"

namespace SevenWondersDuel {

    class InputManager;

    /**
     * @brief 玩家代理接口 (Strategy Pattern)
     * 抽象了"决策者"的概念。无论是人类还是 AI，都通过实现此接口来与游戏互动。
     */
	class IPlayerAgent {
	public:
		virtual ~IPlayerAgent() = default;
		
        /**
         * @brief 决策核心方法
         * 给定当前游戏上下文，返回一个决策 (Action)。
         * @param controller 游戏控制器 (提供 Model 和 State)
         * @param view 视图 (人类玩家需要看，AI 可能不需要但接口保留)
         * @param input 输入管理器 (人类玩家需要用到)
         * @return 决定的动作
         */
		virtual Action decideAction(GameController& controller, GameView& view, InputManager& input) = 0;

		/**
         * @brief 是否为人类玩家
         * 用于 View 层判断是否需要渲染 UI 交互提示，或处理 Controller 中的错误反馈逻辑。
         */
		virtual bool isHuman() const;
	};

	/**
     * @brief 人类玩家代理
     * 它的 decideAction 实现主要是调用 InputManager 阻塞等待键盘输入。
     */
	class HumanAgent : public IPlayerAgent {
	public:
		Action decideAction(GameController& controller, GameView& view, InputManager& input) override;
		bool isHuman() const override;
	};

	/**
     * @brief 随机 AI 代理
     * 在合法动作空间内完全随机选择一个动作。
     * 同时也实现了所有特殊阶段 (如摧毁、陵墓) 的随机逻辑。
     */
	class RandomAIAgent : public IPlayerAgent {
	public:
		Action decideAction(GameController& controller, GameView& view, InputManager& input) override;
	};

	/**
     * @brief 贪心 AI 代理
     * 基础策略：
     * 1. 优先购买分数 (VP) 最高的蓝卡。
     * 2. 其次购买其他可负担的高分卡牌。
     * 3. 尝试建造奇迹。
     * 4. 实在不行就弃牌换钱。
     */
	class GreedyAIAgent : public IPlayerAgent {
	public:
		Action decideAction(GameController& controller, GameView& view, InputManager& input) override;
	};

}

#endif // SEVEN_WONDERS_DUEL_AGENT_H
