#include "GameView.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <limits>
#include <algorithm>
#include <map>
#include <cmath>

namespace SevenWondersDuel {

    // ==========================================================
    //  基础工具
    // ==========================================================

    void GameView::clearScreen() {
        std::cout << "\033[2J\033[1;1H";
    }

    void GameView::printLine(char c, int width) {
        std::cout << std::string(width, c) << "\n";
    }

    void GameView::printCentered(const std::string& text, int width) {
        int len = 0;
        bool inEsc = false;
        for(char c : text) {
            if(c == '\033') inEsc = true;
            if(!inEsc) len++;
            if(inEsc && c == 'm') inEsc = false;
        }
        int padding = (width - len) / 2;
        if (padding > 0) std::cout << std::string(padding, ' ');
        std::cout << text << "\n";
    }

    int GameView::parseId(const std::string& input, char prefix) {
        if (input.empty()) return -1;
        std::string numPart = input;

        // 允许用户输入 "C1" 或 "1"
        if (toupper(input[0]) == prefix) {
            if (input.size() > 1) numPart = input.substr(1);
            else return -1;
        }

        try {
            return std::stoi(numPart);
        } catch (...) {
            return -1;
        }
    }

    // ==========================================================
    //  颜色与文本
    // ==========================================================

    std::string GameView::getCardColorCode(CardType t) {
        switch(t) {
            case CardType::RAW_MATERIAL: return "\033[33m"; // Brown
            case CardType::MANUFACTURED: return "\033[90m"; // Grey
            case CardType::CIVILIAN: return "\033[34m";     // Blue
            case CardType::SCIENTIFIC: return "\033[32m";   // Green
            case CardType::COMMERCIAL: return "\033[93m";   // Yellow
            case CardType::MILITARY: return "\033[31m";     // Red
            case CardType::GUILD: return "\033[35m";        // Purple
            case CardType::WONDER: return "\033[36m";       // Cyan
            default: return "\033[0m";
        }
    }

    std::string GameView::getResetColor() { return "\033[0m"; }

    std::string GameView::getTypeStr(CardType t) {
        std::string name;
        switch(t) {
            case CardType::RAW_MATERIAL: name = "Brown"; break;
            case CardType::MANUFACTURED: name = "Grey"; break;
            case CardType::CIVILIAN: name = "Blue"; break;
            case CardType::SCIENTIFIC: name = "Green"; break;
            case CardType::COMMERCIAL: name = "Yellow"; break;
            case CardType::MILITARY: name = "Red"; break;
            case CardType::GUILD: name = "Guild"; break;
            default: name = "Unknown";
        }
        return getCardColorCode(t) + name + getResetColor();
    }

    void GameView::printMessage(const std::string& msg) {
        std::cout << "\033[96m[INFO] " << msg << "\033[0m\n";
    }

    void GameView::printError(const std::string& msg) {
        std::cout << "\033[91m[ERROR] " << msg << "\033[0m\n";
    }

    void GameView::printTurnInfo(const Player* player) {
        // Dashboard contains turn info
    }

    std::string GameView::getTokenName(ProgressToken t) {
        switch(t) {
            case ProgressToken::AGRICULTURE: return "Agriculture";
            case ProgressToken::URBANISM: return "Urbanism";
            case ProgressToken::STRATEGY: return "Strategy";
            case ProgressToken::THEOLOGY: return "Theology";
            case ProgressToken::ECONOMY: return "Economy";
            case ProgressToken::MASONRY: return "Masonry";
            case ProgressToken::ARCHITECTURE: return "Architecture";
            case ProgressToken::LAW: return "Law";
            case ProgressToken::MATHEMATICS: return "Mathematics";
            case ProgressToken::PHILOSOPHY: return "Philosophy";
            default: return "Unknown";
        }
    }

    std::string GameView::resourceName(ResourceType r) {
        switch(r) {
            case ResourceType::WOOD: return "Wood";
            case ResourceType::STONE: return "Stone";
            case ResourceType::CLAY: return "Clay";
            case ResourceType::PAPER: return "Paper";
            case ResourceType::GLASS: return "Glass";
        }
        return "?";
    }

    std::string GameView::formatCost(const ResourceCost& cost) {
        std::stringstream ss;
        if(cost.coins > 0) ss << "$" << cost.coins << " ";
        for(auto [r,v]: cost.resources) {
            if(v>0) ss << v << resourceName(r).substr(0,1) << " ";
        }
        std::string s = ss.str();
        if(s.empty()) return "Free";
        return s;
    }

