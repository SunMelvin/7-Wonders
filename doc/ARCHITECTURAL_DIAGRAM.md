---
config:
  class:
    hideEmptyMembersBox: true
---
classDiagram
direction LR

%% --- 模块说明 (Legend) ---
class Legend {
    Entities(核心实体): Blue
    Rules(游戏逻辑): Green
    Control(流程控制): Orange
    Factory(数据构建): Purple
    View(视图交互): Red
}
%% 将图例设为透明或特殊样式
style Legend fill:#fff,stroke:#333,stroke-dasharray: 5 5

%% --- Entities (核心实体) ---
class GameModel:::Entities
class Player:::Entities
class Board:::Entities
class MilitaryTrack:::Entities
class CardPyramid:::Entities
class CardSlot:::Entities
class Card:::Entities
class Wonder:::Entities
class Action:::Entities
class ResourceCost:::Entities

%% --- Rules (游戏逻辑) ---
class RulesEngine:::Rules
class ScoringManager:::Rules
class IEffect:::Rules
class EffectFactory:::Rules
class IGuildStrategy:::Rules

%% --- Control (流程控制) ---
class GameController:::Control
class IGameActions:::Control
class IGameStateLogic:::Control
class IGameCommand:::Control
class CommandFactory:::Control
class IPlayerAgent:::Control

%% --- Factory (数据构建) ---
class IGameFactory:::Factory
class BaseGameFactory:::Factory
class CardBuilder:::Factory

%% --- View (视图交互) ---
class GameView:::View
class InputManager:::View
class RenderContext:::View
class ILogger:::View

%% --- Relationships ---

%% Entities Aggregation
GameModel "1" *-- "2" Player : contains
GameModel "1" *-- "1" Board : contains
GameModel o-- Wonder : manages
Player o-- Card : owns
Player o-- Wonder : owns
Board *-- MilitaryTrack : contains
Board *-- CardPyramid : contains
CardPyramid *-- CardSlot : contains
CardSlot o-- Card : holds
Card *-- ResourceCost : has
Wonder *-- ResourceCost : has

%% Factory
BaseGameFactory --|> IGameFactory : implements
BaseGameFactory ..> CardBuilder : uses
BaseGameFactory ..> Card : creates
BaseGameFactory ..> Wonder : creates

%% Control
GameController --|> IGameActions : implements
GameController --|> ILogger : implements
GameController o-- GameModel : owns
GameController o-- IGameStateLogic : current state
GameController ..> Action : consumes
IGameStateLogic ..> GameController : validates/transitions

IPlayerAgent ..> GameController : observes
IPlayerAgent ..> Action : produces
IPlayerAgent ..> GameView : uses
IPlayerAgent ..> InputManager : uses

InputManager ..> Action : produces
InputManager o-- RenderContext : uses
InputManager ..> GameView : uses

CommandFactory ..> Action : consumes
CommandFactory ..> IGameCommand : creates
IGameCommand ..> GameController : executes

%% Rules & Effects
IEffect ..> IGameActions : modifies state via
IEffect ..> ILogger : logs via
EffectFactory ..> IEffect : creates
GameController ..> ScoringManager : uses
GameController ..> RulesEngine : uses
ScoringManager ..> Player : reads
ScoringManager ..> Board : reads
RulesEngine ..> Player : reads
RulesEngine ..> Board : reads
IGuildStrategy ..> Player : reads

%% View
GameView ..> RenderContext : uses
GameView ..> GameModel : reads

%% --- Styling Definitions ---
classDef Entities fill:#e3f2fd,stroke:#1565c0,stroke-width:1px;
classDef Rules    fill:#e8f5e9,stroke:#2e7d32,stroke-width:1px;
classDef Control  fill:#fff3e0,stroke:#ef6c00,stroke-width:1px;
classDef Factory  fill:#f3e5f5,stroke:#7b1fa2,stroke-width:1px;
classDef View     fill:#ffebee,stroke:#c62828,stroke-width:1px;
