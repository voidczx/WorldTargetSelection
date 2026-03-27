# WorldTargetSelection

## 简介

WorldTargetSelection 是一个蓝图拓展友好的，用于在 World 中选择并辅助管理目标的插件。

![插件示意图](https://raw.githubusercontent.com/voidczx/ImageBed/refs/heads/main/plugin_overview.jpg)

**用户侧**

- **WorldTargetComponent**
  只有该类型能被该系统作为目标选择，并作为配置的目标类型（TargetTags）的载体。

- **WorldTargetSelectionComponent**
  用来选择目标、获取或移除已选择过的目标关系，并作为配置的目标选择规则（TargetRules）的载体。

**内部系统侧**

- **WorldTargetSelectionSubsystem**
  用于存储并管理目标组件实例以及目标注册关系。

## 快速开始

1. 插件目录下的 Example 文件夹里包含了一个按距离和是否带有 Actor Tag 来选择目标的示例资产集合

![示例资产集合](https://raw.githubusercontent.com/voidczx/ImageBed/refs/heads/main/example_assets.png)

2. 将示例的目标选择者（ExampleSelector）和示例的目标候补者（ExampleTargetWithoutTag 与 ExampleTargetWithTag）拖放到场景中随意排布

![场景布局](https://raw.githubusercontent.com/voidczx/ImageBed/refs/heads/main/scene_layout.png)

3. 以 StandAlone 模式运行游戏，圆锥体每秒会进行一次重新目标选择，被圆锥体选为目标的候选者将在自身前面渲染作为目标的位次（Array Index）未被选为目标的则显示自身的物体名（Object Name），可通过弹出（F8）选中并拖动物体位置来观察距离和目标次序的关系

**视频文件**: [Demo_Video.mp4](https://raw.githubusercontent.com/voidczx/ImageBed/refs/heads/main/demo_video.mp4)

4. 在示例的目标选择者（ExampleSelector）的蓝图中，在事件图里将 EventTick 里预设好的 SelectionMode 01 ~ 04 分别连接至 New Mode Name，编译蓝图后重新运行游戏，以观察在仅选择无 Tag 目标，仅选择有 Tag 目标，按从近到远的顺序排列目标，按从远到近的顺序排列目标，这四种规则下的表现区别。

![蓝图配置](https://raw.githubusercontent.com/voidczx/ImageBed/refs/heads/main/blueprint_config.png)

5. 配合示例表现及蓝图配置，对示例蓝图中的流程进行断点调试，以快速掌握 WorldTargetSelection 的使用和拓展方式。

## 配置/拓展简述

### 配置侧

**目标候选者**
候选者 Actor 需要携带 WorldTargetComponent（数量至少为 1，也可有多个），并在组件的 ConfigTags 下配置自己可作为哪种类型的目标。

**目标选择者**
选择者 Actor 需要携带 WorldTargetSelectionComponent（每 Actor 只能有一个此 Component），并在组件的 ConfigRules 下配置各种模式所选择的目标类型、选择条件、排序方式以及能得到的最多目标数目。

### 拓展侧

**目标选择条件**
创建继承于 WorldTargetSelectionConditionBase 的蓝图并实现 IsConditionPass 函数

**目标排序策略**
创建继承于 WorldTargetSelectionSortProxyBase 的蓝图并实现 SortFunction 函数