#ifndef SEVEN_WONDERS_DUEL_GLOBAL_H
#define SEVEN_WONDERS_DUEL_GLOBAL_H

#include <vector>
#include <string>
#include <map>

namespace SevenWondersDuel {

    /**
     * @brief 控制台输出颜色枚举
     * 用于渲染层区分不同类型的卡牌、资源和提示信息。
     */
    enum class ConsoleColor {
        DEFAULT, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, GREY, BROWN
    };

    /**
     * @brief 游戏中的基础资源类型
     * WOOD, STONE, CLAY 为原材料（棕卡提供）
     * PAPER, GLASS 为加工品（灰卡提供）
     */
    enum class ResourceType {
        WOOD, STONE, CLAY,
        PAPER, GLASS
    };

    /**
     * @brief 卡牌类型
     * 决定了卡牌的颜色、功能以及计分方式。
     */
    enum class CardType {
        RAW_MATERIAL,   // 棕色：生产原材料
        MANUFACTURED,   // 灰色：生产加工品
        CIVILIAN,       // 蓝色：提供胜利点数 (VP)
        SCIENTIFIC,     // 绿色：提供科技符号
        COMMERCIAL,     // 黄色：提供金币、交易优惠或改变资源规则
        MILITARY,       // 红色：增加军事力量 (盾牌)
        GUILD,          // 紫色：游戏结束时根据特定条件计分
        WONDER          // 奇迹 (特殊类型，用于统一处理)
    };

    /**
     * @brief 科技符号枚举
     * 用于判定科技胜利 (集齐6种不同符号) 和配对奖励 (集齐2个相同符号)。
     */
    enum class ScienceSymbol {
        NONE,
        GLOBE,    // 地球仪
        TABLET,   // 石板
        MORTAR,   // 研钵
        COMPASS,  // 罗盘
        WHEEL,    // 轮子
        QUILL,    // 羽毛笔
        LAW       // 法律 (由进度标记提供)
    };

    /**
     * @brief 游戏状态机状态枚举
     * 定义了游戏主循环中的各个阶段，用于 GameStateLogic 切换逻辑。
     */
    enum class GameState {
        WONDER_DRAFT_PHASE_1,                // 游戏开始时的奇迹轮抽阶段 (第一轮 4张)
        WONDER_DRAFT_PHASE_2,                // 游戏开始时的奇迹轮抽阶段 (第二轮 4张)
        AGE_PLAY_PHASE,                      // 主要游戏阶段：从金字塔拿牌
        WAITING_FOR_TOKEN_SELECTION_PAIR,    // 等待玩家选择科技标记 (因为凑齐了一对科技符号)
        WAITING_FOR_TOKEN_SELECTION_LIB,     // 等待玩家选择科技标记 (因为建造了图书馆奇迹)
        WAITING_FOR_DESTRUCTION,             // 等待玩家选择要摧毁的卡牌
        WAITING_FOR_DISCARD_BUILD,           // 等待玩家从弃牌堆选择一张建造
        WAITING_FOR_START_PLAYER_SELECTION,  // 时代结束，军事优势方选择下一时代先手
        GAME_OVER                            // 游戏结束
    };

    /**
     * @brief 玩家动作类型
     * 对应 Agent 决策输出的意图。
     */
    enum class ActionType {
        DRAFT_WONDER,             // 在轮抽阶段选择奇迹
        BUILD_CARD,               // 建造当前可选的某张卡牌
        DISCARD_FOR_COINS,        // 弃掉当前可选的某张卡牌换取金币
        BUILD_WONDER,             // 埋掉当前可选的某张卡牌来建造奇迹
        SELECT_PROGRESS_TOKEN,    // 从公共区选择一个科技标记
        SELECT_DESTRUCTION,       // 选择对手的一张卡牌进行摧毁
        SELECT_FROM_DISCARD,      // 从弃牌堆选择一张卡牌建造
        CHOOSE_STARTING_PLAYER    // 选择下一时代的先手玩家
    };