    std::string GameView::formatResourcesCompact(const Player& p) {
        std::stringstream ss;
        ss << "W:" << p.fixedResources.at(ResourceType::WOOD) << " ";
        ss << "C:" << p.fixedResources.at(ResourceType::CLAY) << " ";
        ss << "S:" << p.fixedResources.at(ResourceType::STONE) << " ";
        ss << "G:" << p.fixedResources.at(ResourceType::GLASS) << " ";
        ss << "P:" << p.fixedResources.at(ResourceType::PAPER);
        return ss.str();
    }

    // ==========================================================
    //  主渲染逻辑
    // ==========================================================

    void GameView::renderMainMenu() {
        clearScreen();
        std::cout << "\n";
        printLine('=', 80);
        printCentered("\033[1;33m7   W O N D E R S    D U E L\033[0m");
        printLine('=', 80);
        std::cout << "\n";
        printCentered("Please Select Game Mode:");
        std::cout << "\n";
        std::string indent(28, ' ');
        std::cout << indent << "[1] Human vs Human\n";
        std::cout << indent << "[2] Human vs AI (Recommended)\n";
        std::cout << indent << "[3] AI vs AI (Watch Mode)\n";
        std::cout << indent << "[4] Quit Game\n";
        std::cout << "\n";
        printLine('=', 80);
        std::cout << "  Input > ";
    }

    // --- 奇迹轮抽阶段 ---
    void GameView::renderDraftPhase(const GameModel& model) {
        clearScreen();
        printLine('=');
        printCentered("\033[1;36mWONDER DRAFT PHASE\033[0m");
        printLine('=');
        std::cout << "\n";

        // 只显示当前玩家信息
        const Player* p = model.getCurrentPlayer();
        std::string nameTag = "\033[1;33m[" + p->name + "]\033[0m";
        std::cout << "  " << nameTag << " Select a Wonder to keep:\n\n";

        ctx.draftWonderIds.clear(); // 清空旧数据
        int idx = 1;
        for (const auto& w : model.draftPool) {
            ctx.draftWonderIds.push_back(w->id);
            std::cout << "  [" << idx << "] \033[1;37m" << std::left << std::setw(20) << w->name << "\033[0m";
            std::cout << " Cost: " << formatCost(w->cost) << "\n";
            std::cout << "       Effect: ";
            for(auto& eff : w->effects) std::cout << eff->getDescription() << " ";
            std::cout << "\n\n";
            idx++;
        }
        printLine('-');
        renderCommandHelp(true);
    }

    // --- 主游戏阶段组件 ---

    void GameView::renderMilitaryTrack(const Board& board) {
        int pos = board.militaryTrack.position;
        std::stringstream ss;
        ss << "        P1  ";
        for (int i = -9; i <= 9; ++i) {
            if (i == pos) ss << "\033[1;31m@\033[0m ";
            else if (i == 0) ss << "| ";
            else ss << "- ";
        }
        ss << " P2";
        std::cout << ss.str() << "\n";

        // Loot Tokens
        std::cout << "            "
                  << (board.militaryTrack.lootTokens[1] ? "[$ 5] " : "      ")
                  << (board.militaryTrack.lootTokens[0] ? "[$ 2]" : "     ")
                  << "               "
                  << (board.militaryTrack.lootTokens[2] ? "[$ 2] " : "      ")
                  << (board.militaryTrack.lootTokens[3] ? "[$ 5]" : "     ")
                  << "\n";
    }

    void GameView::renderProgressTokens(const std::vector<ProgressToken>& tokens) {
        ctx.tokenIdMap.clear();
        if (tokens.empty()) return;
        std::cout << "[TOKENS] ";
        for (size_t i = 0; i < tokens.size(); ++i) {
            int displayId = i + 1;
            ctx.tokenIdMap[displayId] = tokens[i];
            // [S1]Name
            std::cout << "\033[32m[S" << displayId << "]" << getTokenName(tokens[i]) << "\033[0m  ";
        }
        std::cout << "\n";
    }

    void GameView::renderHeader(const GameModel& model) {
        std::stringstream ss;
        ss << "[ 7 WONDERS DUEL - AGE " << model.currentAge << " ]";
        printLine('=');
        printCentered("\033[1;37m" + ss.str() + "\033[0m");
        renderMilitaryTrack(*model.board);
        renderProgressTokens(model.board->availableProgressTokens);
        printLine('-');
    }

