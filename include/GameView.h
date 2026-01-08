#ifndef SEVEN_WONDERS_DUEL_GAMEVIEW_H
#define SEVEN_WONDERS_DUEL_GAMEVIEW_H

#include "Global.h"
#include "GameController.h"
#include "RenderContext.h"
#include <string>
#include <vector>

namespace SevenWondersDuel {

    /**
     * @brief 游戏视图层 (Console UI)
     * 负责将游戏 Model 的数据渲染到控制台屏幕。
     * 遵循 "Immediate Mode GUI" 风格，每帧重绘。
     */
	class GameView {
	public:
		GameView() = default;

		// --- 公共接口 ---
        
        /**
         * @brief 清屏
         * 使用 ANSI 转义序列清空终端内容。
         */
		void clearScreen();

        /**
         * @brief 渲染主菜单
         * 显示游戏 Logo 和模式选择。
         */
		void renderMainMenu();

        /**
         * @brief 提示输入玩家姓名
         * @param playerIndex 玩家序号 (1 或 2)
         * @param defaultName 如果用户直接回车，使用的默认名
         * @return 用户输入的姓名
         */
        std::string promptPlayerName(int playerIndex, const std::string& defaultName);

		void printMessage(const std::string& msg);

        /**
         * @brief 核心渲染入口
         * 根据当前 GameState 自动分发到具体的子渲染函数 (如 renderDraftPhase, renderPyramid)。
         * @param model 游戏数据模型
         * @param state 当前游戏状态
         * @param ctx [Out] 渲染上下文，用于收集屏幕上显示的元素 ID 映射 (例如将 "Card-101" 映射为 "1")
         * @param lastError 上一次操作的错误信息 (用于显示在底部)
         */
        void renderGame(const GameModel& model, GameState state, RenderContext& ctx, const std::string& lastError);
        
        /**
         * @brief AI 回合专用渲染
         * 不涉及用户输入上下文收集，仅用于展示 AI 操作过程。
         */
        void renderGameForAI(const GameModel& model, GameState state);

        // --- 详情页渲染 (Public, 供 InputManager 处理 'info' 命令调用) ---
		void renderPlayerDashboard(const Player& p, bool isCurrent, const Player& opp, int& wonderCounter, const Board& board, RenderContext& ctx, bool targetMode = false);
        void renderPlayerDetailFull(const Player& p, const Player& opp, const Board& board);
		void renderCardDetail(const Card& c);
		void renderWonderDetail(const Wonder& w);
		void renderTokenDetail(ProgressToken t);
        void renderDiscardPile(const std::vector<Card*>& pile);
        void renderFullLog(const std::vector<std::string>& log);

	private:
		// --- 渲染组件 ---

        // 分阶段/状态渲染逻辑
        void renderDraftPhase(const GameModel& model, RenderContext& ctx, const std::string& lastError);
        void renderTokenSelection(const GameModel& model, bool fromBox, RenderContext& ctx, const std::string& lastError);
        void renderDestructionPhase(const GameModel& model, RenderContext& ctx, const std::string& lastError);
        void renderDiscardBuildPhase(const GameModel& model, RenderContext& ctx, const std::string& lastError);
        void renderStartPlayerSelect(const GameModel& model, const std::string& lastError);

        // 基础绘图辅助
		void printLine(char c = '-', int width = 80);
        void printCentered(const std::string& text, int width = 80);

        // 格式化与颜色辅助
		std::string getCardColorCode(CardType t);
        std::string getResetColor();
        std::string getTypeStr(CardType t);
		std::string getTokenName(ProgressToken t);
        std::string resourceName(ResourceType r);
        std::string formatCost(const ResourceCost& cost);
        std::string formatResourcesCompact(const Player& p);

        // 子模块渲染
		void renderHeader(const GameModel& model);
		void renderMilitaryTrack(const Board& board);
        void renderProgressTokens(const std::vector<ProgressToken>& tokens, RenderContext& ctx, bool isBoxContext = false);

        /**
         * @brief 渲染卡牌金字塔
         * 计算每个 Slot 的位置、遮挡关系，并生成可视化 ID (C1, C2...)。
         */
        void renderPyramid(const GameModel& model, RenderContext& ctx);
        
        void renderActionLog(const std::vector<std::string>& log);
        void renderCommandHelp(GameState state);
        void renderErrorMessage(const std::string& lastError);
	};

}

#endif // SEVEN_WONDERS_DUEL_GAMEVIEW_H