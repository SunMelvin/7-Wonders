#include "Agent.h"
#include "GameController.h"
#include "GameView.h"
#include "InputManager.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

namespace SevenWondersDuel {

    // 辅助：获取随机数引擎
    std::mt19937& getRNG() {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        return rng;
    }

    // ==========================================================
    //  IPlayerAgent
    // ==========================================================

    bool IPlayerAgent::isHuman() const { return false; }

    // ==========================================================
    //  Human Agent
    // ==========================================================

    Action HumanAgent::decideAction(GameController& game, GameView& view, InputManager& input) {
        return input.promptHumanAction(view, game.getModel(), game.getState());
    }

    bool HumanAgent::isHuman() const { return true; }

    // ==========================================================
    //  Random AI Agent (Robust & Validated)
    // ==========================================================

    Action RandomAIAgent::decideAction(GameController& game, GameView& view, InputManager& input) {
        // 1. 提示 AI 正在思考
        std::cout << "\033[1;35m[AI] 正在思考...\033[0m" << std::endl;

        // 2. 模拟思考时间 (1.5秒)
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        const GameModel& model = game.getModel();
        GameState state = game.getState();
        Action action;
        action.type = static_cast<ActionType>(-1); // Init invalid

        auto& rng = getRNG();

        // ------------------------------------------------------
        // 1. 奇迹轮抽阶段
        // ------------------------------------------------------
        if (state == GameState::WONDER_DRAFT_PHASE_1 || state == GameState::WONDER_DRAFT_PHASE_2) {
            if (!model.getDraftPool().empty()) {
                std::uniform_int_distribution<int> dist(0, model.getDraftPool().size() - 1);
                action.type = ActionType::DRAFT_WONDER;
                Wonder* selectedWonder = model.getDraftPool()[dist(rng)];
                action.targetWonderId = selectedWonder->getId();

                std::cout << "\033[1;35m[AI] 决定拿取奇迹: " << selectedWonder->getName() << "\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                return action;
            }
        }

        // ------------------------------------------------------
        // 2. 特殊交互状态 (Interrupts)
        // ------------------------------------------------------

        // A. 科技标记 (配对)
        else if (state == GameState::WAITING_FOR_TOKEN_SELECTION_PAIR) {
            const auto& tokens = model.getBoard()->getAvailableProgressTokens();
            if (!tokens.empty()) {
                std::uniform_int_distribution<int> dist(0, tokens.size() - 1);
                action.type = ActionType::SELECT_PROGRESS_TOKEN;
                action.selectedToken = tokens[dist(rng)];

                std::cout << "\033[1;35m[AI] 获得科技配对奖励，选择标记...\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                return action;
            }
        }

        // B. 科技标记 (图书馆盒子)
        else if (state == GameState::WAITING_FOR_TOKEN_SELECTION_LIB) {
            const auto& tokens = model.getBoard()->getBoxProgressTokens();
            if (!tokens.empty()) {
                std::uniform_int_distribution<int> dist(0, tokens.size() - 1);
                action.type = ActionType::SELECT_PROGRESS_TOKEN;
                action.selectedToken = tokens[dist(rng)];

                std::cout << "\033[1;35m[AI] 触发图书馆效果，从盒子中选择标记...\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                return action;
            }
        }

        // C. 摧毁对手卡牌
        else if (state == GameState::WAITING_FOR_DESTRUCTION) {
            const Player* opp = model.getOpponent();
            std::vector<Card*> candidates = opp->getBuiltCards();
            std::shuffle(candidates.begin(), candidates.end(), rng);

            // 1. 尝试摧毁
            for (auto c : candidates) {
                Action tryDestruct;
                tryDestruct.type = ActionType::SELECT_DESTRUCTION;
                tryDestruct.targetCardId = c->getId();

                if (game.validateAction(tryDestruct).isValid) {
                    std::cout << "\033[1;35m[AI] 决定摧毁对手的卡牌: " << c->getName() << "\033[0m\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                    return tryDestruct;
                }
            }

            // 2. 放弃
            Action skipAction;
            skipAction.type = ActionType::SELECT_DESTRUCTION;
            skipAction.targetCardId = "";
            if (game.validateAction(skipAction).isValid) {
                std::cout << "\033[1;35m[AI] 没有合适的目标，选择跳过摧毁。\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                return skipAction;
            }

            return action;
        }

        // D. 陵墓复活
        else if (state == GameState::WAITING_FOR_DISCARD_BUILD) {
            const auto& pile = model.getBoard()->getDiscardPile();
            if (!pile.empty()) {
                std::vector<Card*> candidates = pile;
                std::shuffle(candidates.begin(), candidates.end(), rng);

                for (auto c : candidates) {
                    Action tryResurrect;
                    tryResurrect.type = ActionType::SELECT_FROM_DISCARD;
                    tryResurrect.targetCardId = c->getId();
                    if (game.validateAction(tryResurrect).isValid) {
                        std::cout << "\033[1;35m[AI] 决定从弃牌堆复活: " << c->getName() << "\033[0m\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                        return tryResurrect;
                    }
                }
            }
            return action;
        }

        // E. 选择先手
        else if (state == GameState::WAITING_FOR_START_PLAYER_SELECTION) {
            std::uniform_int_distribution<int> dist(0, 1);
            action.type = ActionType::CHOOSE_STARTING_PLAYER;
            bool chooseMe = (dist(rng) == 0);
            action.targetCardId = chooseMe ? "ME" : "OPPONENT";

            std::cout << "\033[1;35m[AI] 决定下个时代 " << (chooseMe ? "自己" : "对手") << " 先手。\033[0m\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
            return action;
        }

        // ------------------------------------------------------
        // 3. 主游戏阶段 (Age Play)
        // ------------------------------------------------------
        else if (state == GameState::AGE_PLAY_PHASE) {
            const Player* me = model.getCurrentPlayer();

            // 获取所有可用的金字塔卡牌
            const CardPyramid& pyramid = model.getBoard()->getCardStructure();
            std::vector<const CardSlot*> validSlots;

            for(const auto& slot : pyramid) {
                if(slot.getCardPtr() && slot.isFaceUp()) validSlots.push_back(&slot);
            }

            if (validSlots.empty()) return action;

            std::shuffle(validSlots.begin(), validSlots.end(), rng);

            // --- 策略 A: 尝试建造奇迹 (20% 概率) ---
            std::uniform_int_distribution<int> dice(1, 100);
            if (dice(rng) <= 20 && !me->getUnbuiltWonders().empty()) {
                for (auto w : me->getUnbuiltWonders()) {
                    for(auto slot : validSlots) {
                        Action tryWonder;
                        tryWonder.type = ActionType::BUILD_WONDER;
                        tryWonder.targetCardId = slot->getCardPtr()->getId();
                        tryWonder.targetWonderId = w->getId();

                        if (game.validateAction(tryWonder).isValid) {
                            std::cout << "\033[1;35m[AI] 决定建造奇迹: " << w->getName() << " (使用卡牌: " << slot->getCardPtr()->getName() << ")\033[0m\n";
                            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                            return tryWonder;
                        }
                    }
                }
            }

            // --- 策略 B: 尝试建造卡牌 ---
            for (auto slot : validSlots) {
                Action tryBuild;
                tryBuild.type = ActionType::BUILD_CARD;
                tryBuild.targetCardId = slot->getCardPtr()->getId();

                if (game.validateAction(tryBuild).isValid) {
                    std::cout << "\033[1;35m[AI] 决定建造卡牌: " << slot->getCardPtr()->getName() << "\033[0m\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
                    return tryBuild;
                }
            }

            // --- 策略 C: 弃牌换钱 (Fallback) ---
            action.type = ActionType::DISCARD_FOR_COINS;
            action.targetCardId = validSlots[0]->getCardPtr()->getId();
            std::cout << "\033[1;35m[AI] 资源不足，决定弃掉卡牌换钱: " << validSlots[0]->getCardPtr()->getName() << "\033[0m\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 决策后暂停
            return action;
        }

        return action;
    }