    void GameView::renderPlayerDashboard(const Player& p, bool isCurrent, const Player& opp) {
        std::string nameTag = isCurrent ? ("\033[1;36m[" + p.name + "]\033[0m") : ("[" + p.name + "]");

        int displayVP = p.getScore(opp) - (p.coins/3);
        std::cout << nameTag
                  << " Coins:\033[33m" << p.coins << "\033[0m"
                  << " VP:\033[36m" << displayVP << "\033[0m "
                  << formatResourcesCompact(p) << "\n";

        // Wonders with IDs only for current player
        std::cout << "Wonder: ";
        if (isCurrent) ctx.wonderIdMap.clear();

        // Built
        for(auto w : p.builtWonders) {
            std::cout << "\033[32m[X]" << w->name << "\033[0m  ";
        }

        // Unbuilt (Needs IDs for current player)
        int wIdx = 1;
        for(auto w : p.unbuiltWonders) {
            if (isCurrent) {
                ctx.wonderIdMap[wIdx] = w->id;
                std::cout << "[W" << wIdx << "]" << w->name << "  ";
                wIdx++;
            } else {
                std::cout << "[ ]" << w->name << "  ";
            }
        }
        std::cout << "\n";
        printLine('-');
    }

    void GameView::renderPyramid(const GameModel& model) {
        int remaining = model.getRemainingCardCount();
        int discardCount = model.board->discardPile.size();

        std::cout << "           PYRAMID: " << remaining << " cards left   |   DISCARD: " << discardCount << " cards\n";

        const auto& slots = model.board->cardStructure.slots;
        if (slots.empty()) return;

        // Reset Card Map
        ctx.cardIdMap.clear();

        // Group by row
        std::map<int, std::vector<const CardSlot*>> rows;
        int maxRow = 0;
        // Use a persistent counter for display ID across the whole pyramid structure
        // UI design implies numbering follows the structure order (1..20)
        int displayIdCounter = 1;

        // We need to iterate slots in the order they were added to ensure consistent numbering
        for (const auto& slot : slots) {
            rows[slot.row].push_back(&slot);
            if (slot.row > maxRow) maxRow = slot.row;
        }

        // Render each row
        for (int r = 0; r <= maxRow; ++r) {
            const auto& rowSlots = rows[r];
            if (rowSlots.empty()) continue;

            // Centering logic
            int cardWidth = 11; // "[ C12 ]  "
            int rowLen = rowSlots.size() * cardWidth;
            int padding = (80 - rowLen) / 2;
            std::cout << std::string(std::max(0, padding), ' ');

            for (const auto* slot : rowSlots) {
                // Determine ID for this slot (based on its index in the flat slots vector)
                // Note: slots vector is ordered by row usually.
                // Let's rely on the pointer arithmetic or index stored in slot
                // Actually Board.h stores `index` as row-index.
                // Let's just use the `displayIdCounter` logic or mapping based on `slots` iteration order.
                // Better: find the slot in the main vector to get its absolute index 1..20
                int absIndex = -1;
                for(size_t k=0; k<slots.size(); ++k) {
                    if (&slots[k] == slot) { absIndex = k + 1; break; }
                }

                if (slot->isRemoved) {
                    std::cout << "           ";
                } else if (!slot->isFaceUp) {
                    std::cout << " [\033[90m ? ? ? \033[0m] ";
                } else {
                    Card* c = slot->cardPtr;
                    // Register ID for interaction
                    ctx.cardIdMap[absIndex] = c->id;

                    std::string color = getCardColorCode(c->type);
                    // Format: [ C1 ] or [ C12 ]
                    std::stringstream ss;
                    ss << " C" << absIndex << " ";
                    std::string label = ss.str();

                    // Pad label to fit width 7 inside brackets
                    while(label.length() < 7) label += " ";

                    std::cout << " [" << color << label << getResetColor() << "] ";
                }
            }
            std::cout << "\n";
        }
        printLine('-');
    }

    void GameView::renderActionLog(const std::vector<std::string>& log) {
        std::cout << " [LAST ACTION]\n";
        if (log.empty()) {
            std::cout << " > Game Started.\n";
        } else {
            size_t count = std::min((size_t)2, log.size());
            for (size_t i = 0; i < count; ++i) {
                std::cout << " > " << log[log.size() - count + i] << "\n";
            }
        }
        printLine('=');
    }

