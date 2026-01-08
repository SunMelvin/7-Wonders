#ifndef SEVEN_WONDERS_DUEL_GAMECOMMANDS_H
#define SEVEN_WONDERS_DUEL_GAMECOMMANDS_H

#include "Global.h"
#include <memory>

namespace SevenWondersDuel {

    class GameController;

    /**
     * @brief 游戏命令接口 (Command Pattern)
     * 将“玩家的意图”封装为对象，解耦请求者(Agent)和执行者(Controller)。
     * 每个命令负责修改 Model 的状态，并产生副作用 (如写日志)。
     */
    class IGameCommand {
    public:
        virtual ~IGameCommand() = default;
        
        /**
         * @brief 执行命令
         * 修改游戏状态的核心逻辑。
         */
        virtual void execute(GameController& controller) = 0;
    };

    /**
     * @brief 命令工厂
     * 根据 ActionType 创建对应的 ConcreteCommand 对象。
     */
    class CommandFactory {
    public:
        static std::unique_ptr<IGameCommand> createCommand(const Action& action);
    };

    // --- Concrete Commands (具体命令) ---

    /**
     * @brief 奇迹轮抽命令
     * 玩家在游戏开始阶段选择奇迹。
     */
    class DraftWonderCommand : public IGameCommand {
        std::string wonderId;
    public:
        explicit DraftWonderCommand(std::string id);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 建造卡牌命令
     * 支付费用，将卡牌从金字塔移入玩家区域，触发效果。
     */
    class BuildCardCommand : public IGameCommand {
        std::string cardId;
    public:
        explicit BuildCardCommand(std::string id);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 弃牌换钱命令
     * 将卡牌移入弃牌堆，玩家获得金币 (2 + 黄卡数)。
     */
    class DiscardCardCommand : public IGameCommand {
        std::string cardId;
    public:
        explicit DiscardCardCommand(std::string id);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 建造奇迹命令
     * 使用一张金字塔卡牌作为垫材，建造手中的奇迹。
     */
    class BuildWonderCommand : public IGameCommand {
        std::string cardId;    // 垫材 (金字塔中的卡)
        std::string wonderId;  // 目标奇迹
    public:
        BuildWonderCommand(std::string cid, std::string wid);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 选择科技标记命令
     * 从桌面或盒子中拿取一个科技标记。
     */
    class SelectProgressTokenCommand : public IGameCommand {
        ProgressToken token;
    public:
        explicit SelectProgressTokenCommand(ProgressToken t);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 摧毁卡牌命令
     * 指定对手的一张已建卡牌进行移除。
     */
    class DestructionCommand : public IGameCommand {
        std::string targetId;
    public:
        explicit DestructionCommand(std::string tid);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 弃牌堆复活命令 (陵墓)
     * 从弃牌堆选择一张卡牌免费建造。
     */
    class SelectFromDiscardCommand : public IGameCommand {
        std::string cardId;
    public:
        explicit SelectFromDiscardCommand(std::string id);
        void execute(GameController& controller) override;
    };

    /**
     * @brief 选择先手命令
     * 在时代过渡时，决定下一时代的先手玩家。
     */
    class ChooseStartingPlayerCommand : public IGameCommand {
        std::string targetId; // "ME" or "OPPONENT"
    public:
        explicit ChooseStartingPlayerCommand(std::string tid);
        void execute(GameController& controller) override;
    };

}

#endif // SEVEN_WONDERS_DUEL_GAMECOMMANDS_H