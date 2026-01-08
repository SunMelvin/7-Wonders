# 七大奇迹：对决 (7 Wonders Duel) - 项目架构文档

本文档旨在帮助新成员快速理解本项目的代码架构、模块划分以及核心工作流程。

## 1. 项目概览

本项目是桌游《七大奇迹：对决 (7 Wonders Duel)》的 C++ 控制台实现。项目遵循 **MVC (Model-View-Controller)** 架构模式，并结合了 **状态模式 (State Pattern)** 和 **命令模式 (Command Pattern)** 来处理复杂的游戏逻辑和状态流转。

## 2. 模块划分

系统主要分为以下几个核心层级：

### 2.1 模型层 (Model Layer)
负责存储游戏的所有状态数据，不包含复杂的业务逻辑。
*   **GameModel (`GameController.h` 内定义)**: 聚合根，包含所有玩家、棋盘、卡牌和奇迹数据。
*   **Player (`Player.h`)**: 玩家数据，包括金币、手牌、已建奇迹、科技标记等。
*   **Board (`Board.h`)**: 游戏公共区域数据，如军事进度条、科技标记池。
*   **Card / Wonder (`Card.h`)**: 卡牌和奇迹的基础数据结构。

### 2.2 视图层 (View Layer)
负责将游戏状态渲染到控制台，并处理与用户的视觉交互。
*   **GameView (`GameView.h`)**: 核心渲染类，提供主菜单、游戏主界面、详情页等渲染方法。
*   **RenderContext (`RenderContext.h`)**: 辅助类，用于维护渲染上下文（如光标位置、当前选中项）。

### 2.3 控制层 (Controller Layer)
系统的“大脑”，负责调度游戏流程、处理输入并更新模型。
*   **GameController (`GameController.h`)**: 游戏主控制器，维护主循环，协调 Model 和 View。
*   **GameStateLogic (`GameStateLogic.h`)**: 状态模式基类。不同的游戏阶段（如轮抽、正常回合、摧毁卡牌阶段）有不同的子类实现，用于验证动作的合法性。
*   **GameCommands (`GameCommands.h`)**: 命令模式实现。将玩家的“动作”（如建造、弃牌）封装为命令对象执行，解耦意图与执行。

### 2.4 系统与逻辑层 (Systems & Logic)
封装具体的游戏规则计算。
*   **RulesEngine (`RulesEngine.h`)**: 判定规则，如是否满足建造条件、游戏是否结束（军事/科技胜利）。
*   **EffectSystem (`EffectSystem.h`)**: 效果系统。处理卡牌和奇迹被建造后产生的具体效果（如获得金币、再次行动、军事推进）。
*   **ScoringManager (`ScoringManager.h`)**: 负责游戏结束时的平民胜利分数计算。

### 2.5 基础设施 (Infrastructure)
*   **GameFactory (`GameFactory.h`)**: 工厂模式，负责从 JSON 文件加载数据并初始化游戏对象。
*   **InputManager (`InputManager.h`)**: 处理跨平台的键盘输入。
*   **Agent (`Agent.h`)**: 玩家代理接口。实现了人类玩家 (`HumanAgent`) 和 AI 玩家 (`RandomAIAgent`, `GreedyAIAgent`) 的统一接口。

## 3. 核心工作流程

### 3.1 游戏初始化
1.  `main.cpp` 创建 `GameView`, `GameController` 和 `InputManager`。
2.  用户选择游戏模式（Human vs Human, Human vs AI 等）。
3.  `GameController` 调用 `GameFactory` 读取 `data/gamedata.json`。
4.  初始化 `GameModel`，洗牌、发牌、设置初始金币。

### 3.2 游戏主循环 (Main Loop)
游戏通过 `while (game.getState() != GAME_OVER)` 进行循环，每一帧流程如下：

1.  **渲染 (Render)**: 
    *   `GameView` 根据当前 `GameModel` 和 `GameState` 绘制界面。
2.  **决策 (Decide)**: 
    *   当前行动的 `IPlayerAgent` (人类或AI) 分析局势，通过 `InputManager` 获取输入或通过算法计算，生成一个 `Action` 对象。
3.  **验证 (Validate)**: 
    *   `GameController` 将 `Action` 传递给当前的 `IGameStateLogic` 进行合法性验证（例如：钱够不够？是不是轮到你了？）。
4.  **处理 (Process)**: 
    *   如果验证通过，`GameController` 将 `Action` 转化为具体的 `IGameCommand`。
    *   执行命令，调用 `EffectSystem` 应用效果，更新 `GameModel`。
5.  **后处理 (Post-Process)**: 
    *   检查胜利条件 (`RulesEngine`)。
    *   切换玩家 (`switchPlayer`) 或处理连锁状态（如再次行动）。

## 4. 关键类图关系简述

*   **GameController** 拥有 **GameModel** (组合关系)。
*   **GameController** 依赖 **IGameStateLogic** 处理状态特定逻辑。
*   **IPlayerAgent** 依赖 **GameView** 来观察游戏，但不直接修改 Model。
*   **Card** 和 **Wonder** 是纯数据类，具体效果逻辑由 **EffectSystem** 根据其 ID 或属性解析执行。

## 5. 目录结构说明

*   `src/`: 所有 `.cpp` 源文件。
*   `include/`: 所有 `.h` 头文件。
*   `data/`: 游戏数据文件 (`gamedata.json`)。
*   `doc/`: 项目文档。
*   `cmake-build-debug/`: 编译生成目录。

---
建议新成员从 `main.cpp` 入手，追踪 `GameController::startGame` 和 `GameController::processAction` 的调用栈，以深入理解代码运行机制。