    void GameView::renderCommandHelp(bool isDraft) {
        std::cout << " [COMMANDS]\n";
        if (isDraft) {
            std::cout << " - pick <ID>      : Pick a wonder (e.g., 'pick 1')\n";
            std::cout << " - detail         : View your stats\n";
        } else {
            std::cout << " - build <ID>       : Build card (e.g., 'build C1' or 'build 1')\n";
            std::cout << " - wonder <CID> <WID>: Build Wonder <WID> using card <CID> (e.g., 'wonder 1 W1')\n";
            std::cout << " - discard <ID>     : Sell card <ID> for coins\n";
            std::cout << " - info <ID>        : Info (Card 'C1', Wonder 'W1', Token 'S1')\n";
            std::cout << " - pile / log       : View Discard Pile / Full Log\n";
            std::cout << " - detail <1/2>     : View player detail\n";
        }
    }

    void GameView::renderGame(const GameModel& model) {
        clearScreen();
        renderHeader(model);
        renderPlayerDashboard(*model.players[0], model.currentPlayerIndex == 0, *model.players[1]);
        renderPyramid(model);
        renderPlayerDashboard(*model.players[1], model.currentPlayerIndex == 1, *model.players[0]);
        renderActionLog(model.gameLog);
        renderCommandHelp(false);
    }

    // ==========================================================
    //  详情页
    // ==========================================================