    /**
     * @brief 科技进步标记
     * 提供特殊的全局被动效果或胜利点数。
     */
    enum class ProgressToken {
        NONE,
        AGRICULTURE,  // 农业：6金币，4分
        URBANISM,     // 城市规划：建造连锁卡获金币
        STRATEGY,     // 策略：新建军事建筑多1盾
        THEOLOGY,     // 神学：所有奇迹此时具备“再次行动”效果
        ECONOMY,      // 经济：对手交易费全归你
        MASONRY,      // 砌体结构：蓝卡建造少2费
        ARCHITECTURE, // 建筑学：奇迹建造少2费
        LAW,          // 法律：相当于一个额外的科技符号
        MATHEMATICS,  // 数学：每个进度标记3分
        PHILOSOPHY    // 哲学：7分
    };

    /**
     * @brief 动作描述结构体
     * 封装一次玩家决策的所有必要信息，传递给 Controller 执行。
     */
    struct Action {
        ActionType type;
        std::string targetCardId;         // 目标卡牌ID (用于 Build/Discard/Wonder)
        std::string targetWonderId;       // 目标奇迹ID (用于 BuildWonder/Draft)
        ProgressToken selectedToken = ProgressToken::NONE; // 选择的科技标记
        ResourceType chosenResource = ResourceType::WOOD;  // (备用) 某些特殊效果选择资源
    };

    /**
     * @brief 动作验证结果
     * 包含验证是否通过，以及具体的消耗或错误信息。
     */
    struct ActionResult {
        bool isValid;
        int cost = 0;        // 执行该动作需要的总金币 (含交易费)
        std::string message; // 错误信息或成功提示
    };

    /**
     * @brief 胜利类型
     */
    enum class VictoryType { NONE, MILITARY, SCIENCE, CIVILIAN };

    /**
     * @brief 游戏配置常量
     * 集中管理游戏的数值平衡参数。
     */
    namespace Config {
        static constexpr int INITIAL_COINS = 7;           // 初始金币
        static constexpr int COINS_PER_VP = 3;            // 游戏结束时每3金币换1分
        static constexpr int BASE_DISCARD_GAIN = 2;       // 弃牌基础获得金币 (2 + 黄卡数)
        
        static constexpr int MASONRY_DISCOUNT = 2;        // 砌体结构减免
        static constexpr int ARCHITECTURE_DISCOUNT = 2;   // 建筑学减免
        
        static constexpr int URBANISM_CHAIN_BONUS = 4;    // 城市规划连锁奖励金币
        static constexpr int URBANISM_TOKEN_BONUS = 6;    // 城市规划立即获得金币
        
        static constexpr int AGRICULTURE_VP = 4;
        static constexpr int PHILOSOPHY_VP = 7;
        static constexpr int MATHEMATICS_VP_PER_TOKEN = 3;

        // 军事轨道阈值与奖励
        static constexpr int MILITARY_THRESHOLD_LOOT_1 = 3; // 掠夺2金币的阈值
        static constexpr int MILITARY_THRESHOLD_LOOT_2 = 6; // 掠夺5金币的阈值
        static constexpr int MILITARY_THRESHOLD_WIN = 9;    // 直接获胜阈值
        static constexpr int MILITARY_LOOT_VALUE_1 = 2;
        static constexpr int MILITARY_LOOT_VALUE_2 = 5;
        static constexpr int MILITARY_VP_LEVEL_1 = 2;       // 第1区间分数
        static constexpr int MILITARY_VP_LEVEL_2 = 5;       // 第2区间分数
        static constexpr int MILITARY_VP_WIN = 10;          // 压倒性胜利分数 (虽然直接赢了，但逻辑上保留)

        static constexpr int SCIENCE_WIN_THRESHOLD = 6;     // 科技胜利需要的不同符号数
        static constexpr int SCIENCE_PAIR_COUNT = 2;        // 触发配对奖励需要的相同符号数

        static constexpr int TRADING_BASE_COST = 2;         // 基础交易费
        static constexpr int MAX_TOTAL_WONDERS = 7;         // 也就是一旦建成第7个，第8个立即废弃
    }

    // 字符串转换辅助函数
    ResourceType strToResource(const std::string& s);
    CardType strToCardType(const std::string& s);
    ScienceSymbol strToScienceSymbol(const std::string& s);
    std::string resourceToString(ResourceType r);
}

#endif // SEVEN_WONDERS_DUEL_GLOBAL_H