    // ==========================================================
    //  Greedy AI Agent (优先购买高分蓝卡)
    // ==========================================================

    Action GreedyAIAgent::decideAction(GameController& game, GameView& view, InputManager& input) {
        // 1. 提示 AI 正在思考
        std::cout << "\033[1;36m[GreedyAI] 正在思考...\033[0m" << std::endl;

        // 2. 模拟思考时间 (1秒)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        const GameModel& model = game.getModel();
        GameState state = game.getState();
        Action action;
        action.type = static_cast<ActionType>(-1); // Init invalid

        auto& rng = getRNG();

        // ------------------------------------------------------
        // 1. 奇迹轮抽阶段：选择分数最高的奇迹
        // ------------------------------------------------------
        if (state == GameState::WONDER_DRAFT_PHASE_1 || state == GameState::WONDER_DRAFT_PHASE_2) {
            if (!model.getDraftPool().empty()) {
                Wonder* bestWonder = nullptr;
                int bestVP = -1;

                for (auto w : model.getDraftPool()) {
                    int vp = w->getVictoryPoints(model.getCurrentPlayer(), model.getOpponent());
                    if (vp > bestVP) {
                        bestVP = vp;
                        bestWonder = w;
                    }
                }

                if (bestWonder) {
                    action.type = ActionType::DRAFT_WONDER;
                    action.targetWonderId = bestWonder->getId();
                    std::cout << "\033[1;36m[GreedyAI] 选择高分奇迹: " << bestWonder->getName()
                              << " (VP: " << bestVP << ")\033[0m\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    return action;
                }
            }
        }

        // ------------------------------------------------------
        // 2. 特殊交互状态 (复用随机逻辑)
        // ------------------------------------------------------

        // A. 科技标记 (配对) - 随机选择
        else if (state == GameState::WAITING_FOR_TOKEN_SELECTION_PAIR) {
            const auto& tokens = model.getBoard()->getAvailableProgressTokens();
            if (!tokens.empty()) {
                std::uniform_int_distribution<int> dist(0, tokens.size() - 1);
                action.type = ActionType::SELECT_PROGRESS_TOKEN;
                action.selectedToken = tokens[dist(rng)];

                std::cout << "\033[1;36m[GreedyAI] 获得科技配对奖励，选择标记...\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                return action;
            }
        }

        // B. 科技标记 (图书馆盒子) - 随机选择
        else if (state == GameState::WAITING_FOR_TOKEN_SELECTION_LIB) {
            const auto& tokens = model.getBoard()->getBoxProgressTokens();
            if (!tokens.empty()) {
                std::uniform_int_distribution<int> dist(0, tokens.size() - 1);
                action.type = ActionType::SELECT_PROGRESS_TOKEN;
                action.selectedToken = tokens[dist(rng)];

                std::cout << "\033[1;36m[GreedyAI] 触发图书馆效果，从盒子中选择标记...\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                return action;
            }
        }

        // C. 摧毁对手卡牌 - 优先摧毁对手高分蓝卡
        else if (state == GameState::WAITING_FOR_DESTRUCTION) {
            const Player* opp = model.getOpponent();
            std::vector<Card*> candidates = opp->getBuiltCards();

            // 按分数降序排序，优先摧毁高分卡
            std::sort(candidates.begin(), candidates.end(), [&](Card* a, Card* b) {
                return a->getVictoryPoints(opp, model.getCurrentPlayer())
                     > b->getVictoryPoints(opp, model.getCurrentPlayer());
            });

            for (auto c : candidates) {
                Action tryDestruct;
                tryDestruct.type = ActionType::SELECT_DESTRUCTION;
                tryDestruct.targetCardId = c->getId();

                if (game.validateAction(tryDestruct).isValid) {
                    std::cout << "\033[1;36m[GreedyAI] 决定摧毁对手的高分卡牌: " << c->getName() << "\033[0m\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    return tryDestruct;
                }
            }

            // 没有合法目标，跳过
            Action skipAction;
            skipAction.type = ActionType::SELECT_DESTRUCTION;
            skipAction.targetCardId = "";
            if (game.validateAction(skipAction).isValid) {
                std::cout << "\033[1;36m[GreedyAI] 没有合适的目标，选择跳过摧毁。\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                return skipAction;
            }

            return action;
        }

        // D. 陵墓复活 - 优先复活高分蓝卡
        else if (state == GameState::WAITING_FOR_DISCARD_BUILD) {
            const auto& pile = model.getBoard()->getDiscardPile();
            if (!pile.empty()) {
                std::vector<Card*> candidates = pile;

                // 按分数降序排序，优先复活高分卡
                std::sort(candidates.begin(), candidates.end(), [&](Card* a, Card* b) {
                    return a->getVictoryPoints(model.getCurrentPlayer(), model.getOpponent())
                         > b->getVictoryPoints(model.getCurrentPlayer(), model.getOpponent());
                });

                for (auto c : candidates) {
                    Action tryResurrect;
                    tryResurrect.type = ActionType::SELECT_FROM_DISCARD;
                    tryResurrect.targetCardId = c->getId();
                    if (game.validateAction(tryResurrect).isValid) {
                        std::cout << "\033[1;36m[GreedyAI] 决定从弃牌堆复活高分卡: " << c->getName() << "\033[0m\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                        return tryResurrect;
                    }
                }
            }
            return action;
        }

        // E. 选择先手 - 总是选择自己先手
        else if (state == GameState::WAITING_FOR_START_PLAYER_SELECTION) {
            action.type = ActionType::CHOOSE_STARTING_PLAYER;
            action.targetCardId = "ME"; // 贪心策略：总是自己先手

            std::cout << "\033[1;36m[GreedyAI] 决定下个时代自己先手。\033[0m\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return action;
        }

        // ------------------------------------------------------
        // 3. 主游戏阶段 (Age Play) - 核心贪心策略
        // ------------------------------------------------------
        else if (state == GameState::AGE_PLAY_PHASE) {
            const Player* me = model.getCurrentPlayer();
            const Player* opp = model.getOpponent();

            // 获取所有可用的金字塔卡牌
            const CardPyramid& pyramid = model.getBoard()->getCardStructure();
            std::vector<const CardSlot*> validSlots;

            for (const auto& slot : pyramid) {
                if (slot.getCardPtr() && slot.isFaceUp()) {
                    validSlots.push_back(&slot);
                }
            }

            if (validSlots.empty()) return action;

            // --- 策略 A: 优先购买可负担的蓝卡，按分数排序 ---
            std::vector<std::pair<const CardSlot*, int>> blueCards;  // <slot, VP>
            std::vector<std::pair<const CardSlot*, int>> otherCards; // <slot, VP>

            for (auto slot : validSlots) {
                Card* card = slot->getCardPtr();
                Action tryBuild;
                tryBuild.type = ActionType::BUILD_CARD;
                tryBuild.targetCardId = card->getId();

                if (game.validateAction(tryBuild).isValid) {
                    int vp = card->getVictoryPoints(me, opp);
                    if (card->getType() == CardType::CIVILIAN) {
                        blueCards.push_back({slot, vp});
                    } else {
                        otherCards.push_back({slot, vp});
                    }
                }
            }

            // 按分数降序排序蓝卡
            std::sort(blueCards.begin(), blueCards.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

            // 优先选择分数最高的蓝卡
            if (!blueCards.empty()) {
                action.type = ActionType::BUILD_CARD;
                action.targetCardId = blueCards[0].first->getCardPtr()->getId();
                std::cout << "\033[1;36m[GreedyAI] 决定建造高分蓝卡: "
                          << blueCards[0].first->getCardPtr()->getName()
                          << " (VP: " << blueCards[0].second << ")\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                return action;
            }

            // --- 策略 B: 没有蓝卡，选择其他可购买卡牌中分数最高的 ---
            std::sort(otherCards.begin(), otherCards.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

            if (!otherCards.empty()) {
                action.type = ActionType::BUILD_CARD;
                action.targetCardId = otherCards[0].first->getCardPtr()->getId();
                std::cout << "\033[1;36m[GreedyAI] 决定建造卡牌: "
                          << otherCards[0].first->getCardPtr()->getName()
                          << " (VP: " << otherCards[0].second << ")\033[0m\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                return action;
            }

            // --- 策略 C: 尝试建造奇迹 ---
            if (!me->getUnbuiltWonders().empty()) {
                for (auto w : me->getUnbuiltWonders()) {
                    for (auto slot : validSlots) {
                        Action tryWonder;
                        tryWonder.type = ActionType::BUILD_WONDER;
                        tryWonder.targetCardId = slot->getCardPtr()->getId();
                        tryWonder.targetWonderId = w->getId();

                        if (game.validateAction(tryWonder).isValid) {
                            std::cout << "\033[1;36m[GreedyAI] 决定建造奇迹: " << w->getName()
                                      << " (使用卡牌: " << slot->getCardPtr()->getName() << ")\033[0m\n";
                            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                            return tryWonder;
                        }
                    }
                }
            }

            // --- 策略 D: 弃牌换钱 (Fallback) ---
            action.type = ActionType::DISCARD_FOR_COINS;
            action.targetCardId = validSlots[0]->getCardPtr()->getId();
            std::cout << "\033[1;36m[GreedyAI] 资源不足，决定弃掉卡牌换钱: "
                      << validSlots[0]->getCardPtr()->getName() << "\033[0m\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return action;
        }

        return action;
    }

}
