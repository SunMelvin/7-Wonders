#ifndef SEVEN_WONDERS_DUEL_EFFECTSYSTEM_H
#define SEVEN_WONDERS_DUEL_EFFECTSYSTEM_H

#include "Global.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace SevenWondersDuel {

    // 前向声明，避免循环引用
    class Player;
    class Board;

    /**
     * @brief 日志接口 (Interface Segregation)
     * 让 EffectSystem 能够记录日志，而不需要依赖完整的 Controller。
     */
    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void addLog(const std::string& msg) = 0;
    };

    /**
     * @brief 游戏动作接口 (Interface Segregation)
     * 让 EffectSystem 能够回调核心游戏逻辑 (如状态切换、移动军事)，而不需要依赖完整的 Controller。
     */
    class IGameActions {
    public:
        virtual ~IGameActions() = default;
        virtual void setState(GameState newState) = 0;
        virtual void setPendingDestructionType(CardType t) = 0;
        virtual void grantExtraTurn() = 0;

        // 抽象化的棋盘操作
        virtual std::vector<int> moveMilitary(int shields, int playerId) = 0;
        virtual bool isDiscardPileEmpty() const = 0;
    };

    /**
     * @brief 效果基类 (Command Pattern)
     * 代表卡牌或奇迹被建造后产生的具体影响。
     */
    class IEffect {
    public:
        virtual ~IEffect() = default;

        /**
         * @brief 应用效果
         * 当卡牌/奇迹被建造时调用。
         * @param self 建造者
         * @param opponent 对手
         * @param logger 日志记录器
         * @param actions 游戏动作回调接口
         */
        virtual void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) = 0;

        /**
         * @brief 计算该效果提供的胜利点数
         * 在游戏结束时调用。
         */
        virtual int calculateScore(const Player* self, const Player* opponent) const { return 0; }

        /**
         * @brief 获取效果描述文本
         * 用于 UI 显示。
         */
        virtual std::string getDescription() const = 0;
    };

    /**
     * @brief 1. 资源产出效果 (棕/灰/黄/奇迹)
     */
    class ProductionEffect : public IEffect {
    private:
        std::map<ResourceType, int> producedResources;
        bool isChoice;   // true=多选一 (黄卡/奇迹), false=固定产出 (棕/灰)
        bool isTradable; // true=对手可见产量 (棕/灰), false=私有产量 (黄/奇迹)

    public:
        ProductionEffect(std::map<ResourceType, int> res, bool choice = false, bool tradable = false)
            : producedResources(res), isChoice(choice), isTradable(tradable) {}

        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 2. 军事效果 (红卡/奇迹)
     */
    class MilitaryEffect : public IEffect {
    private:
        int shields;
        bool isFromCard; // 标记来源：是否为红卡 (受 Strategy 科技标记影响)

    public:
        explicit MilitaryEffect(int count, bool fromCard = false)
            : shields(count), isFromCard(fromCard) {}

        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 3. 科技效果 (绿卡/法律)
     */
    class ScienceEffect : public IEffect {
    private:
        ScienceSymbol symbol;

    public:
        explicit ScienceEffect(ScienceSymbol s) : symbol(s) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 4. 直接给分效果 (蓝卡/奇迹)
     */
    class VictoryPointEffect : public IEffect {
    private:
        int points;

    public:
        explicit VictoryPointEffect(int p) : points(p) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override; // 此时不做任何事
        int calculateScore(const Player* self, const Player* opponent) const override;
        std::string getDescription() const override;
    };

    /**
     * @brief 5. 立即获得金币效果
     */
    class CoinEffect : public IEffect {
    private:
        int amount;

    public:
        explicit CoinEffect(int a) : amount(a) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 6. 条件金币效果 (按卡牌类型给钱)
     * 常见于黄卡 (如 Forum: 每张黄卡1元) 和某些奇迹。
     */
    class CoinsPerTypeEffect : public IEffect {
    private:
        CardType targetType; // 统计的目标类型
        int coinsPerCard;    // 每张多少钱
        bool countWonder;    // 是否也统计奇迹数量

    public:
        CoinsPerTypeEffect(CardType type, int amount, bool wonder = false)
            : targetType(type), coinsPerCard(amount), countWonder(wonder) {}

        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 7. 交易优惠效果 (黄卡)
     * 将某类资源的购买价格固定为 1。
     */
    class TradeDiscountEffect : public IEffect {
    private:
        ResourceType resource;

    public:
        explicit TradeDiscountEffect(ResourceType r) : resource(r) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 8. 摧毁卡牌效果 (Zeus / Circus Maximus)
     */
    class DestroyCardEffect : public IEffect {
    private:
        CardType targetColor; // 只能摧毁特定颜色的卡

    public:
        explicit DestroyCardEffect(CardType color) : targetColor(color) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 9. 再次行动效果 (奇迹)
     */
    class ExtraTurnEffect : public IEffect {
    public:
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override { return "Take another turn immediately."; }
    };

    /**
     * @brief 10. 从弃牌堆建造 (Mausoleum 奇迹)
     */
    class BuildFromDiscardEffect : public IEffect {
    public:
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override { return "Build a card from discard pile for free."; }
    };

    /**
     * @brief 11. 发展标记选择 (Library 奇迹)
     */
    class ProgressTokenSelectEffect : public IEffect {
    public:
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override { return "Choose a progress token from the box."; }
    };

    /**
     * @brief 12. 强迫对手丢钱 (Appian Way 奇迹)
     */
    class OpponentLoseCoinsEffect : public IEffect {
    private:
        int amount;
    public:
        explicit OpponentLoseCoinsEffect(int a) : amount(a) {}
        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        std::string getDescription() const override;
    };

    /**
     * @brief 13. 行会效果 (紫卡)
     * 策略模式：具体的计分规则委托给 IGuildStrategy。
     */
    enum class GuildCriteria {
        YELLOW_CARDS,      // 双方最多的黄卡
        BROWN_GREY_CARDS,  // 双方最多的棕+灰
        WONDERS,           // 双方最多的奇迹
        BLUE_CARDS,        // 双方最多的蓝卡
        GREEN_CARDS,       // 双方最多的绿卡
        RED_CARDS,         // 双方最多的红卡
        COINS              // 双方最多的金币
    };

    // 行会计分策略接口
    class IGuildStrategy {
    public:
        virtual ~IGuildStrategy() = default;
        virtual int calculateCoins(const Player* self, const Player* opponent) const = 0;
        virtual int calculateVP(const Player* self, const Player* opponent) const = 0;
    };

    class GuildEffect : public IEffect {
    private:
        std::unique_ptr<IGuildStrategy> m_strategy;

    public:
        explicit GuildEffect(GuildCriteria c);

        void apply(Player* self, Player* opponent, ILogger* logger, IGameActions* actions) override;
        int calculateScore(const Player* self, const Player* opponent) const override;
        std::string getDescription() const override;
    };

    /**
     * @brief 效果工厂
     * 负责从 JSON 数据解析并创建对应的 IEffect 对象。
     */
    class EffectFactory {
    public:
        static std::vector<std::shared_ptr<IEffect>> createEffects(const nlohmann::json& vList, CardType sourceType, bool isFromCard);
    };

}

#endif // SEVEN_WONDERS_DUEL_EFFECTSYSTEM_H