    void GameView::renderPlayerDetailFull(const Player& p, const Player& opp) {
        clearScreen();
        printLine('=');
        printCentered("DETAIL: " + p.name);
        std::cout << "\n";
        std::cout << " [1] BASIC INFO\n";
        std::cout << "     Coins: " << p.coins << "   VP: " << p.getScore(opp) << "\n\n";

        std::cout << " [2] RESOURCES\n";
        std::cout << "     " << formatResourcesCompact(p) << "\n";
        std::cout << "     Buy Costs (Wood/Clay/Stone/Glass/Paper): ";
        std::vector<ResourceType> types = {ResourceType::WOOD, ResourceType::CLAY, ResourceType::STONE, ResourceType::GLASS, ResourceType::PAPER};
        for (auto t : types) std::cout << p.getTradingPrice(t, opp) << "$ ";
        std::cout << "\n\n";

        std::cout << " [3] SCIENCE\n";
        std::cout << "     Symbols: ";
        for(auto [s, c] : p.scienceSymbols) {
            if(c > 0 && s != ScienceSymbol::NONE) std::cout << "[" << (int)s << "]x" << c << " ";
        }
        std::cout << "\n\n";

        std::cout << " [4] WONDERS (All)\n";
        for(auto w : p.builtWonders) std::cout << "     [Built] " << w->name << "\n";
        for(auto w : p.unbuiltWonders) std::cout << "     [Plan ] " << w->name << "\n";

        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    void GameView::renderCardDetail(const Card& c) {
        clearScreen();
        printLine('=');
        printCentered("INFO: " + c.name);
        std::cout << "\n";
        std::cout << "  Type:   " << getTypeStr(c.type) << "\n";
        std::cout << "  Cost:   " << formatCost(c.cost) << "\n\n";
        std::cout << "  --- EFFECTS ---\n";
        for(auto& eff : c.effects) std::cout << "  * " << eff->getDescription() << "\n";
        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    void GameView::renderWonderDetail(const Wonder& w) {
        clearScreen();
        printLine('=');
        printCentered("INFO: " + w.name);
        std::cout << "\n";
        std::cout << "  Cost:   " << formatCost(w.cost) << "\n\n";
        std::cout << "  --- EFFECTS ---\n";
        for(auto& eff : w.effects) std::cout << "  * " << eff->getDescription() << "\n";
        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    void GameView::renderTokenDetail(ProgressToken t) {
        clearScreen();
        printLine('=');
        printCentered("TOKEN: " + getTokenName(t));
        std::cout << "\n";
        std::cout << "  (Effect description placeholder)\n";
        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    void GameView::renderDiscardPile(const std::vector<Card*>& pile) {
        clearScreen();
        printLine('=');
        printCentered("DISCARD PILE");
        std::cout << " Total Cards: " << pile.size() << "\n\n";
        int idx = 1;
        for(auto c : pile) {
            std::cout << "  [D" << idx++ << "] " << c->name << " (" << getTypeStr(c->type) << ")\n";
        }
        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    void GameView::renderFullLog(const std::vector<std::string>& log) {
        clearScreen();
        printLine('=');
        printCentered("GAME LOG");
        std::cout << "\n";
        for(const auto& l : log) std::cout << " " << l << "\n";
        printLine('=');
        std::cout << " (Press Enter to return)";
        std::cin.get();
    }

    // ==========================================================
    //  交互与输入解析
    // ==========================================================

    Action GameView::promptHumanAction(const GameModel& model, GameState state) {
        Action act;
        act.type = static_cast<ActionType>(-1);

        while (true) {
            bool isDraft = (model.currentAge == 0);
            if (isDraft) renderDraftPhase(model);
            else renderGame(model);

            std::cout << "\n " << model.getCurrentPlayer()->name << " > ";

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) continue;

            std::stringstream ss(line);
            std::string cmd; ss >> cmd;
            std::string arg1, arg2; ss >> arg1 >> arg2;

            // --- 1. Draft Logic ---
            if (isDraft) {
                if (cmd == "pick") {
                    int idx = parseId(arg1, ' '); // simple int
                    if (idx >= 1 && idx <= (int)ctx.draftWonderIds.size()) {
                        act.type = ActionType::DRAFT_WONDER;
                        act.targetWonderId = ctx.draftWonderIds[idx-1]; // Index 0-based
                        return act;
                    } else {
                        printError("Invalid index."); std::cin.get();
                    }
                }
                else if (cmd == "detail") { renderPlayerDetailFull(*model.getCurrentPlayer(), *model.getOpponent()); }
                else if (cmd == "info") {
                    // Support info 1
                    int idx = parseId(arg1, ' ');
                     if (idx >= 1 && idx <= (int)model.draftPool.size()) {
                         renderWonderDetail(*model.draftPool[idx-1]);
                     }
                }
                continue;
            }

            // --- 2. Main Game Logic ---
            if (cmd == "build" || cmd == "discard") {
                int id = parseId(arg1, 'C'); // Support 'C1' or '1'
                if (ctx.cardIdMap.count(id)) {
                    act.type = (cmd == "build") ? ActionType::BUILD_CARD : ActionType::DISCARD_FOR_COINS;
                    act.targetCardId = ctx.cardIdMap[id];
                    return act;
                } else {
                    printError("Invalid Card ID (Use C1, C2...). Check if card is face up.");
                    std::cout << "(Press Enter)"; std::cin.get();
                }
            }
            else if (cmd == "wonder") {
                // wonder C1 W1
                int cId = parseId(arg1, 'C');
                int wId = parseId(arg2, 'W');

                if (ctx.cardIdMap.count(cId) && ctx.wonderIdMap.count(wId)) {
                    act.type = ActionType::BUILD_WONDER;
                    act.targetCardId = ctx.cardIdMap[cId];
                    act.targetWonderId = ctx.wonderIdMap[wId];
                    return act;
                } else {
                    printError("Invalid Card or Wonder ID (e.g., 'wonder C1 W1')");
                    std::cout << "(Press Enter)"; std::cin.get();
                }
            }
            else if (cmd == "info") {
                // Try C<id>, W<id>, S<id>
                int cId = parseId(arg1, 'C');
                int wId = parseId(arg1, 'W');
                int sId = parseId(arg1, 'S');

                if (cId != -1 && ctx.cardIdMap.count(cId)) {
                    // Need to find the Card object. Map stores ID string.
                    // This is slow but safe for UI
                    for(const auto& c : model.allCards) {
                        if (c.id == ctx.cardIdMap[cId]) { renderCardDetail(c); break; }
                    }
                }
                else if (wId != -1 && ctx.wonderIdMap.count(wId)) {
                    for(const auto& w : model.allWonders) {
                         if (w.id == ctx.wonderIdMap[wId]) { renderWonderDetail(w); break; }
                    }
                }
                else if (sId != -1 && ctx.tokenIdMap.count(sId)) {
                    renderTokenDetail(ctx.tokenIdMap[sId]);
                }
                else {
                    printError("ID not found or not visible."); std::cin.get();
                }
            }
            else if (cmd == "pile") renderDiscardPile(model.board->discardPile);
            else if (cmd == "log") renderFullLog(model.gameLog);
            else if (cmd == "detail") {
                 if (arg1 == "2") renderPlayerDetailFull(*model.players[1], *model.players[0]);
                 else renderPlayerDetailFull(*model.players[0], *model.players[1]);
            }
        }
    }
}