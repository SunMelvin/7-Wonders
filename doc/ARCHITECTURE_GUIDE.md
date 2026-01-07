# 7 Wonders Duel - 架构图解读指南

本文档旨在辅助阅读 `ARCHITECTURAL_DIAGRAM.md` 中的 UML 类图。该图展示了系统的静态结构和动态交互流程。

## 1. 颜色图例 (模块划分)

为了使图表紧凑，我们移除了边框，改用**颜色**来区分功能模块：

*   🔵 **蓝色 (核心实体 Entities)**: 保存数据的“哑”对象。
    *   **核心**: `GameModel` (总状态), `Player` (玩家), `Board` (棋盘)。
    *   **作用**: 它们是被操作的对象，通常不包含复杂的业务逻辑。
*   🟠 **橙色 (流程控制 Control)**: 系统的“大脑”和“中枢神经”。
    *   **核心**: `GameController` (主控), `IGameStateLogic` (状态机), `IPlayerAgent` (AI/玩家代理)。
    *   **作用**: 负责调度、决策和状态流转。
*   🟢 **绿色 (游戏逻辑 Rules)**: 具体的规则实现。
    *   **核心**: `IEffect` (卡牌效果), `RulesEngine` (胜利判定)。
    *   **作用**: 定义“建造这张卡会发生什么”以及“谁赢了”。
*   🔴 **红色 (视图交互 View)**: 输入与输出。
    *   **核心**: `GameView` (显示), `InputManager` (输入)。
    *   **作用**: 用户与系统的接口。
*   🟣 **紫色 (数据构建 Factory)**: 初始化。
    *   **核心**: `BaseGameFactory`。
    *   **作用**: 负责游戏开始时的数据加载和对象创建。

## 2. 关系符号解读

*   **实线 + 空心三角形 (`--|>`)**: **继承/实现** (Inheritance)。
    *   *例*: `GameController --|> IGameActions` (控制器实现了动作接口)。
*   **实线 + 菱形 (`*--`)**: **组合** (Composition)。强拥有关系，父亡子亡。
    *   *例*: `GameModel *-- Board` (模型销毁了，棋盘也就没了)。
*   **实线 + 空心菱形 (`o--`)**: **聚合** (Aggregation)。弱拥有关系，子对象可独立存在或被共享。
    *   *例*: `Player o-- Card` (玩家持有卡牌，但卡牌本身是独立的数据实体)。
*   **虚线 + 箭头 (`..>`)**: **依赖** (Dependency)。“使用”关系，通常作为参数传递。
    *   *例*: `GameView ..> GameModel` (视图需要读取模型来绘图)。

## 3. 系统流程导读 (Flow)

请配合 UML 图，按以下顺序追踪一回合的执行流程：

### 阶段 1: 观察与决策 (View & Control)
1.  **观察**: `IPlayerAgent` (橙色) 依赖 `GameView` (红色) 来获取当前游戏信息。
2.  **决策**: `IPlayerAgent` 也是通过 `InputManager` (红色) 获取用户输入（如果是人类）或自行计算（如果是 AI）。
3.  **产出**: 最终，`IPlayerAgent` 生成一个 `Action` (蓝色) 对象。

### 阶段 2: 命令处理 (Control)
4.  **接收**: `GameController` (橙色) 接收这个 `Action`。
5.  **封装**: `CommandFactory` (橙色) 将 `Action` 转换为具体的 `IGameCommand` (橙色) 对象（例如“建造卡牌命令”）。
6.  **验证**: `GameController` 调用当前的 `IGameStateLogic` (橙色) 来验证该动作在当前阶段是否合法。

### 阶段 3: 执行与规则 (Rules & Entities)
7.  **执行**: 命令执行时，会修改 `GameModel` (蓝色) 中的数据（扣钱、拿牌）。
8.  **效果**: 如果涉及卡牌效果，`EffectFactory` (绿色) 创建的 `IEffect` (绿色) 会被触发，通过 `IGameActions` 接口回调控制器，进而修改 `Board` 或 `Player`。
9.  **判定**: 动作结束后，`GameController` 调用 `RulesEngine` (绿色) 检查是否有“军事压制”或“科技压制”发生。

### 阶段 4: 反馈 (View)
10. **渲染**: 循环回到 `GameView` (红色)，它读取最新的 `GameModel` (蓝色) 并刷新屏幕。

---

**总结**: 数据流向通常是 `View` -> `Agent` -> `Controller` -> `Model` -> `View`，形成一个闭环。
