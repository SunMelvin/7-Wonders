//
// Created by choyichi on 2025/12/16.
//

#ifndef SEVEN_WONDERS_DUEL_GAMEVIEW_H
#define SEVEN_WONDERS_DUEL_GAMEVIEW_H

#include "Global.h"
#include "GameController.h"
#include <string>
#include <vector>
#include <map>

namespace SevenWondersDuel {

	class GameView {
	public:
		GameView() = default;

		// --- 公共接口 ---

		void clearScreen();
		void renderMainMenu();

        // 打印普通消息或错误
		void printMessage(const std::string& msg);
		void printError(const std::string& msg);
        void printTurnInfo(const Player* player);

		// 获取用户输入 (人类玩家使用) - 包含内部交互循环
		Action promptHumanAction(const GameModel& model, GameState state);

		void renderGame(const GameModel& model);

	private:
        // --- 内部状态 (每帧渲染时更新) ---
        // 用于将界面显示的短ID映射回内部真实ID
        struct RenderContext {
            std::map<int, std::string> cardIdMap;   // Display ID (1, 2...) -> Card String ID
            std::map<int, std::string> wonderIdMap; // Display ID (1, 2...) -> Wonder String ID (针对当前玩家)
            std::map<int, ProgressToken> tokenIdMap;// Display ID (1, 2...) -> Token Enum
            std::vector<std::string> draftWonderIds; // Index -> Wonder ID
            std::vector<std::string> discardIds;    // Index -> Card ID
        } ctx;

		// --- 渲染组件 ---


        void renderDraftPhase(const GameModel& model);

        // 基础绘图
		void printLine(char c = '-', int width = 80);
        void printCentered(const std::string& text, int width = 80);

        // 文本与颜色
		std::string getCardColorCode(CardType t);
        std::string getResetColor();
        std::string getTypeStr(CardType t);
		std::string getTokenName(ProgressToken t);
        std::string resourceName(ResourceType r);
        std::string formatCost(const ResourceCost& cost);
        std::string formatResourcesCompact(const Player& p);

        // 模块渲染
		void renderHeader(const GameModel& model);
		void renderMilitaryTrack(const Board& board);
        void renderProgressTokens(const std::vector<ProgressToken>& tokens);
		void renderPlayerDashboard(const Player& p, bool isCurrent, const Player& opp);
        void renderPyramid(const GameModel& model);
        void renderActionLog(const std::vector<std::string>& log);
        void renderCommandHelp(bool isDraft);

        // 辅助页面
		void renderPlayerDetailFull(const Player& p, const Player& opp);
		void renderCardDetail(const Card& c);
		void renderWonderDetail(const Wonder& w);
		void renderTokenDetail(ProgressToken t);
        void renderDiscardPile(const std::vector<Card*>& pile);
        void renderFullLog(const std::vector<std::string>& log);

        // 内部辅助
        int parseId(const std::string& input, char prefix); // 解析 "C1" -> 1, "1" -> 1
	};

}

#endif // SEVEN_WONDERS_DUEL_GAMEVIEW_H