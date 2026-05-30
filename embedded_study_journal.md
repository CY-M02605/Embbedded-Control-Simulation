## 四十三、mini_control_sim rebuild 完成 + 公司代码学习计划（5/27-29）

### 43.1 mini_control_sim rebuild 完成（5/27）
- ✅ utility/increment_timer.h — Config{threshold}, Update(++count_), IsTimeUp(), Clear()
- ✅ utility/hysteresis.h — enum STATUS{ON,OFF} + switch, Config{high/low_threshold}
- ✅ modules/hydraulic_oil_warning.h/.cc — 继承 ModuleInterface, Hysteresis + IncrementTimer 组合
- ✅ main.cpp — struct CycleInput 数组 + for 循环测试，编译运行成功
- ✅ CMakeLists.txt — add_executable + target_include_directories

### 43.2 hydraulic_oil_warning 学到的要点
- **Hysteresis 是对象不是数值**：不能 `oil_temp > oil_temp_high_`，要 `oil_temp_high_.Update(value)` 再查 `GetState()`
- **Signal 赋值要用方法**：不能 `output_ = signals::ON`，要 `output_.SetValue(ON)` 或 `output_.Set(ON, VALID)`
- **public 继承**：`class X: ModuleInterface` 是 private 继承，Manager 无法通过基类指针调用，必须写 `public`
- **display_timer 位置**：只在 STATE_ON 时计时，不要无条件调 Update()
- **config 存值不存引用**：`const Config config_` 而非 `const Config& config_`，防止外部临时变量析构后悬空

### 43.3 main.cpp 测试技巧
- 用 struct 数组 + for 循环代替逐行手写，方便扩展场景
- `sizeof(arr) / sizeof(arr[0])` 算数组长度（C++11 没有 std::size）
- 打印时用变量值而非硬编码数字：`<< scenario[i].temp` 而非 `"Temp=105"`
- 测试要覆盖：正常→过高亮灯→诊断异常立即灭→诊断恢复→温度降+timer到灭灯

### 43.4 公司代码学习计划（6/1 - 8/31，13周）
**目标：8月31日前学完公司代码中8大设计模式**

| 周 | 主题 | 关键文件 |
|---|---|---|
| 6/1 | 1. C/C++ 边界适配器（TargetLink C → C++ wrapper） | wla/ 下 .c+.cc 配对 |
| 6/8 | 7. Facade 模式（原始信号→领域类型） | output_facade_model_* |
| 6/15 | 5. INSTANTIATION 宏 DI（系统组装） | instantiation_*.h |
| 6/22 | 2. SO State Converter（通信故障信号处理） | gw_rtcdbrx_so_state_converter.cc |
| 7/6 | 3. 定时器驱动状态机 | general_order_mediation_model_function.cc |
| 7/20 | 8. 生命周期状态机（6状态Profile管理） | profile_synchronize_model_function.h |
| 8/3 | 4. 模板装饰器（类型萃取+CRTP） | adjustable_mdo_correction_model_* |
| 8/10 | 6. 卡尔曼滤波器（矩阵运算+控制理论） | inclination_angle_estimation_model_* |
| 8/24 | 总复习 + 面试准备 | — |

学法：读代码 → 讲原理 → 从零重写



## 四十二、mini_control_sim 项目搭建开始（5/26）

### 42.1 项目理解
- mini_control_sim = 把嵌入式控制逻辑从 RH850 搬到 PC 上跑的模拟器
- 不需要真实硬件，用假数据注入替代传感器，打印替代实际输出
- 展示能力：循环控制框架、信号对象、迟滞判断、Timer确认、Template Method 继承、异常安全回退

### 42.2 搭建进度（5/26）
- ✅ CMakeLists.txt（cmake_minimum_required + project + CXX_STANDARD 11）
- ✅ framework/module_interface.h（纯虚基类，virtual ~析构 + virtual Update()=0）
- ✅ framework/manager.h（vector<ModuleInterface*>，RegisterModule + UpdateAll）
- ✅ signals/signal.h（模板类 Signal<T>，值+有效性，typedef 常用类型）
- 🔲 utility/increment_timer.h
- 🔲 utility/hysteresis.h
- 🔲 modules/hydraulic_oil_warning.h/.cc
- 🔲 main.cpp + 编译运行
- 项目路径：C:\MyPractice\MyEmbeddedPractice\min_control_sim_rebuild\

### 42.3 学到的设计概念
- **多态**：Manager 不认识具体模块类型，通过 ModuleInterface* 统一调用 Update()
- **依赖注入**：模块不自己创建信号，由外部传入引用 → 可替换输入源（真传感器 or 假数据）
- **模板类**：Signal<T> 把类型当参数，一个类定义覆盖所有信号类型（float/bool/enum）
- **虚函数必要性**：没有虚函数 → Manager 必须认识每种模块（紧耦合）；有虚函数 → 只认识接口（松耦合）

### 42.4 常见错误
- `LANGUAGE` vs `LANGUAGES`（CMake 关键字要加 s）
- class 默认是 private，函数/成员要放在 `public:` 下
- 函数必须声明返回类型（`void`）
- 虚析构函数要有函数体 `{}`，纯虚函数用 `= 0`


## 四十一、練習模块 #30-31：diagnosis modules（5/26）

### 41.1 voltage_range_fault_detector（#30，继承基类）
- 继承 `fault_state_detection_model::Function`，只需覆盖 `IsFaultCondition()` 和 `IsRecoverCondition()`
- 两个函数只返回 bool（纯条件判断），Timer/Mask/状态机全由基类 `FailureDetector` 管理
- Config 只需 `super`（含 fault_detector_config）+ 自定义阈值（low/high_voltage_threshold）
- 不需要自己添加 IncrementTimer、BoolSignal mask、enum STATUS — 基类已包含

### 41.2 基类继承链深入理解
- `fault_state_detection_model::Function` → 内部持有 `abnormality::FailureDetector`（即 `AbnormalityDetector` 特化）
- `AbnormalityDetector::Update(bool current_flag)`：Timer确认 → Mask判断 → JudgeAbnormalityState → 输出 FailureSignal
- `current_flag`（实时条件）vs `fault_value`（Timer确认后的值）→ 决定 4 种状态：NO/CHECK/OCCUR/RECOVER
- `FaultTransitionMaskOrder::ON` 时：用 `fault_value` 替代 `current_flag`，冻结状态不转移
- 一个 `FaultTransitionMaskOrder` 管整个模块的屏蔽，不需要按故障类型分别加 BoolSignal mask

### 41.3 signal_timeout_fault_detector（#31，从头写）
- 直接继承 `ModuleInterface`，自己实现全部逻辑：Timer + 状态机 + Mask + FailureSignal 输出
- 场景：CAN 信号超时检测 — 信号状态变 INVALID 持续一段时间 → 判定通信故障
- 设计理由：timeout_timer（防偶尔丢帧误报）、recover_timer（防接触不良反复报消）、mask（防启动误报）
- 输出用 `FailureSignal(EnumFailureState, EnumSignalState)` 构造，不能直接赋枚举

### 41.4 常见错误总结
- `=` vs `==`：C/C++ 最经典陷阱，`if (x = value)` 是赋值（永远为 true），`==` 才是比较
- `&this` vs `*this`：`this` 已是指针，`*this` 解引用得到对象引用，`&this` 是指针的地址（类型错）
- 初始化列表用逗号 `,` 分隔，不是分号 `;`；注意中文逗号 `，` vs 英文逗号 `,`
- switch-case 进入后不会因变量改变而跳转 → 输出应放在 switch 外或转移后立即覆盖
- GetValue() 别忘调：信号对象不能直接跟枚举比较，要 `.GetValue() ==`
- `EnumFailureState::OFF` 不存在，正确值：NO/CHECK/OCCUR/RECOVER

### 41.5 switch-case 输出延迟问题
- switch 只在进入时读一次变量，之后即使变量改了也不会跳到其他 case
- if-else 每次执行都重新检查变量最新值
- 解决方案1：输出放在 switch 外面用 if-else 判断最终 status_
- 解决方案2：case 内先设默认输出，转移时立刻覆盖（用户采用的方案）

### 进度：31 个模块完成


## 【parking_lamp_model_practice 实战补充】2026-04-27

### 常见错误与解决方案对照

1. **Manager 参数必须用引用（&）**
   - 错误：Manager 用值传递会导致复制，编译报错或逻辑错误。
   - 解决：用 framework::Manager& manager，只传递引用。

2. **成员变量类型要和信号对象一致**
   - 错误：输出变量声明成枚举类型（如 EnumElectricalOnOffState::Type），实际应为信号对象类型（如 ElectricalOnOffState）。
   - 解决：声明为 signal_object::ElectricalOnOffState。

3. **命名空间拼写要一致**
   - 错误：赋值时写成 SignalObject::EnumElectricalOnOffState，应为 signal_object::ElectricalOnOffState。
   - 解决：保持命名空间和类型一致。

4. **构造函数初始化列表顺序建议与成员变量声明顺序一致**
   - 便于阅读和维护。

5. **Update/输出方法必须一一对应**
   - 错误：.h 声明了方法，.cc 必须全部实现，不能遗漏。
   - 解决：严格一一对应。

6. **定时器三步模式**
   - 正确：Update → Clear（条件不满足时）→ IsTimeUp 判断输出。
   - 常见错误：Clear 条件写错（用 && 代替 ||），导致定时器无法归零。
   - 解决：用 if (!A || !B) timer_.Clear();

7. **Config 结构体与 config_app.h 的关系**
   - 易错点：Config 只定义“形状”，具体数值在 config_app.h 填写，模块不 include config_app.h。
   - 解决：在接线文件 include config_app.h，并把 config 传给模块。

8. **嵌套结构体初始化要用 {} 包裹**
   - 如 { 40.F, {10U, 0U, 500U} }。

9. **信号对象赋值要“装包”**
   - 正确：output_ = signal_object::ElectricalOnOffState(value, VALID);
   - 常见错误：直接赋值枚举类型，导致类型不匹配。

10. **其它补充**
    - .h 只声明接口，.cc 实现所有声明的方法。
    - 所有输入信号用 const 引用保存，输出信号用对象保存。
    - Manager 注册模块用 manager.RegistModule(*this);

11. **GHS 编译器不支持 C++11 的 `enum class`（作用域枚举）**
    - 错误：用 `enum class State { OFF, WAIT, ON };`，GHS cxrh850 报 `error #110: expected either a definition or a tag name`。
    - 原因：GHS 编译器只支持 C++03 标准，`enum class` 是 C++11 特性。
    - 解决：改成普通 `enum State { STATE_OFF, STATE_WAIT, STATE_ON };`。
    - 注意：普通 enum 没有作用域隔离，枚举值要加前缀（如 `STATE_`）防止与 `signal_object` 里的 `ON`/`OFF` 冲突。
    - 引用方式也要改：`Function::State::OFF` → `STATE_OFF`（普通 enum 值直接用，不需要经过类名::枚举名::值）。
    - **通用规则**：GHS cxrh850 = C++03，不能用 `enum class`、`auto`、`nullptr`、`override`、`range-based for`、`lambda` 等 C++11 特性。

---

## 【parking_lamp_model_practice 第二次从零练习】2026-04-28

### 本次犯错总结（从零写 → 3轮修改 → 编译通过）

#### .h 文件犯的9个错

| # | 错误 | 正确 | 错误类型 |
|---|------|------|---------|
| 1 | `"signale_object.h"` 多了e | `"signal_object.h"` | 拼写 |
| 2 | Config成员加了 `const` | 不需要const，靠传参 `const Config&` 保护 | const乱加 |
| 3 | `parking_press_active_time_config` 少了r | `parking_press_active_timer_config` | 拼写 |
| 4 | 类型 `pressure` 小写，参数名 `Pressure` 大写 | 类型大写 `Pressure`，参数名小写 `pressure` | 大小写 |
| 5 | `Void Update();` 大写V | `void` — C++关键字全小写 | 大小写 |
| 6 | 输出变量类型写成HysteresisFloat32，且只有一个变量 | 需要两个：① `HysteresisFloat32 is_parking_press_active_` ② `ElectricalOnOffState parking_press_output_`，都不能const | 变量搞混 |
| 7 | timer加了const，名字少了r | 去掉const（要被Update/Clear修改），名字 `parking_press_active_timer_` | const乱加+拼写 |
| 8 | 把Manager保存为成员变量 `manager_` | Manager只在构造函数里用参数注册，不保存 | 设计理解 |
| 9 | class结尾 `}` 没加分号 | `};` — class/struct定义结尾必须有分号 | 语法 |

#### .cc 文件犯的12个错

| # | 错误 | 正确 | 错误类型 |
|---|------|------|---------|
| 1 | `Fucntion::Function` | `Function::Function` | 拼写 |
| 2 | 初始化列表缺少 is_parking_press_active_ 和 timer_ | 必须用config的子config初始化 | 遗漏 |
| 3 | `status_(Status::STATE_OFF)` 用了前缀 | `status_(STATE_OFF)` — 普通enum不需要前缀 | enum用法 |
| 4 | `manager_.RegistModule(*this)` 用成员变量 | `manager.RegistModule(*this)` 用构造函数参数 | 设计理解 |
| 5 | `void Function Update()` 缺少 `::` | `void Function::Update()` | 语法 |
| 6 | `parking_press_active_.Update()` 变量名错+缺参数 | `is_parking_press_active_.Update(pressure_.GetValue())` | 变量名+API用法 |
| 7 | 缺少定时器Update | 补上 `parking_press_active_timer_.Update()` | 遗漏 |
| 8 | 所有case用 `Status::STATE_OFF` | 直接 `STATE_OFF` — 普通enum不需要前缀 | enum用法 |
| 9 | Clear了迟滞器而不是定时器 | `parking_press_active_timer_.Clear()` | 逻辑搞混 |
| 10 | WAIT状态判断条件不完整 | 应判断 `timer_.IsTimeUp()` → ON，条件不满足 → 回OFF | 状态机逻辑 |
| 11 | output赋值分散在case里，部分case没赋值 | switch之后统一：`(status_ == STATE_ON) ? ON : OFF` | 未初始化风险 |
| 12 | ParkingPressRef()写在了Update()函数内部 | 必须作为独立函数写在Update()的 `}` 外面 | 函数嵌套 |

#### 第二轮修改还残留的错（共11个）

| 错误 | 出现次数 | 根因 |
|------|---------|------|
| 成员变量忘加下划线 `_` | 4次 | 习惯问题 |
| 变量改名没全局替换 | 3次 | 不仔细 |
| 普通enum仍加 `Status::` 前缀 | 3次 | 知识没固化 |
| **`=` 写成 `==`（赋值vs比较）** | **1次** | **致命级！** |
| 初始化列表用分号而非逗号 | 1次 | 语法不熟 |

#### 第三轮修改还残留的错（共4个）

| 错误 | 根因 |
|------|------|
| `parking_press_active_timer.Clear()` 少 `_` | 下划线习惯 |
| `.IstimeUp()` → `.IsTimeUp()` | 大小写 |
| `parking_press_active_` → `is_parking_press_active_` | 改名没全局替换 |
| `status = STATE_OFF` 少 `_` | 下划线习惯 |

### 我的高频错误模式（需要重点注意）

```
🔴 致命级：= vs ==（赋值vs比较）— 嵌入式中这个bug可以让灯永远亮着
🟡 高频：成员变量忘加下划线 _ — 每次都犯，需要写完后专门检查一遍
🟡 高频：改名后没有全局替换 — 改了声明但用的地方忘改
🟡 高频：拼写/大小写不仔细 — signale, Fucntion, Void, IstimeUp
🟢 已改善：enum class → 普通enum（昨天学的，今天基本记住了）
🟢 已改善：函数嵌套问题（ParkingPressRef写进Update内部，修了）
```

### 自查清单（写完代码后按这个检查）

```
□ 所有成员变量都有下划线 _  后缀吗？
□ 初始化列表用的是逗号 , 不是分号 ; 吗？
□ 普通enum的值直接用（STATE_OFF），没加类名前缀吗？
□ class/struct 定义结尾有分号 }; 吗？
□ void/const 等关键字全小写了吗？
□ 类型名大写（Pressure）、变量名小写（pressure）了吗？
□ .cc里方法名有 Function:: 前缀吗？
□ 所有比较用的是 == 不是 = 吗？
□ Config成员没有多余的const吗？
□ Manager没有存为成员变量吗？
□ 每个输出方法都独立在Update()外面吗？
□ 变量改名后，所有引用的地方都改了吗？
```

---

【建议】如需更多模板、常见错误对照、或有具体报错信息，随时补充！


# 嵌入式变速箱控制系统 学习笔记（面試用・機密除外版）

> ⚠️ **注意**：本笔记已脱敏处理，移除了客户名称、项目代号、具体文件路径和内部框架专有名称。
> 面试时说 "a major construction equipment manufacturer" 即可，不要点名客户公司。
> 北欧面试官完全理解 NDA 约束，保持机密反而体现专业性和诚信。

---

## 一、项目概述

- **项目类型**：大型建设机械制造商 变速箱(Transmission)控制系统
- **类型**：嵌入式 C++ 实时控制软件
- **芯片**：Renesas RH850
- **编译器**：Green Hills (GHS)
- **不能在 PC 上运行**，必须编译后烧写到硬件（.mot 文件）
- **安全关键系统**：控制建设机械的变速箱，涉及行驶安全

---

## 二、四层软件架构

```
[project_root]/
├── OS/[version]/          → 操作系统层（底层驱动和调度）
├── utility/[version]/     → 工具类库（IncrementTimer等通用工具）
│   └── timer/
│       ├── increment_timer.h    ← IncrementTimer 的声明
│       └── increment_timer.cc   ← IncrementTimer 的实现
├── Middleware/[version]/  → 中间件层（框架类：Manager、ModuleInterface等）
├── Application/           → 应用组件版本库（各功能模块的发布版本）
└── user_workspace/        → 开发者工作区（主战场，日常开发在这里）
```

**层次关系**：下层提供服务给上层，上层不要去改下层。
**注意**：utility/ 是独立的共享库，和 Middleware 平级，不在 user_workspace 内部。

---

## 三、开发者工作区目录结构详解

```
user_workspace/
├── app_logic/
│   ├── assembly/               ← 组装接线文件（最重要！）
│   │   ├── instantiation_app.h           ← 总装配文件（include所有分类接线子文件，共约10个）
│   │   ├── instantiation_userland.h      ← 最大的接线文件（数万行）
│   │   ├── instantiation_special.h       ← 硬件引脚相关
│   │   └── instantiation_*.h             ← 其他分类接线文件
│   ├── config/
│   │   └── config_app.h        ← 所有模块的Config数值（阈值、定时器参数等）
│   ├── config_tl/              ← 不同机型的配置差异
│   └── bin/                    ← 编译输出（见下面"编译产物"说明）
├── module/
│   ├── wlh/                    ← 手写业务模块（主要阅读对象）
│   │   ├── src/                ← .cc 实现文件（400+个模块）
│   │   └── include/            ← .h 头文件
│   ├── wla/                    ← 自动生成代码（AutoCode工具生成）
│   └── lib/                    ← 库文件
├── system/
│   ├── ErrorSnap/              ← 错误快照日志配置
│   ├── memory/                 ← 内存管理配置
│   └── network/                ← CAN/HTTP网络配置
└── tool/                       ← 开发工具（Excel定义表、脚本等）
    └── AppBuilder/             ← C# 代码生成工具（在PC上运行，不烧写到芯片）
```

### bin/ 目录编译产物说明
| 文件类型 | 说明 |
|---------|------|
| `.mot` | 烧写文件（最终产物，烧到 RH850 芯片） |
| `.abs` | 可执行文件（含调试信息） |
| `.o` | 编译中间产物（单个 .cc 编译出的机器码） |
| `.d` | 依赖文件（记录每个 .cc 编译时依赖了哪些 .h） |
| `.map` | 内存映射（变量/函数在芯片内存中的地址） |
| `.lst` | 汇编列表（C++代码对应的汇编指令） |

### 小技巧：通过 .d 文件找源文件位置
当你找不到某个 .h 或 .cc 文件在哪时，可以看 bin/ 下的 .d 文件：
```
bin/increment_timer.d 里记录了：
  ./../../../utility/[version]/timer/increment_timer.cc
  ./../../../utility/[version]/timer/increment_timer.h
→ 换算出绝对路径: [project_root]/utility/[version]/timer/
```

---

## 四、Manager 树形架构（组合模式）

### 树形结构
```
root_task_manager（根节点，在 Middleware 中创建）
├── special_manager          ← 硬件引脚/传感器（最先执行）
├── memory_framework_manager ← 非易失性内存数据
├── network_manager          ← CAN总线/HTTP通信
├── app_manager              ← 业务逻辑（最大，挂了大量模块）
│   ├── app_manager_node_01  ← 子节点01
│   ├── app_manager_node_02  ← 子节点02
│   ├── ...
│   └── app_manager_node_XX  ← Manager满了就加节点
├── abnormality_send_manager ← 故障报告（DM1/DTC）
└── error_snap_manager       ← Logger日志（ErrorSnap）
```

### Manager 类的定义（Middleware层 framework/manager.h）
```cpp
class Manager : public framework::ModuleInterface {
 public:
  Manager();                 // 无参构造 → 创建根节点（INSTANTIATION_NOARGS 用这个）
  Manager(Manager *manager); // 有参构造 → 创建子节点（INSTANTIATION 用这个，传父节点指针）
  void RegistModule(...);    // 模块注册到这个Manager
  void Update();             // 遍历调用所有子模块的 Update
 private:
  ArrayContainer<ModuleInterface, 32> module_array_;  // 32格盒子！
};
```

### 根节点的创建 — instantiation.h 总入口链
```
Middleware层/framework/UserMainTask.cc     ← Middleware 的入口文件
  └→ #include "instantiation.h"                   ← 总调度文件
       ├→ #define INSTANTIATION_DEFINE             ← 打开"创建对象"开关
       ├→ #include "instantiation_middleware.h"    ← 平级①：Middleware 层
       │    └→ #include "instantiation_middleware_manager.h"
       │         └→ INSTANTIATION_NOARGS(Manager, root_task_manager)  ← 🎯 根节点在这里创建！
       ├→ #include "instantiation_parameter_switcher.h"  ← 平级②
       └→ #include "instantiation_app.h"           ← 平级③：应用层
            ├→ #include "instantiation_special.h"
            │    └→ INSTANTIATION(Manager, special_manager, &root_task_manager)  ← 子节点
            ├→ #include "instantiation_userland.h"
            │    └→ INSTANTIATION(Manager, app_manager, &root_task_manager) ← 子节点
            └→ ...其他分类接线子文件
```
**顺序很重要**：middleware 先 include → 先创建根节点，app 后 include → 后创建子节点。

### 关键规则
- 每个 Manager 最多装 **32 个子模块**（`ArrayContainer<ModuleInterface, 32>`）
- Manager 满了就创建子节点 node00_01, node00_02...
- main_task_manager 调用 Update() 时，**按注册顺序**依次调用每个子模块的 Update()
- 这就是为什么 instantiation_userland.h 中的 **顺序很重要**（先创建 = 先执行）

---

## 五、三个核心文件的分工

### ① 模块文件（module/wlh/）—— "做什么"
- `.h` 文件：声明类的结构（有哪些输入、输出、成员变量）
- `.cc` 文件：实现具体逻辑（构造函数 + Update 方法）
- **模块不知道自己的输入从哪来**，只知道"我会收到这些类型的参数"

### ② 配置文件（config_app.h）—— "用什么数值"
- 存放所有模块的 Config 具体数值（阈值、定时器参数等）
- 例如：`vehicle_completely_stop_model_config = { 40.F, {10U, 0U, 500U} }`
- **模块不 include 这个文件**——模块不知道具体数值从哪来

### ③ 接线文件（instantiation_userland.h）—— "怎么连"
- include 了 config_app.h（所以能看到配置数值）
- include 了所有模块的 .h 文件（所以能看到类的定义）
- 用 INSTANTIATION 宏创建每个模块对象，把配置和信号传进去
- **是唯一知道"谁连谁"的地方**

### 三者关系图
```
模块.h                config_app.h              接线文件
定义 struct Config     填入具体数值               把两者连起来
的"形状"              (40.F, 500U等)
     ↑                     ↑                       |
     |                     |                       |
     └─── 模块只知道     └─── 数值在这里 ──→ 接线文件 include 了它
          Config长什么样                       然后传给模块构造函数
```

**类比**：
- 模块 = 厨师（"给我食材我就做菜，不管你从哪买的"）
- config_app.h = 超市仓库（存放具体食材/数值）
- 接线文件 = 服务员（从仓库取食材，递给厨师）

---

## 六、INSTANTIATION 宏详解

### 基本语法
```cpp
INSTANTIATION(
    (类名),                    // 第1个：要创建什么类型的对象（注意有括号）
    对象名,                    // 第2个：给对象起的名字
    /* 参数名 : */ 参数值1,     // 第3个起：构造函数参数，按顺序一一对应
    /* 参数名 : */ 参数值2,
    ...
    /* manager : */ manager名); // 最后一个总是 manager
```

### 展开原理
```
INSTANTIATION(类型, 名字, 参数...)
  → 展开为：类型 名字(参数...)  （就是创建对象的普通C++代码）
```

### INSTANTIATION_DEFINE 开关
- **定义了 INSTANTIATION_DEFINE** → 宏展开为"创建对象"
- **没定义** → 宏展开为 `extern` 声明（引用别的文件创建的对象）
- 这让一个接线文件可以被多处 include，但只在一处真正创建对象

### 实际例子对照
```cpp
// 接线文件中的写法：
INSTANTIATION(
    (logic::wl::vehicle_information::vehicle_completely_stop_model::Function),
    vehicle_completely_stop_model,
    /* config : */ vehicle_completely_stop_model_config,
    /* revolution : */
    tm_output_convert_model_tm_output_direction_and_revolution_switch_function
        .OutputRevolutionRef(),
    /* manager : */ app_manager_node_05);

// 展开后等价于：
logic::wl::vehicle_information::vehicle_completely_stop_model::Function
    vehicle_completely_stop_model(
        vehicle_completely_stop_model_config,                    // config参数
        tm_output_convert_model_...function.OutputRevolutionRef(), // revolution参数
        app_manager_node_05                                // manager参数
    );
```

### /* 参数名 : */ 是什么？
```cpp
/* config : */ vehicle_completely_stop_model_config,
|←── 注释 ──→| |←── 实际值，这才是代码！──→|
```
`/* ... */` 只是注释标签，帮助人理解"这个值对应构造函数的哪个参数"。
去掉注释后代码完全不变。

---

## 七、模块的固定代码模式

### .h 文件模板
```cpp
class MyModule : public framework::ModuleInterface {
 public:
  // ─── 有可调参数的模块才有 Config ───
  struct Config {
      float32 some_threshold;           // 简单数值
      utility::IncrementTimer::Config timer_config;  // 嵌套的子Config
  };

  // ─── 构造函数 ───
  MyModule(
      const Config& config,                    // 有Config的模块才有这个参数
      const signal_object::Xxx& input1,        // 输入信号（const引用）
      const signal_object::Yyy& input2,
      framework::Manager& manager);            // 最后总是Manager

  // ─── 生命周期方法 ───
  void Update();                               // 必须实现，被循环调用

  // ─── 输出方法 ───
  const signal_object::Zzz& OutputRef() const; // 别的模块通过这个读输出

 private:
  // ─── 输入保存 ───
  const signal_object::Xxx& input1_;           // const & → 只读引用
  const signal_object::Yyy& input2_;

  // ─── 输出变量 ───
  signal_object::Zzz output_;                  // 非const → 自己可以写

  // ─── 配置保存 ───
  const Config& config_;                       // const & → 只读引用

  // ─── 内部状态（有的模块有，有的没有）───
  utility::IncrementTimer timer_;              // 有记忆的成员变量
};
```

### .cc 文件模板
```cpp
// 构造函数
MyModule::MyModule(const Config& config, ..., Manager& manager)
    : input1_(input1),           // 保存输入引用
      config_(config),           // 保存配置引用
      timer_(config.timer_config), // 用配置初始化内部状态
      output_(初始值, VALID) {   // 初始化输出
  manager.RegistModule(*this);   // 注册到Manager（固定写法）
}

// Update — 被Manager循环调用
void MyModule::Update() {
  // 1. 更新内部状态
  // 2. 读输入（input1_.GetValue()）
  // 3. 计算
  // 4. 写输出（output_ = ...）
}

// 输出方法 — 别的模块通过这个读
const signal_object::Zzz& MyModule::OutputRef() const {
  return output_;
}
```

### 四种模块模式对比
| | 组合逻辑（back_lamp） | 定时器防抖（vehicle_completely_stop） | 多输出（vehicle_moving_direction） | 状态机（switch_momentry_convert） |
|---|---|---|---|---|
| 构造函数参数 | 信号输入 + manager | Config + 信号输入 + manager | Config + 信号输入 + manager | 信号输入 + manager |
| 有无Config | 无 | 有 | 有 | 有（但为空） |
| 有无内部状态 | 无 | 有（timer_） | 无 | 有（状态记忆在output_中） |
| 核心逻辑 | if/else | timer + Clear | if/else | switch-case 状态迁移 |
| 输出个数 | 1个 | 1个 | 2个 | 1个 |
| 适用场景 | 简单开关判断 | 需要防抖/计时 | 一个模块产出多信号 | 需要记住"当前处于什么阶段" |

### .h 和 .cc 的对应关系详解

#### .cc 的开头固定两件事
```cpp
// ① include 自己的 .h（拿到类的完整声明）
#include "vehicle_completely_stop_model_function.h"

// ② using 简化超长的命名空间路径
using logic::wl::vehicle_information::vehicle_completely_stop_model::Function;
// 之后就可以直接写 Function，不用写那一长串
```

#### .h 声明 vs .cc 实现的一一对应
```
.h 里声明（"我有这些"）                    .cc 里实现（"具体怎么做"）
──────────────────                        ──────────────────
class Function {                          （不需要重复 class）
  struct Config { ... };                  （不需要重复，.h里已定义）
  Function(参数列表);          ──→         Function::Function(参数列表) : 初始化列表 { 函数体 }
  void Update();               ──→         void Function::Update() { 具体逻辑 }
  const BoolSignal& IsStopRef() const; →   const BoolSignal& Function::IsStopRef() const { return ...; }
  private: 成员变量声明;                    （不需要重复，.h里已声明）
};
```

#### 关键语法：为什么 .cc 里要加 `Function::` 前缀？
```cpp
// .h 里 — 写在 class Function { } 内部，编译器知道这属于 Function
void Update();                       // 不需要前缀

// .cc 里 — 写在 class 外部，必须告诉编译器"这个方法属于谁"
void Function::Update() { ... }      // 必须加 Function::
     ^^^^^^^^^^
     "这个 Update 是 Function 类的"
```

#### 构造函数初始化列表：参数 → 成员变量
```cpp
// .h 里的 private 成员变量           // .cc 里构造函数的初始化列表
private:                              Function::Function(
  const RotationalSpeed& revolution_; //   const RotationalSpeed& revolution,
  const Config& config_;              //   const Config& config,
  IncrementTimer timer_;              //   Manager& manager)
  BoolSignal is_stop_;                //   : revolution_(revolution),    ← 参数→成员
                                      //     config_(config),            ← 参数→成员
                                      //     timer_(config.timer_config),← 从config取子项→成员
                                      //     is_stop_(false, VALID) {    ← 直接初始化
```

#### 命名规律
- 构造函数参数叫 `config` → 成员变量叫 `config_`（加下划线后缀 `_`）
- 参数叫 `revolution` → 成员叫 `revolution_`
- 这样代码里能一眼区分"传进来的参数"和"保存在对象身上的成员"

#### 嵌套类型的 :: 写法
```cpp
utility::IncrementTimer::Config timer_config;
│          │              │
│          │              └── Config 结构体（嵌套在 IncrementTimer 类里面）
│          └── IncrementTimer 类
└── utility 命名空间

utility::IncrementTimer timer_;
│          │
│          └── IncrementTimer 类本身
└── utility 命名空间
```
一个 `#include "increment_timer.h"` 就把类和它的嵌套结构体都引进来了。

#### .cc 必须实现 .h 里声明的所有方法
- .h 声明了3个方法（构造函数、Update、IsStopRef）→ .cc 必须实现这3个
- **不多不少**：.cc 不能实现 .h 里没声明的方法，也不能遗漏
- .h = "菜单"（列出有什么），.cc = "厨房"（写出怎么做）

---

## 八、模块间通信（接线机制）

### 通信方式
```
模块A                    接线文件                     模块B
─────                    ────────                     ─────
输出方法 XxxRef()  ──→   A.XxxRef() 作为参数传给B  ──→  构造函数收到 const Xxx&
                         （INSTANTIATION宏里写出来）     保存到 input_ 成员
                                                        Update()里用GetValue()读
```

### 关键规则
1. **模块A不知道谁在读自己的输出**（松耦合）
2. **模块B不知道输入从哪来**（只知道类型）
3. **只有接线文件知道谁连谁**
4. **参数按位置一一对应**，顺序不能错
5. **被依赖的模块必须先创建**（在接线文件中排在前面 = 先执行 Update）
6. 模块B只能**读**模块A的输出（const &），不能改

### INSTANTIATION 中只有输入，没有输出

```
INSTANTIATION 的参数 = 构造函数的参数 = 全是输入
├── 构造函数定义在模块自己的 .h 中（Function(config, revolution, manager)）
├── INSTANTIATION 展开后就是调用这个构造函数
└── 所以参数全是"传给这个模块的东西" = 全是输入

输出 = 模块的 XxxRef() 方法
├── 定义在模块自己的 .h 中（const BoolSignal& IsStopRef() const;）
├── 不在自己的 INSTANTIATION 里
├── 出现在别人的 INSTANTIATION 里（作为别人的输入参数）
└── 搜 "实例名." 就能找到所有使用这个输出的地方
```
找输入：看自己的 INSTANTIATION 块
找输出：搜 "实例名." → 看被谁引用

### 模块之间完全不互相 #include（松耦合的实现方式）

```
模块A.h 的 #include：
├── rotational_speed.h     ← 信号类型定义
├── bool_signal.h          ← 信号类型定义
├── increment_timer.h      ← 工具类
├── manager.h              ← 框架类
├── module_interface.h     ← 基类
└── ❌ 没有任何其他业务模块的 .h ！

模块A.cc 的 #include：
└── vehicle_completely_stop_model_function.h  ← 只 include 自己的 .h

模块只 include "类型定义"（信号类型、工具类、框架类），
不 include 其他业务模块 → 模块之间完全不认识彼此。
```

**唯一知道所有模块的是接线文件**：
```
instantiation_userland.h 顶部：
├── #include "vehicle_completely_stop_model_function.h"  ← 400+个模块全部 include
├── #include "parking_brake_model_function.h"
├── ...
└── 然后用 INSTANTIATION 把 A.OutputRef() 传给 B 的构造函数 → 完成连接
```

这就是为什么修改一个模块时只需要改它自己的 .h/.cc，不会影响其他模块。

### 信号对象 = 容器（值 + 状态）

每个信号对象都是一个"包裹"，里面装着两样东西：

```cpp
signal_object::BoolSignal(true, signal_object::EnumSignalState::VALID)
//                        ↑值          ↑状态
```
- **VALID** = 这个值是可信的
- **INVALID** = 这个值不可信（比如传感器故障）
- 在安全关键系统中，光有值不够，还要知道值是否可信

### 信号对象的类继承结构

```
enum EnumXxx { 值1, 值2, 值3 }     ← 枚举类型（定义有哪些可能的值）
class Xxx : EnumSignal<EnumXxx>     ← 信号容器类（包裹 = 枚举值 + VALID/INVALID）
```

**例子**：
```
enum EnumSwitchState { ON, OFF }
class SwitchStateSignal : EnumSignal<EnumSwitchState>     ← 输入信号

enum EnumSwitchMomentaryState { OFF, OFF_ON, ON, ON_OFF }
class SwitchMomentaryState : EnumSignal<EnumSwitchMomentaryState>  ← 输出信号
```

### 信号对象的三步操作：拆包→逻辑→装包

```cpp
// ① 拆包（GetValue 取出枚举值）
signal_object::EnumSwitchState::Type in = input_.GetValue();
signal_object::EnumSwitchMomentaryState::Type out = output_.GetValue();

// ② 逻辑处理（纯枚举值层面的判断）
switch (out) { ... }  // 用枚举值做状态迁移

// ③ 装包（构造函数把枚举值+状态塞回容器）
output_ = signal_object::SwitchMomentaryState(out, signal_object::EnumSignalState::VALID);
//        ↑ 类名(值, 状态) = 调用构造函数创建新信号对象
```

**装包语法是标准 C++**：`类名(参数1, 参数2)` = 调用构造函数创建临时对象再赋值，不是项目特有的。

### 信号类型定义在哪？

| 层级 | 文件 | 内容 |
|------|------|------|
| 中间件层 | `tool/AppBuilder/.../SignalObjects.cs` | 通用信号（SwitchStateSignal等） |
| 应用层 | `tool/AppBuilder/.../SignalObjectcs.cs` | 应用自定义信号（SwitchMomentaryState等） |

注意文件名很像：`SignalObjects.cs`（有s）vs `SignalObjectcs.cs`（cs连写）。
C# 文件是代码生成工具的定义，真正的 C++ 定义在 Middleware 层。

### INVALID 的产生规则

- **业务模块（wlh层）永远输出 VALID** — "我算出来的结果就是对的"
- **INVALID 来自底层**：CAN 总线超时、传感器断线等 → SO Converter 标记 INVALID
- 业务模块收到 INVALID 的输入时通常不检查，直接用值

---

## 九、Config 与 config_app.h 详解

### Config 结构体
- 定义在模块自己的 .h 文件中
- 描述 Config 的"形状"——有哪些字段、什么类型
- 可以嵌套（Config 里套另一个 Config）

### config_app.h 中的写法
```cpp
Config vehicle_completely_stop_model_config = {
    /* revolution_threshold = */ 40.F,
    /* timer_config. */ {
        /* sampling_time = */ 10U,
        /* initial_time = */ 0U,
        /* time_up_time = */ 500U
    }
};
```

### C 结构体初始化语法
- `{ 值1, 值2, 值3 }` → 按位置对应 struct 中字段的顺序
- `/* revolution_threshold = */` → 纯注释标签，不影响代码
- `40.F` → float类型的40.0（F后缀 = float）
- `10U` → unsigned类型的10（U后缀 = unsigned）

### 为什么模块不 include config_app.h？
```
模块.h   → 只定义 struct Config 的形状
模块.cc  → 只用 config_.字段名 读值
config_app.h → 填入具体数值

接线文件 include 了 config_app.h
接线文件 include 了 模块.h
接线文件把 config值 传给模块构造函数
→ 模块通过 const Config& 引用拿到值
→ 模块不需要知道值从哪个文件来（依赖注入）
```

### 怎么找某个模块的Config具体数值？
→ 去 **app_logic/config/config_app.h** 中搜索模块名

---

## 十、IncrementTimer（递增定时器）— 源码详解

### 源文件位置
```
[project_root]/utility/[version]/timer/
├── increment_timer.h    ← 声明（接口/形状）
└── increment_timer.cc   ← 实现（具体代码）
```
**不在 user_workspace 里**，在上层的 utility/ 共享库中。模块通过 `#include "increment_timer.h"` 使用它。

### 注意：不要和 C# 版混淆
```
tool/AppBuilder/Middlewares/utility/IncrementTimer.cs  ← C# 代码生成工具用的
utility/[version]/timer/increment_timer.h/.cc          ← 真正烧到芯片上运行的
```
C# 版是 AppBuilder 工具内部的定义（告诉工具"IncrementTimer 的 Config 有3个字段"），
仅在 PC 上运行，**不参与编译，不烧写到芯片**。

### Config 参数
```cpp
struct IncrementTimer::Config {
    uint32 sampling_time;   // 每次 Update() 增加多少毫秒
    uint32 initial_time;    // 计数器初始值（通常为0）
    uint32 time_up_time;    // 到达这个值就算"时间到"
};
```
具体数值不在这里，在 config_app.h 中由使用者填入。
IncrementTimer 是通用工具，20+个模块共用，每个设不同的时间。

### 内部状态（private 成员变量）
```cpp
private:
  const Config& config_;    // 配置引用（sampling_time等）
  bool time_up_;            // "时间到了吗？"（到了之后一直为true）
  bool time_up_now_;        // "时间刚刚到？"（只在到达那一瞬间为true，下次就变false）
  uint32 time_;             // 当前计数值（毫秒）
  uint32 time_up_time_;     // 目标时间（可运行时修改）
```
类比厨房计时器面板：
```
┌──────────────────────────┐
│  time_        = 当前走了多少秒   │  ← 表盘指针
│  time_up_time_ = 定的目标时间    │  ← 你设的闹钟时间
│  time_up_     = 到时间了吗？    │  ← 灯亮=到了（一直亮）
│  time_up_now_ = 刚刚到的吗？    │  ← 铃响=刚到（响一下就停）
└──────────────────────────┘
```

### 所有方法一览
| 方法 | 作用 | 返回/效果 |
|------|------|----------|
| `Update()` | 计时器走一步（+sampling_time） | time_ 增加 |
| `Clear()` | 归零重来 | time_=0, time_up_=false |
| `IsTimeUp()` | 时间到了吗？ | 到了之后**一直**返回 true |
| `IsTimeUpNow()` | 时间**刚刚**到吗？ | 只在到达那**一次**返回 true |
| `GetTime()` | 当前走了多少毫秒？ | 返回 time_ |
| `SetTimeUpTime(v)` | 运行时修改目标时间 | time_up_time_ = v |
| `SetTime(v)` | 运行时修改当前时间 | time_ = v |

### 构造函数源码
```cpp
IncrementTimer::IncrementTimer(const Config& config)
    : config_(config),
      time_up_(false),
      time_up_now_(false),
      time_(config.initial_time),       // 用初始值（通常0）
      time_up_time_(config.time_up_time) {  // 目标时间（如500）
  if (config.initial_time >= config.time_up_time) {
    time_up_ = true;   // 特殊情况：起始值就≥目标，直接算"到了"
  }
}
```

### Update() 源码 — 核心逻辑
```cpp
void IncrementTimer::Update() {
  // ① 防溢出保护：uint32最大值是4294967295，加到头就不再加了
  uint32 uint_max_value = std::numeric_limits<uint32>::max();
  if (time_ > (uint_max_value - config_.sampling_time)) {
    time_ = uint_max_value;          // 封顶！不会溢出回到0
  } else {
    time_ = time_ + config_.sampling_time;  // 正常递增（如 +10ms）
  }

  // ② 判断是否到时间
  if (time_ >= time_up_time_) {      // 到了！
    if (time_up_ == false) {
      time_up_now_ = true;           // 之前没到，现在到了 → "刚到！"
    } else {
      time_up_now_ = false;          // 之前就到了 → 不是"刚到"
    }
    time_up_ = true;                 // 标记"到了"（一直保持）
  } else {
    time_up_now_ = false;            // 还没到
    time_up_ = false;                // 还没到
  }
}
```

### Update 时间线示例（sampling_time=10, time_up_time=30）
```
调用次数   time_    time_up_   time_up_now_   解释
──────────────────────────────────────────────────
第1次       10       false      false          还没到
第2次       20       false      false          还没到
第3次       30       true       true  ⭐       刚到！now只在这一次=true
第4次       40       true       false          之前就到了，now恢复false
第5次       50       true       false          time_up_一直true
```

### IsTimeUp() vs IsTimeUpNow() 对比
```
IsTimeUp()    = ______████████████████  （到了之后一直亮 — "持续信号"）
IsTimeUpNow() = ______█_______________  （只亮一下 — "脉冲信号/边沿触发"）
                      ↑ 到达时刻
```
- `IsTimeUp()` → 适合判断"已经满足时间条件了吗"（如"车已经停了吗"）
- `IsTimeUpNow()` → 适合触发一次性事件（如"刚停下来的那一刻，发一个通知"）

### Clear() 源码 — 归零
```cpp
void IncrementTimer::Clear() {
  time_ = 0;                   // 计数器归零
  time_up_now_ = false;        // 清掉"刚到"
  if (time_up_time_ == 0) {
    time_up_ = true;           // 特殊情况：目标是0，归零后就算"到了"
  } else {
    time_up_ = false;          // 正常情况：归零 = 重新开始计时
  }
}
```

### 典型用法（防抖动/debounce）
```cpp
void Update() {
    timer_.Update();              // 每次+10ms
    if (条件不满足) {
        timer_.Clear();           // 条件断了就清零
    }
    output_ = timer_.IsTimeUp();  // 持续满足够久才输出true
}
```

**防抖动的意义**：传感器信号有噪声，不能一跳变就响应，要"确认持续了一段时间"才算数。这在嵌入式系统中到处都用。

### 溢出保护为什么重要？
- uint32 最大值 = 4,294,967,295（约49.7天 如果每毫秒+1）
- 如果不做保护，加到最大值后会"溢出"回到0 → 定时器突然从"到了"变成"没到"
- 嵌入式系统可能连续运行几个月不重启，必须处理这种极端情况
- `std::numeric_limits<uint32>::max()` 是C++标准库获取类型最大值的方法

---

## 十一、C++ 编译与链接过程

### 概述：从源码到可执行文件分两步

| | 第1步：编译 | 第2步：链接 |
|---|---|---|
| **做什么** | 把每个 .cc **单独**变成 .o | 把所有 .o **拼成**一个可执行文件 |
| **需要什么** | .h（接口/形状） | .o（机器码/实现） |
| **产物** | .o 文件（含"便签"：我需要调用X） | .abs/.mot2（所有代码拼好） |

### 第1步：编译（各 .cc 独立，互不相干）

#### 编译 increment_timer.cc
```
increment_timer.cc
  #include "increment_timer.h"  ← 复制粘贴进来

  编译器看到：
  ┌───────────────────────────────┐
  │ class IncrementTimer { ... }   │ ← 从 .h 粘贴来的
  │ void IncrementTimer::Update(){ │ ← .cc 自己的实现
  │   time_ += sampling_time;      │
  │ }                              │
  └───────────────────────────────┘
         ↓ 编译
  increment_timer.o  （机器码）
```

#### 编译 vehicle_completely_stop_model_function.cc
```
vehicle_completely_stop_model_function.cc
  #include "vehicle_completely_stop_model_function.h"
    └→ 里面有 #include "increment_timer.h"  ← 也粘贴进来

  编译器看到：
  ┌──────────────────────────────────────┐
  │ class IncrementTimer { ... }          │ ← 知道它的"形状"
  │ class Function {                      │
  │   IncrementTimer timer_;              │ ← 知道timer_有多大
  │ };                                    │
  │ void Function::Update() {             │
  │   timer_.Update();  ← 留便签：        │
  │   "请找IncrementTimer::Update的地址"   │
  │ }                                     │
  └──────────────────────────────────────┘
         ↓ 编译
  vehicle_completely_stop_model_function.o
  （机器码 + 便签："IncrementTimer::Update 的代码在哪？"）
```

**关键点**：编译时**只需要 .h**（知道形状/接口），不需要 .cc（不需要知道具体实现）。
编译器在 .o 文件里留下"便签"→ 等链接器来填。

### 第2步：链接（把所有 .o 拼在一起）
```
increment_timer.o                  vehicle_completely_stop...o
┌───────────────────┐             ┌────────────────────────────┐
│ Update() 的代码    │             │ Function::Update() {       │
│ 在地址 0x1A22E6    │←── 链接 ──→│   调用 timer_.Update()     │
│                   │             │   便签："找Update的地址"     │
└───────────────────┘             └────────────────────────────┘
         ↘                        ↙
           ↓ 链接器拼接（把便签替换成真实地址）
    ┌──────────────────────────────┐
    │  controller.abs                │ ← 可执行文件
    │  Function::Update() {         │
    │    跳转到 0x1A22E6 执行        │ ← 便签被替换了！
    │  }                            │
    └──────────────────────────────┘
         ↓ 转换格式
    controller.mot  ← 烧写到 RH850 芯片
```

### 为什么两个文件 include 同一个 .h 不代表它们有直接关联？
```
              increment_timer.h（声明）
               ↑              ↑
               |              |
vehicle_completely_stop...h   increment_timer.cc
"我要用它作为成员变量，       "我来实现它的方法，
 需要知道它长什么样"           需要知道它长什么样"
```
- 两个文件**各自独立编译**，互相不认识
- 它们只是碰巧都需要看同一个 .h（就像顾客和厨师都看菜谱，但彼此不交流）
- **链接器**负责把它们最终拼在一起

### #include 和 链接 的区别
| | #include（编译时） | 链接（编译后） |
|---|---|---|
| **时机** | 编译前的预处理阶段 | 所有 .cc 编译完之后 |
| **做什么** | 复制粘贴 .h 文件内容 | 把 .o 文件拼成可执行文件 |
| **解决什么** | "这个类长什么样？" | "这个方法的代码在哪？" |
| **类比** | 看菜谱（知道菜名和配料） | 把厨师做的菜端上桌（连接） |

### 本项目的编译流程
```
源码文件                     编译                  链接              转换
─────────                   ────                  ────              ────
400+个 module/wlh/src/*.cc → 400+个 .o  ─┐
increment_timer.cc        → .o        ─┤
manager.cc                → .o        ─┼─→ controller.abs → .mot
其他 Middleware/*.cc       → .o        ─┤                   (烧写)
其他 OS层/*.cc             → .o        ─┘
```

---

## 十二、国际通用知识（走到哪都能用） / Universal Knowledge (Portable Skills)

### C++ 语法速查 / C++ Syntax Quick Reference
| 符号 Symbol | 含义 Meaning | 例子 Example |
|------|------|------|
| `::` | Scope resolution operator（作用域解析 — 命名空间/类成员） | `signal_object::BoolSignal` |
| `&` | Reference (alias, no copy) / Address-of（引用/取地址） | `const Config& config` |
| `*` | Pointer / Dereference（指针/解引用） | `*this` |
| `const` | Immutable / read-only（不可修改） | `const float32& value` |
| `extern` | External linkage — object defined elsewhere（别处创建的对象） | `extern Config config;` |
| `virtual` | Virtual function — overridable by subclass（子类可重写） | `virtual void Update();` |
| `= 0` | Pure virtual function — subclass must implement（纯虚函数） | `virtual void Update() = 0;` |
| `<>` | Template type parameter（模板类型参数） | `ArrayContainer<Module, 32>` |
| `*this` | The object itself — dereference of `this` pointer（对象自身） | `manager.RegistModule(*this)` |
| `#define` | Preprocessor macro — text substitution（宏/文本替换） | `#define MAX 100` |
| `#include` | Preprocessor include — textual copy-paste of a file（复制粘贴文件内容） | `#include "config_app.h"` |
| `#ifndef/#define/#endif` | Include guard — prevents duplicate inclusion（头文件保护） | 防重复包含 |
| `namespace` | Namespace — logical grouping to avoid name collisions（命名空间） | `namespace logic { }` |
| `class / struct` | Class / Struct — struct defaults to public, class defaults to private | struct 默认public，class默认private |
| `public / private` | Access specifiers（访问控制） | public = accessible from outside |
| `: public Base` | Public inheritance（公有继承） | `class Func : public ModuleInterface` |
| Initializer list（初始化列表） | `member_(value)` after colon in constructor（冒号后的成员初始化） | `timer_(config.timer_config)` |
| `.h` / `.cc` | Header (declaration) / Source (implementation)（声明/实现） | .h defines interface, .cc writes code |
| `40.F` | Float literal（float字面量） | F suffix = float type |
| `10U` | Unsigned literal（unsigned字面量） | U suffix = unsigned type |
| `0UL` | Unsigned long literal | UL suffix |
| `std::numeric_limits<T>::max()` | Maximum value of a type（获取类型最大值） | Used for overflow protection |

### C 注释语法 / C Comment Syntax
```cpp
/* This is a comment */  42    ← 42 is NOT a comment!
// This is also a comment (single-line)
/* revolution_threshold = */ 40.F   ← 40.F is code; the part before it is a comment label
```
Only content between `/*` and `*/` is a comment; everything outside is normal code.
config_app.h extensively uses `/* field_name = */` as labels to help map values to struct fields.

### C 结构体初始化语法 / C Aggregate Initialization Syntax
```cpp
struct Config {
    float32 revolution_threshold;              // Field 1
    utility::IncrementTimer::Config timer_config;  // Field 2 (nested struct)
};

Config config = {
    40.F,                    // ← Field 1 (simple value, no braces needed)
    {10U, 0U, 500U}          // ← Field 2 (nested struct, must wrap in { })
};
```
**Rule**: Each level of struct nesting corresponds to one level of `{ }` braces.

### 设计模式 / Design Patterns
| 模式 Pattern | 在项目中的体现 How it appears in this project |
|------|--------------|
| Composite（组合模式） | Manager contains Modules; Manager itself is also a Module |
| Dependency Injection（依赖注入） | Dependencies passed in via constructor parameters; modules never create or look up their own dependencies |
| Interface Abstraction（接口抽象） | ModuleInterface defines a uniform interface (Update, etc.); all modules implement it |
| Star Topology State Machine（星形拓扑状态机） | All states route through a central hub (NOT_ACTIVE); no direct state-to-state transitions |

### 数据类型 / Data Types
| 类型 Type | 说明 Description |
|------|------|
| int / unsigned int | Signed integer / Unsigned integer（整数/无符号整数） |
| uint8 / uint16 / uint32 | Fixed-width unsigned integers — 8/16/32-bit（固定宽度无符号整数） |
| sint8 / sint16 / sint32 | Fixed-width signed integers（固定宽度有符号整数） |
| float32 / float64 | Floating-point — single/double precision（单精度/双精度浮点数） |
| bool | Boolean — true/false |
| enum（枚举） | Enumeration — a set of named constants, e.g. ON/OFF/REVERSE/FORWARD |

### 嵌入式通用概念 / General Embedded Concepts
- **No dynamic memory allocation**（不用动态内存 new/delete） — use fixed-size arrays to prevent memory fragmentation
- **Signal value = value + status** (VALID/INVALID)（信号值 = 值 + 状态） — prevents decisions based on invalid data
- **const protection**（const保护） — prevents accidental modification in safety-critical systems
- **All objects determined at compile time**（编译期确定所有对象） — zero runtime overhead, no surprises
- **Module lifecycle**: Init → Update (called cyclically, e.g. every 10ms) → Exit（模块生命周期）
- **Debounce filtering**（防抖动） — sensor signals have noise; condition must be sustained for a period before it counts
- **Overflow protection**（溢出保护） — integers have a max value (uint32 ≈ 4.29 billion); clamp at max instead of wrapping around to 0
- **Level signal vs edge signal**（持续信号 vs 脉冲信号） — IsTimeUp() stays true persistently vs IsTimeUpNow() is true for exactly one cycle
- **Edge detection**（边沿检测） — converts persistent ON/OFF level signals into momentary OFF_ON/ON_OFF edge signals
  - Rising edge (OFF_ON) = "the instant the button was just pressed"（上升沿 — 刚按下的那一瞬间）, exists for exactly 1 cycle
  - Falling edge (ON_OFF) = "the instant the button was just released"（下降沿 — 刚松开的那一瞬间）, exists for exactly 1 cycle
  - Purpose: triggers only once regardless of press duration; prevents repeated responses
  - Analogy: pressing a doorbell for 3 seconds rings it once, not 300 times
- **Finite State Machine (FSM)**（状态机） — implemented with switch-case + enum for state transitions
  - Current state + input conditions → determine next state
  - State memory stored in the output variable (last output = current state this cycle)
- **Three-phase State Machine**（三相状态机） — transition / entry / do, three separated phases
  - Transition: evaluate whether a state change should occur（判断是否跳转）
  - Entry: one-time initialization upon entering a new state, e.g. Clear timers（进入新状态时做一次性初始化）
  - Do: continuous action while remaining in the state, e.g. Update timers each cycle（在当前状态每周期持续执行）
  - Uses a `pre_` snapshot to detect "did a state change just happen" (state-level edge detection)
- **Star Topology State Machine**（星形拓扑状态机） — all states route through a central hub (e.g. IDLE / NOT_ACTIVE)
  - No direct transitions between non-idle states
  - Design principle: not needed + not safe + more complex = don't do it（不需要 + 不安全 + 更复杂 = 不做）
  - Common pattern in industrial control systems (safety first)
- **Sensor redundancy**（冗余传感器） — same physical quantity measured by two sensors; if one fails, the other takes over
- **Limp-home mode**（跛行回家） — when all sensors fail, switch to estimated values and drive back slowly for repair; never strand the machine
- **Single-responsibility pipeline**（单一职责流水线） — each module does only one thing; complex functions are achieved by chaining multiple modules
- **Polling vs blocking wait**（轮询 vs 阻塞等待） — embedded code never "waits"; it checks the condition every cycle and skips if not met
  - Cooperative scheduling: every module must complete quickly and return control（协作式调度 — 每个模块快速执行完并返回）
  - "Waiting 500ms" = the timer runs for 50 cycles (50 × 10ms), each cycle the check returns "not yet"
  - Analogy: checking your mailbox once a day (polling), not standing at the door waiting for the courier (blocking)
- **Safety override pattern**（安全层 override 模式） — does not interfere with the internal state machine logic; only intercepts the output at the very last step
  - The state machine continues running in the background; recovery is immediate once the fault clears
- **Sequential logic vs combinational logic**（时序逻辑 vs 组合逻辑）:
  - Combinational = input → immediate output（组合逻辑 — back_lamp: reverse gear → light on）
  - Sequential = requires "memory" and "time"（时序逻辑 — vehicle_completely_stop: speed below threshold for 500ms to count as stopped）
- **Compilation vs linking**（编译 vs 链接）:
  - Compilation = each .cc compiled independently into .o (only needs .h for type/shape information)
  - Linking = all .o files combined into one executable (unresolved symbols replaced with real addresses)
  - `#include` solves compile-time questions ("what does this type look like?")
  - Linking solves run-time questions ("where is the actual code for this function?")

### 编程思维 / Programming Mindset
- **Positional parameter matching**（函数参数按位置一一对应） — order matters, cannot be swapped
- **Dependency-ordered instantiation**（按依赖顺序创建对象） — dependencies must be created first
- **Loose coupling via const &**（模块间松耦合） — modules read others' outputs through const references; cannot modify
- **Aggregate initialization by field order**（结构体初始化按字段顺序填值）: `{ val1, val2, { sub1, sub2 } }`
- **Shared #include ≠ direct relationship**（include 同一个 .h ≠ 直接关联） — files are compiled independently
- **Generic tools carry no concrete values**（通用工具不含具体数值） — values are injected from the outside (e.g. IncrementTimer receives its config from the caller)
- **const & member = "borrowed" data**（看别人的，不拥有、不能改）; **non-const non-& member = "owned" data**（自己的，拥有、能改）
- **Three constructor initializer-list patterns**（构造函数初始化列表三种模式）: reference binding (const&), object construction (from Config), value initialization (bool/enum)
- **Config struct ≠ tool object**（Config ≠ 工具对象）: Config = pure data / parameters; tool = stateful object with methods (e.g. IncrementTimer)
- **Inner class for namespace isolation**（inner class 做命名空间隔离）: placing an enum inside a class prevents global name collisions (via `ClassName::EnumValue`)
- **Read variable names in reverse**（变量名倒着读）: `is_wiper_sw_turned_off_` = "is–wiper-switch–turned–off" = "has the switch ever been released?"
- **Same enum name in different classes does not conflict**（同名枚举不冲突）: different classes may each have `enum Type`; disambiguated by `ClassName::Type` (project convention)

---

## 十三、项目专有框架知识（已脱敏）

> ⚠️ 以下内容已移除客户专有名称。面试中可用通用术语描述这些机制。

### 项目专属宏
- `INSTANTIATION(类型, 名字, 参数...)` → 创建对象（宏名可能因项目而异）
- `INSTANTIATION_NOARGS(类型, 名字)` → 无参创建
- `INSTANTIATION_DEFINE` → 控制"创建"还是"extern 声明"
- 两层展开：外层宏 → 内层实体宏 → 实际代码
- 面试表述："compile-time dependency injection via macro-based object wiring"

### 框架核心类
| 类（通用描述） | 作用 |
|---|------|
| `framework::Manager` | 模块容器（固定大小数组，如32格） |
| `framework::ModuleInterface` | 所有模块的父类（定义 Update 等生命周期方法） |
| `signal_object::BoolSignal` | 布尔信号（true/false + VALID/INVALID） |
| `signal_object::RotationalSpeed` | 转速信号 |
| `signal_object::TravelDirection` | 行驶方向信号 |
| `signal_object::SwitchStateSignal` | 开关状态信号（ON/OFF） |
| `signal_object::SwitchMomentaryState` | 开关边沿信号（OFF/OFF_ON/ON/ON_OFF） |
| `signal_object::ElectricalOnOffState` | 电气开关状态 |
| `signal_object::Time` | 时间信号 |
| `utility::IncrementTimer` | 递增定时器（防抖动用） |
| `utility::ArrayContainer<T, N>` | 固定大小数组容器 |

### 项目关键文件（已泛化）
| 文件（泛化名） | 作用 |
|------|------|
| `instantiation_app.h` | 总装配文件（include所有分类接线子文件） |
| `instantiation_userland.h` | 最大的接线文件（数万行，创建模块并接线） |
| `config_app.h` | 所有模块的Config具体数值 |
| `module/wlh/src/*.cc` | 手写业务模块实现（400+个） |
| `module/wlh/include/*.h` | 手写业务模块头文件 |
| `module/wla/` | 自动生成代码 |
| `bin/*.mot` | 烧写文件 |

---

## 十四、工程专业知识（代码之外的设计思维） / Engineering Domain Knowledge (Design Principles Beyond Code)

> 这一章记录"为什么要这样设计"——不是 C++ 语法，而是工程机械控制系统的设计原则。
> This chapter documents *why* things are designed this way — not C++ syntax, but engineering design principles for heavy-equipment control systems.

### 14.1 操作意图 ≠ 物理现实 / Operator Intent ≠ Physical Reality

**原则 / Principle**：驾驶员的操作（挡位、踏板）和车辆的物理状态（速度、转速）是两个独立的信息源，必须分开建模。
The operator's commands (gear lever, pedals) and the vehicle's physical state (speed, RPM) are two independent information sources and must be modelled separately.

| | 操作意图 Operator Intent | 物理现实 Physical Reality |
|---|---|---|
| **来源 Source** | Human input (joystick, buttons, pedals)（人 — 操纵杆、按钮、踏板） | Sensors (speed, RPM, pressure)（传感器 — 转速、车速、压力） |
| **本项目例子 Example** | `travel_direction` (FNR gear position) | `vehicle_moving_direction` (actual velocity direction) |
| **硬件 Hardware** | FNR switch (Forward/Neutral/Reverse) | Transmission output shaft speed sensor |
| **特点 Characteristic** | Changes instantly (immediate on shift)（立刻变化） | Has inertia/delay (vehicle must accelerate/decelerate)（有延迟） |

**为什么不能只用一个？ / Why can't we use just one?**

```
Scenario: Bulldozer going downhill, operator shifts from Forward to Reverse
场景：推土机在下坡，驾驶员从前进挡换到倒挡

  Time T1: travel_direction = REVERSE  (gear already shifted)
           vehicle_moving_direction = FORWARD  (vehicle still sliding forward!)

  If we engage reverse clutch based on gear alone → transmission damage!
  如果只看挡位就接合倒挡离合器 → 损坏变速箱！

  If we also check speed and see it's still moving forward → wait until stopped → safe
  如果同时看速度，发现还在前进 → 等车停稳再接合 → 安全
```

**设计方法 / Design Method**：Cross-validation（交叉验证）
```
is_ods_brake_reverse = (STOP && in reverse gear) || (actually moving backward)
                        ↑                          ↑
                   Intent + Reality           Pure Reality
                   操作意图+物理现实            纯物理现实
```
The collision detection system adopts a conservative strategy: better a false positive than a missed detection.
碰撞检测系统采取"宁可误判也不漏判"的保守策略。

### 14.2 防抖动 / Debounce Filtering

**问题 / Problem**：Sensor signals contain noise and may oscillate rapidly near threshold values.
传感器信号有噪声，可能在阈值附近快速跳变。
```
Actual RPM (smooth):    ────────────╲_____________________
Sensor reading (noisy): ─────┬─┬──╲┬┬──┬________________
                             40rpm threshold line
                        ↑↑↑↑ rapid crossings
```

**如果不防抖 / Without debouncing**: The system would rapidly toggle between "stopped / not stopped", causing the clutch/brake to actuate repeatedly → mechanical wear, poor operator experience.
系统会在"停了/没停"之间快速切换，导致离合器/刹车频繁动作 → 机械磨损、驾驶员体验差。

**防抖方案 / Debounce solution**: The condition must be sustained for a defined duration before it takes effect.
持续满足条件一段时间才生效。
```
vehicle_completely_stop: RPM continuously below 40 for 500ms → then considered "truly stopped"
                         转速持续低于40rpm，维持500ms，才算"真的停了"
```

**在这个项目中的实现 / Implementation in this project**: IncrementTimer + Clear pattern
```
Condition met   → timer increments    条件满足 → 计时器累加
Condition broken → timer resets to 0  条件断了 → 计时器清零
Timer reaches threshold → output true 计时器到达阈值 → 才输出true
```

### 14.3 安全关键系统的保守原则 / Conservative Principles for Safety-Critical Systems

**原则 / Principle**: Better to limit functionality than to cause danger.
宁可限制功能，也不能造成危险。

| 策略 Strategy | 说明 Description | 项目中的例子 Project Example |
|------|------|------------|
| **Output permission**（输出许可） | No output allowed until the system is ready（系统没准备好时不允许输出） | `output_permission_model` — no control signals before power-up init completes |
| **Graceful degradation**（故障降级） | Use safe default values when a sensor fails（传感器坏了就用安全默认值） | INVALID signal status — use conservative value when data is untrustworthy |
| **Multi-source validation**（多源验证） | Cross-check using multiple independent signals（用多个独立信号交叉确认） | Speed + gear cross-validated to determine direction |
| **Debounce filtering**（防抖动） | Reject transient noise to prevent false actuation（防止噪声导致误动作） | Speed below threshold sustained for 500ms before "stopped" |
| **Sensor redundancy**（冗余传感器） | Same physical quantity measured by two sensors（同一物理量用两个传感器测量） | Transmission output shaft dual speed sensors A/B |
| **Limp-home mode**（跛行回家） | Even total sensor failure must not strand the machine（传感器全坏也不趴窝） | Switch to estimated RPM on total sensor failure; drive back slowly for repair |

### 14.4 信号的三种含义层次 / Three Abstraction Layers of a Signal

同一个物理量在系统中经过多层处理：
The same physical quantity passes through multiple processing layers in the system:

```
Layer 1: Raw hardware signal     → Sensor pulses / voltage (gear pulses, 0–5V analog)
层次1:   原始硬件信号               传感器脉冲/电压

         ↓ Device driver layer (device::RotationalSpeedSensorByDcPulseInput, etc.)
         ↓ 设备驱动层

Layer 2: Physical quantity signal → RPM, speed (km/h), pressure (MPa)
层次2:   物理量信号                 转速(rpm)、速度(km/h)、压力(MPa)

         ↓ Conversion / switching layer (diagnostic switchover, redundancy selection, limp-home switch)
         ↓ 转换/切换层

Layer 2.5: Safety-processed signal → RPM after A/B redundancy selection and fault degradation
层次2.5:   安全处理后信号              经过 A/B 冗余选择和故障降级的转速

         ↓ Business modules
         ↓ 业务模块

Layer 3: State / decision signal  → "Stopped?" (bool), "Direction" (FORWARD/REVERSE/STOP)
层次3:   状态/判断信号                "停了吗"(bool)、"方向"(FORWARD/REVERSE/STOP)
```

实际追踪发现：层次2内部也有多步处理（见第十七章数据流全链路追踪）。
Actual tracing revealed: Layer 2 itself contains multiple processing steps (see Chapter 17, full data-flow trace).

### 14.5 变速箱控制系统常见信号 / Common Signals in Transmission Control Systems

| 信号名 Signal Name | 中文 Chinese | 物理来源 Physical Source | 用途 Purpose |
|---------------|------|---------|------|
| travel_direction | 行驶方向（挡位） | FNR joystick switch（FNR操纵杆开关） | Operator's intended direction（驾驶员想往哪走） |
| vehicle_velocity | 车速 | Output shaft RPM × tire radius（变速箱输出轴转速 × 轮胎半径） | Actual vehicle speed（实际多快） |
| engine_speed / revolution | 发动机/输出轴转速 | Speed sensor（转速传感器） | RPM control（转速控制） |
| oil_temperature | 油温 | Temperature sensor（温度传感器） | Thermal protection（热保护） |
| brake_stroke | 刹车踏板行程 | Displacement sensor（位移传感器） | Operator braking intent（驾驶员刹车意图） |
| parking_brake | 驻车制动 | Switch（开关） | Is the parking brake engaged?（是否锁住） |
| key_sw / starter | 钥匙开关 | Rotary switch（旋钮开关） | Ignition state: START/ACC/IG（启动/ACC/IG状态） |
| accumulator_pressure | 蓄能器压力 | Pressure sensor（压力传感器） | Transmission hydraulic system（变速箱液压系统） |

### 14.6 FNR 操纵杆 / FNR Joystick (Forward-Neutral-Reverse)

```
      F (Forward / 前进)
      ↑
  N──●──N (Neutral / 空挡)
      ↓
      R (Reverse / 倒车)
```
- Direction control lever for bulldozers/loaders, typically on the operator's left side
  推土机/装载机的方向控制杆，通常在驾驶员左手边
- Unlike passenger cars with P/R/N/D, construction equipment typically has only F/N/R
  不像汽车有 P/R/N/D 多个挡位，工程机械通常只有 F/N/R
- FNR signal acquired via switch or potentiometer, converted to `travel_direction` signal
  FNR 信号通过开关或电位器采集，转换为 `travel_direction` 信号

### 14.7 单一职责与流水线设计 / Single Responsibility & Pipeline Design

**原则 / Principle**: Each module does exactly one thing; complex functionality is achieved by chaining multiple modules in a pipeline.
每个模块只做一件事，复杂功能通过多个模块串联完成。

```
Why not do "read sensor → redundancy selection → fault switchover → stop detection" all in one module?
为什么不在一个模块里把"读传感器→冗余选择→故障切换→停车判断"全做完？

→ Each module handles one step; modifying/replacing/testing affects only that single module
→ 每个模块只做一步，修改/替换/测试时只影响一个模块

→ Analogy: factory assembly line — each station performs exactly one operation
→ 类比工厂流水线：每个工位只做一道工序
```

**数据流全链路实例 / Full signal-chain example**（见第十七章详细追踪 / see Chapter 17 for full trace）:
```
Pulse→RPM (device driver) → Select A/B (redundancy) → Normal/limp-home (fault switch) → Stopped? (decision)
脉冲→转速（设备驱动）        → 选A/B（冗余选择）       → 正常/跛行（故障切换）          → 停了吗（判断）

4 modules, each handling one step — not 1 module doing all 4 steps
4个模块各做一步，不是1个模块做4步
```

---

## 十五、已读懂的模块

### 模块1：key_on_elapsed_time_model_function（钥匙开启经过时间）
- **输入**：无
- **输出**：Time（从钥匙开启开始累积的时间）
- **逻辑**：纯计时器，每次 Update 累加时间
- **代码模式**：最简单的模块，无输入纯输出
- **学到的**：模块的基本骨架（构造→注册→Update→输出）

### 模块2：back_lamp_model_function（倒车灯控制）
- **输入**：4个（输出许可、行驶方向、点火开关、ACC开关）
- **输出**：1个（倒车灯继电器 ON/OFF）
- **逻辑**：组合逻辑 if/else（倒挡且有许可→亮灯）
- **无Config**：逻辑写死，没有可调参数
- **无内部状态**：纯组合，输入变→输出立刻变
- **学到的**：多输入→判断→单输出的基本模式

### 模块3：back_up_alarm_model_function（倒车警报控制）
- **输入**：5个（比倒车灯多了静音开关）
- **输出**：1个（倒车蜂鸣器 ON/OFF）
- **逻辑**：组合逻辑 if/else，和倒车灯类似但多了静音条件
- **学到的**：同类模块的差异对比（灯 vs 警报）

### 模块4：output_permission_model_output_permit_judge_function（输出许可判断）
- **功能**：判断系统是否允许输出控制信号
- **学到的**：权限/许可判断模块的存在

### 模块5：vehicle_completely_stop_model_function（车辆完全停止判断） ⭐新知识最多
- **输入**：1个（revolution 转速，来自变速箱输出轴）
- **输出**：1个（is_stop 是否停了）
- **逻辑**：时序逻辑（转速低于阈值 + 持续一段时间 → 才算停）
- **有Config**：revolution_threshold=40.0rpm, time_up_time=500ms
- **有内部状态**：IncrementTimer timer_（跨 Update 调用保持计时）
- **被9+个模块引用**：parking_brake、neutral_safety、shift_change 等
- **核心代码**：
```cpp
void Update() {
    timer_.Update();                    // 每次+10ms
    if (revolution >= 40.0) {           // 转速还高？
        timer_.Clear();                 // 清零重来
    }
    is_stop_ = timer_.IsTimeUp();       // 累积≥500ms才输出true
}
```
- **学到的新概念**：
  1. **Config机制** — 可调参数单独管理（config_app.h）
  2. **时序逻辑** — 有记忆的成员变量（timer_跨Update保持状态）
  3. **防抖动/debounce** — 持续满足条件一段时间才生效
  4. **嵌套Config** — Config里套IncrementTimer::Config
  5. **三文件协作** — 模块.h定义形状、config_app.h填数值、接线文件传递

### 模块6：vehicle_moving_direction_model_function（车辆移动方向判断）
- **输入**：2个
  - `filtered_vehicle_velocity`（Velocity）— 过滤后的车速（正数=前进，负数=后退）
  - `travel_direction`（TravelDirection）— 挡位操纵杆方向（FORWARD/NEUTRAL/REVERSE）
- **输出**：2个 ⭐第一次见2个输出
  - `VehicleMovingDirectionRef()` — 移动方向（FORWARD/REVERSE/STOP 枚举）
  - `IsOdsBrakeReverseRef()` — 碰撞检测系统是否认为在倒车（bool）
- **有Config**：velocity_threshold_for_moving = 0.01（几乎=速度不为零就算在动）
- **无内部状态**：纯组合逻辑
- **核心代码**：
```cpp
void Update() {
    // 第一个输出：根据速度正负判断方向
    if (velocity > +0.01)       → FORWARD
    else if (velocity < -0.01)  → REVERSE
    else                        → STOP

    // 第二个输出：碰撞检测倒车判断（更保守）
    is_ods_brake_reverse = (STOP且挂倒挡) || (实际在后退)
}
```
- **学到的新概念**：
  1. **2个输出** — 一个模块可以提供多个输出信号
  2. **有正负的速度值** — 正数=前进，负数=后退（不同于转速只有正值）
  3. **`-1.0F *` 取反** — 把正阈值变负数，用于判断反方向
  4. **操作意图 ≠ 物理现实** — travel_direction是"想往哪走"，vehicle_moving_direction是"实际往哪走"
  5. **多信号交叉验证** — 安全关键系统通用原则：把操作意图和物理测量分开，交叉判断更保守
  6. **日文注释** — 车体行进方向值，碰撞检测

### 模块7：switch_momentry_convert_model_function（开关边沿转换） ⭐状态机模式
- **输入**：1个
  - `input_`（SwitchStateSignal）— 开关状态（ON/OFF）
- **输出**：1个
  - `OutputRef()`（SwitchMomentaryState）— 边沿信号（OFF/OFF_ON/ON/ON_OFF 四状态）
- **无Config**：Config 结构体为空（不需要可调参数）
- **有内部状态**：状态记忆在 output_ 中（上次输出 = 这次的当前状态）
- **逻辑**：switch-case 状态机
- **状态迁移图**：
```
    ┌─────────────────────────────────────────┐
    │              input=ON                    │
    │                 ↓                        │
    │  OFF ──(ON)──→ OFF_ON ──(ON)──→ ON      │
    │   ↑              ↓(OFF)          ↓(OFF) │
    │   └──(OFF)── ON_OFF ←──(OFF)────┘      │
    │              ↑(ON)                       │
    │              └─→ OFF_ON                  │
    └─────────────────────────────────────────┘
```
- **核心代码**（三步：拆包→匹配→装包）：
```cpp
void Update() {
    // ① 拆包：从信号容器中取出枚举值
    bool input_on = (input_.GetValue() == EnumSwitchState::ON);
    EnumSwitchMomentaryState::Type out = output_.GetValue();  // 上次的输出=当前状态

    // ② 状态迁移：纯枚举匹配
    switch (out) {
        case OFF:    if (input_on) out = OFF_ON; break;
        case OFF_ON: if (input_on) out = ON; else if (input_off) out = ON_OFF; break;
        case ON:     if (input_off) out = ON_OFF; break;
        case ON_OFF: if (input_on) out = OFF_ON; else if (input_off) out = OFF; break;
    }

    // ③ 装包：把新枚举值+VALID塞回容器
    output_ = SwitchMomentaryState(out, EnumSignalState::VALID);
}
```
- **为什么需要这个模块？（边沿检测的意义）**：
  - 开关信号每10ms读一次，按住1秒 = 100次ON
  - 如果下游直接判断 ON → 按住就持续触发（如挡位切换100次）
  - 有了 OFF_ON，下游判断 `if (state == OFF_ON)` → 只在按下那一瞬间触发一次
  - **一句话：把"按住"变成"按了一下"**
- **学到的新概念**：
  1. **状态机模式** — switch-case + 枚举 = 第4种常见模块模式
  2. **边沿检测（Edge Detection）** — 持续信号→瞬间信号，只触发1个周期
  3. **信号对象 = 容器** — 枚举值 + VALID/INVALID，用 GetValue() 拆包，构造函数装包
  4. **信号类型分两处定义** — 中间件层（SignalObjects.cs） vs 应用层（SignalObjectcs.cs）
  5. **INVALID 不在业务层产生** — wlh 模块永远输出 VALID，INVALID 来自底层
  6. **状态记忆在输出中** — output_.GetValue() 既是上次的输出，也是这次的当前状态
  7. **Momentary = 瞬间的** — 模块名含义（虽然拼成了 momentry）

### 模块8：wiper_control_model_function（雨刷/清洗控制） ⭐⭐最复杂的模块，集大成

> **文件位置**：
> - 头文件：`module/wlh/include/wiper_control_model_function.h`
> - 实现：`module/wlh/src/wiper_control_model_function.cc`
> - 依赖：`utility/[version]/timer/increment_timer.h/.cc`（计时器源码）

#### 8.1 模块功能概述

控制建筑机械驾驶室的雨刷和清洗器。操作员面前有一个按钮：
- **快速点按松手** → 雨刷刷一次（剧情1）
- **按住不松手（按够久）** → 喷水清洗（剧情2）
- **开关故障 / 系统禁止** → 强制全关（剧情3）

#### 8.2 输入/输出/配置

**3个输入**（全部是 `const &` 只读引用）：

| 变量名 | 类型 | 中文含义 |
|--------|------|---------|
| `is_sw_abnormal_` | `BoolSignal` | 开关是否硬件故障 |
| `is_wiper_control_available_` | `BoolSignal` | 系统是否允许雨刷工作 |
| `wiper_sw_state_` | `SwitchMomentaryState` | 雨刷开关当前状态（OFF/ON/OFF_ON/ON_OFF） |

> `wiper_sw_state_` 来自 Module 7（switch_momentary_convert）的输出。Module 7 和 Module 8 是上下游关系：
> `物理按钮 → Module 7(边沿检测) → OFF/ON/OFF_ON/ON_OFF → Module 8(雨刷控制)`

**2个输出**（`const &` 返回，外部只读）：

| 方法 | 类型 | 中文含义 |
|------|------|---------|
| `WiperRelayOrderRef()` | `ElectricalOnOffState` | 雨刷继电器指令 ON/OFF |
| `WasherRelayOrderRef()` | `ElectricalOnOffState` | 清洗继电器指令 ON/OFF |

**4个计时器配置**（Config 结构体，数值来自 config_app.h）：

| Config 字段 | 对应计时器成员 | 简称 | 一句话作用 |
|-------------|---------------|------|-----------|
| `wiper_sw_off_judge_timer_config` | `wiper_sw_off_judge_timer_` | 松开计时器 | "开关松开多久了？" |
| `washer_and_wiper_sw_order_timer_config` | `washer_and_wiper_sw_order_timer_` | 按住计时器 | "开关按住多久了？" |
| `wiper_finish_timer_config` | `wiper_finish_timer_` | 雨刷完成计时器 | "雨刷刷了多久了？该停了吗？" |
| `washer_and_wiper_min_active_time_judge_timer_config` | `washer_and_wiper_min_active_time_judge_timer_` | 最低清洗计时器 | "清洗至少持续X秒了吗？" |

#### 8.3 开关状态 = 电平/边沿

| 枚举值 | 电平类比 | 含义 |
|--------|---------|------|
| `OFF` | 低电平 | 一直松着 |
| `ON` | 高电平 | 一直按着 |
| `OFF_ON` | **上升沿** ↑ | 这一周期刚按下 |
| `ON_OFF` | **下降沿** ↓ | 这一周期刚松开 |

#### 8.4 状态机（3个状态）

```cpp
// .h 文件中的 inner class
class wiper_operate_state {
  enum Type {
    NOT_ACTIVE,                // 未动作（待机）
    WIPER_ACTIVE,              // 仅雨刷动作中
    WASHER_AND_WIPER_ACTIVE    // 清洗+雨刷动作中
  };
};
```

状态→输出映射表：

| 状态 | wiper（雨刷） | washer（清洗） |
|------|:---:|:---:|
| NOT_ACTIVE | OFF | OFF |
| WIPER_ACTIVE | **ON** | OFF |
| WASHER_AND_WIPER_ACTIVE | OFF | **ON** |

#### 8.5 私有内部状态

| 变量 | 类型 | 含义 |
|------|------|------|
| `wiper_operate_state_` | `enum Type` | 当前状态机状态 |
| `is_wiper_sw_turned_off_` | `bool` | 开关"曾经松开过"标志（单次触发，一旦true永不回false） |

#### 8.6 扩展点：protected virtual

```cpp
protected:
  virtual bool IsWiperSwAvailable() const;  // 基类返回true，子类可override
```

用途：如果 ENGM 或 PDC 版本需要额外条件（比如"发动机运转时才允许雨刷"），子类只需 override 这一个函数。

#### 8.7 构造函数详解（.cc L9~L34）

```cpp
Function::Function(const Config& config, ...)
    : config_(config),                           // 保存配置引用
      is_sw_abnormal_(is_sw_abnormal),           // 保存输入信号引用
      is_wiper_control_available_(is_wiper_control_available),
      wiper_sw_state_(wiper_sw_state),
      washer_relay_order_(OFF, VALID),           // 输出初始化=OFF
      wiper_relay_order_(OFF, VALID),            // 输出初始化=OFF
      washer_and_wiper_min_active_time_judge_timer_(config...), // 4个计时器创建
      washer_and_wiper_sw_order_timer_(config...),
      wiper_finish_timer_(config...),
      wiper_sw_off_judge_timer_(config...),
      is_wiper_sw_turned_off_(false),            // "已松开"标志=false
      wiper_operate_state_(NOT_ACTIVE) {         // 初始状态=待机
  manager.RegistModule(*this);                   // 注册到Manager
}
```

开机时：输出全OFF，状态NOT_ACTIVE，标志false，4个计时器从0开始。

#### 8.8 Update() 完整执行流程 — 三相状态机

Update() 每个周期被 Manager 调用一次。执行顺序固定为以下6步：

---

**【第1步】全局计时器更新（.cc L38~L39）**

```cpp
wiper_sw_off_judge_timer_.Update();          // 松开计时器 +sampling_time
washer_and_wiper_sw_order_timer_.Update();   // 按住计时器 +sampling_time
```

这2个计时器是**全局计时器**，不管当前在哪个状态，每周期都 +sampling_time。
另外2个（`wiper_finish_timer_`、`washer_and_wiper_min_active_time_judge_timer_`）是**状态内计时器**，只在特定状态的 do 部分更新。

> **注意**：IncrementTimer.Update() 每次 **+sampling_time**（不是+1）。
> 如果 sampling_time=10，则 time_: 0→10→20→30→...
> 当 time_ >= time_up_time_ 时，IsTimeUp() 返回 true。

---

**【第2步】根据开关状态清除计时器（.cc L41~L53）**

```cpp
// 读取当前开关状态
EnumSwitchMomentaryState::Type wiper_sw_state = wiper_sw_state_.GetValue();

if ((wiper_sw_state == OFF) || (wiper_sw_state == ON_OFF)) {
    washer_and_wiper_sw_order_timer_.Clear();   // 没按着→清零"按住计时器"
} else if ((wiper_sw_state == ON) || (wiper_sw_state == OFF_ON)) {
    wiper_sw_off_judge_timer_.Clear();          // 按着→清零"松开计时器"
}
```

对称关系：

| 开关状态 | 第1步 Update 的 | 第2步 Clear 的 | 净效果 |
|---------|----------------|---------------|-------|
| OFF / ON_OFF（没按） | 两个都+sampling_time | `按住计时器` 归零 | 只有`松开计时器`在累积 |
| ON / OFF_ON（按着） | 两个都+sampling_time | `松开计时器` 归零 | 只有`按住计时器`在累积 |

> 效果：松开计时器只有在开关**持续处于 OFF** 时才会累积到超时；按住计时器只有在开关**持续处于 ON** 时才会累积到超时。

---

**【第3步】更新 is_wiper_sw_turned_off_ 标志（.cc L55~L60）**

```cpp
if (!is_wiper_sw_turned_off_) {              // 还没确认过？
    if (wiper_sw_off_judge_timer_.IsTimeUp()) {  // 松开计时器超时了？
        is_wiper_sw_turned_off_ = true;      // 确认：开关正常
    }
}
```

**单次触发标志（one-shot flag）**：变成 true 后永远不会回到 false。
作用：开机后，开关必须处于 OFF 状态够久，才允许进入清洗模式。
防止：开机时开关卡在 ON（硬件故障）→ 清洗器自动启动 → 危险。

---

**【第4步】三相状态机 — transition / entry / do（.cc L62~L126）**

先保存旧状态快照：
```cpp
const wiper_operate_state::Type pre_wiper_operate_state = wiper_operate_state_;
```

**Phase A: transition（状态迁移判定）**

```
NOT_ACTIVE:
  ├─ 开关=ON_OFF（下降沿=松手）→ 跳到 WIPER_ACTIVE          [剧情1]
  └─ is_wiper_sw_turned_off_ && 按住计时器超时 → 跳到 WASHER_AND_WIPER_ACTIVE  [剧情2]
      （两个条件缺一不可）

WIPER_ACTIVE:
  └─ 雨刷完成计时器超时 → 回到 NOT_ACTIVE

WASHER_AND_WIPER_ACTIVE:
  └─ 最低清洗计时器超时 && 松开计时器超时 → 回到 NOT_ACTIVE
      （两个条件缺一不可：喷够久 + 松手够久）
```

> 注意：NOT_ACTIVE 的 if/else if 顺序 = 优先级。ON_OFF（剧情1）优先于长按（剧情2）。

**Phase B: entry（进入新状态时的一次性动作）**

```cpp
if (wiper_operate_state_ != pre_wiper_operate_state) {  // 状态确实变了！
    case wiper_active:
        wiper_finish_timer_.Clear();                     // 清零雨刷完成计时器
    case washer_and_wiper_active:
        washer_and_wiper_min_active_time_judge_timer_.Clear();  // 清零最低清洗计时器
    case not_active:
        // Do Nothing
}
```

entry 只在状态切换那**一个周期**执行。
为什么要 Clear？进入新状态时计时器可能有旧数据，必须从0开始。

**Phase C: do（状态内每周期持续执行的动作）**

```cpp
case wiper_active:
    wiper_finish_timer_.Update();                        // 雨刷完成计时器 +sampling_time
case washer_and_wiper_active:
    washer_and_wiper_min_active_time_judge_timer_.Update();  // 最低清洗计时器 +sampling_time
case not_active:
    // Do Nothing
```

do 每个周期都执行（只要还在这个状态）。

**entry 和 do 的分工（教室比喻）**：
- entry = 走进教室那一刻：开灯、擦黑板（只做一次）
- do = 坐下后每分钟做的事：听课、记笔记（一直做）
- 你**不会每分钟都重新开灯**（=不能每次都Clear），也**不会只听一分钟课就不听了**（=不能只Update一次）
- 如果把 Clear 放在 do 里：每周期先归零再+1，计时器永远只有一格，永远不会超时 → 雨刷永远停不下来
- 如果把 Update 放在 entry 里：计时器只走了一格就再也不动 → 也永远到不了超时

---

**【第5步】根据状态决定输出值（.cc L128~L154）**

```cpp
switch (wiper_operate_state_) {
    case not_active:      wiper = off;  washer = off;   // 全关
    case wiper_active:    wiper = on;   washer = off;   // 只开雨刷
    case washer_and_wiper_active: wiper = off; washer = on; // 只开清洗
    default:              wiper = off;  washer = off;   // 安全兜底
}
```

---

**【第6步】安全保护覆盖（.cc L156~L159）**

```cpp
if ((!IsWiperSwAvailable()) || (!is_wiper_control_available_.GetValue())
    || (is_sw_abnormal_.GetValue())) {
    wiper_relay_order = off;
    washer_relay_order = off;
}
```

**不管前5步算出什么结果**，三个条件任一成立就强制全关：

| 条件 | 含义 | 场景 |
|------|------|------|
| `!IsWiperSwAvailable()` | 子类说"不可用" | 基类永远true，子类override时触发 |
| `!is_wiper_control_available_` | 外部系统说"禁止" | 控制器特殊模式 |
| `is_sw_abnormal_` | 开关硬件故障 | 诊断模块检测到线路异常 |

**关键设计**：安全层**不改变状态**，只改变**输出**。
状态机继续在"后台"正常运转（计时器继续跑、状态继续迁移），安全层只在最后一刻拦截输出。
等故障消除时，如果状态机还在 WIPER_ACTIVE → 雨刷立刻恢复。如果计时器已经跑完 → 自然停止。

---

**【最后】写入输出信号对象（.cc L161~L165）**

```cpp
wiper_relay_order_ = ElectricalOnOffState(wiper_relay_order, VALID);
washer_relay_order_ = ElectricalOnOffState(washer_relay_order, VALID);
```

打包成信号对象（值 + VALID），外部模块通过 Ref() 方法读取。

#### 8.9 剧情1 完整时序模拟（快速点按）

```
周期0: 开关=OFF, 状态=NOT_ACTIVE
  → 全局计时器+1, 按住计时器被Clear, 输出=全OFF

周期N: 开关=OFF_ON (↑ 按下)
  → 松开计时器被Clear, transition检查ON_OFF? 不是 → 不跳转

周期N+1: 开关=ON (按着)
  → 松开计时器被Clear, 不跳转

周期N+2: 开关=ON_OFF (↓ 松手) ⭐
  → transition: NOT_ACTIVE + ON_OFF → 跳到 WIPER_ACTIVE
  → entry: wiper_finish_timer_.Clear()（归零）
  → do: wiper_finish_timer_.Update()（0→10）
  → 输出: 雨刷ON, 清洗OFF

周期N+3~N+X: 开关=OFF, 状态=WIPER_ACTIVE
  → do: wiper_finish_timer_ 持续累积 (20→30→40→...)
  → 输出: 雨刷ON

周期N+X: wiper_finish_timer_ 超时
  → transition: 回到 NOT_ACTIVE
  → 输出: 全OFF → 雨刷停了
```

#### 8.10 剧情2 完整时序模拟（长按清洗）

```
前提: is_wiper_sw_turned_off_ = true（开机后已确认开关正常）

周期N: 开关=OFF_ON (↑ 按下)
  → 按住计时器开始累积, 松开计时器被Clear

周期N+1~N+X: 开关=ON (一直按着)
  → 按住计时器持续累积: 10→20→30→...
  → 松开计时器每次被Clear（因为在按着）

周期N+X: 按住计时器超时 ⭐
  → transition: is_wiper_sw_turned_off_=true ✅ + 按住计时器超时 ✅
  → 跳到 WASHER_AND_WIPER_ACTIVE
  → entry: washer_and_wiper_min_active_time_judge_timer_.Clear()
  → do: 最低清洗计时器 +sampling_time
  → 输出: 清洗ON

继续按着: 
  → 松开计时器每次被Clear → 退出条件永远不满足 → 一直喷水
  （操作员按着=一直清洗，这就是控制权）

周期M: 开关=ON_OFF (↓ 松手)
  → 松开计时器不再被Clear，开始累积

周期M+Y: 松开计时器超时 + 最低清洗计时器超时（两个都满足）⭐
  → transition: 回到 NOT_ACTIVE
  → 输出: 全OFF → 清洗停了

退出需要两个条件同时满足的原因:
  - 最低清洗计时器: 防止"刚喷就停"（水喷出来还没擦就停了）
  - 松开计时器: 尊重操作员意图（松手=想停）
```

#### 8.11 剧情3：安全保护

```
场景: 状态机在 WIPER_ACTIVE（雨刷正在刷），突然 is_sw_abnormal_ 变 true

这个周期:
  ① transition: 状态不变（计时器还没到）
  ② entry: 跳过（状态没变）
  ③ do: wiper_finish_timer_.Update()（继续计时）
  ④ 算输出: WIPER_ACTIVE → wiper=ON
  ⑤ 安全检查: is_sw_abnormal_=true → 强制 wiper=OFF ← 最后一刻拦截！
  ⑥ 写入SO: 雨刷不动

状态机在后台照常运转，安全层只拦截输出。
故障消除后：如果状态还在 WIPER_ACTIVE → 雨刷立刻恢复。
```

#### 8.12 4个计时器的分类

| 计时器 | 全局/状态内 | 在哪里Update | 在哪里Clear |
|--------|-----------|-------------|------------|
| `wiper_sw_off_judge_timer_`（松开） | **全局** | Update开头L38 | 开关=ON/OFF_ON时L48 |
| `washer_and_wiper_sw_order_timer_`（按住） | **全局** | Update开头L39 | 开关=OFF/ON_OFF时L45 |
| `wiper_finish_timer_`（雨刷完成） | **状态内** | do部分L120（仅WIPER_ACTIVE） | entry部分L105（进入WIPER_ACTIVE时） |
| `washer_and_wiper_min_active_time_judge_timer_`（最低清洗） | **状态内** | do部分L123（仅WASHER_AND_WIPER_ACTIVE） | entry部分L108（进入WASHER_AND_WIPER_ACTIVE时） |

全局计时器：每周期都跑，通过Clear控制"什么时候有效累积"
状态内计时器：只在对应状态里才跑，entry归零+do累加

#### 8.13 IncrementTimer 源码要点（复习 + 补充）

```
位置: Utility/1_0_0/timer/increment_timer.h/.cc
（[project]/Development/Utility/ 和 [project]/Utility/ 是同一份代码的拷贝）
```

**Config 结构体**（3个参数）：
```cpp
struct Config {
  uint32 sampling_time;   // 每次Update加多少（如10ms）
  uint32 initial_time;    // 初始时间（通常0）
  uint32 time_up_time;    // 超时阈值（如500ms）
};
```

**核心方法**：
```cpp
void Update() {
  // 溢出保护：快到uint32最大值时封顶，不会绕回0
  if (time_ > (MAX - sampling_time)) time_ = MAX;
  else time_ = time_ + sampling_time;    // ← +sampling_time，不是+1！

  // 判断是否超时
  if (time_ >= time_up_time_) {
    time_up_ = true;                     // IsTimeUp() 返回 true（电平）
    if (刚刚超过) time_up_now_ = true;   // IsTimeUpNow() 返回 true（边沿，只有一个周期）
  } else {
    time_up_ = false;
  }
}

void Clear() {
  time_ = 0;             // 归零
  time_up_ = false;      // 未超时
  time_up_now_ = false;
}
```

**为什么有 `time_up_time_` 成员变量（和 config_.time_up_time 重复）？**
因为 `SetTimeUpTime(newVal)` 方法允许运行时**动态修改阈值**。
config_.time_up_time 是"出厂设定"（const&，不能改），time_up_time_ 是"当前正在用的值"。

**IsTimeUp vs IsTimeUpNow**：
- `IsTimeUp()` = "到了吗？" → 到了之后一直返回 true（**电平**，像 ON）
- `IsTimeUpNow()` = "是不是**刚刚到**？" → 只在超过阈值那一个周期返回 true（**边沿**，像 OFF_ON）

#### 8.14 新学到的模式和概念

1. **三相状态机（transition / entry / do）** — 比 Module 7 的纯 switch-case 更规范
   - transition：判断是否要跳转
   - entry：跳转后做一次性初始化（只执行一次）
   - do：在当前状态持续执行（每周期执行）
   - 用 `pre_wiper_operate_state != wiper_operate_state_` 判断"是否刚刚发生了状态切换"
2. **全局计时器 vs 状态内计时器** — 同一个模块内，不同计时器有不同的生命周期
3. **单次触发标志（one-shot flag）** — `is_wiper_sw_turned_off_`，变 true 后永不回 false
4. **安全层 override** — 最后一步强制覆盖输出，不干预状态机逻辑
5. **protected virtual 扩展点** — 基类提供默认实现，子类按需覆盖
6. **松手检测（release detection）** — 检测 ON_OFF（下降沿）而非 OFF_ON（上升沿），因为要区分点按和长按
7. **双条件退出门控** — 进入 WASHER_AND_WIPER_ACTIVE 后需要"最低时间 + 松手"两个条件同时满足才能退出
8. **Module 7→Module 8 串联** — switch_momentary_convert 的输出就是 wiper_control 的输入
9. **输出查表用本地变量暂存** — 先赋值给本地变量 `wiper_relay_order`（无下划线），安全检查后才写入成员变量 `wiper_relay_order_`（有下划线），保证成员变量每周期只写入一次最终确定值
10. **硬件联动：washer_relay = 喷水+雨刷** — WASHER_AND_WIPER_ACTIVE 状态只开 washer_relay 而不开 wiper_relay，因为硬件电路上 washer 继电器同时驱动喷水泵和雨刷电机（并联），两个都开会导致双路供电/电流过大
11. **两条平行路径，不是串联** — 从 NOT_ACTIVE 出发，点按(ON_OFF)→WIPER_ACTIVE 和 长按→WASHER_AND_WIPER_ACTIVE 是两条独立路径，不会先经过雨刷再进入清洗
12. **建筑机械无连续雨刷** — 每次点按只刷一次自动停，和家用车的低速/高速连续雨刷不同。原因：使用场景（偶尔擦一下）、操作环境（震动大，简单按钮最耐用）、清洗更常见（工地脏污）
13. **pre_ 快照 = 状态边沿检测** — `pre_wiper_operate_state` 的作用和 Module 7 的 OFF_ON 完全一样，都是边沿检测思路，只不过检测的是"状态刚刚变了"而非"按钮刚按下"
14. **entry 和 do 无时间损失** — 两者在同一次 Update() 调用中连续执行（同一个周期内的前后两行代码），CPU只差几纳秒，远小于 sampling_time（10ms）的百万分之一

#### 8.15 April 13 深入Q&A — C++ 基础与嵌入式设计深化

> 以下内容来自对 Module 8 每一行代码的逐行追问，属于"看第二遍才能看到"的深层理解。

##### 8.15.1 构造函数参数的三种模式

```cpp
Function::Function(const Config& config, ..., Manager& manager)
    : config_(config),                              // 模式A：引用绑定
      is_sw_abnormal_(is_sw_abnormal),              // 模式A：引用绑定
      wiper_finish_timer_(config.wiper_finish_timer_config),  // 模式B：对象创建
      is_wiper_sw_turned_off_(false),               // 模式C：值初始化
      wiper_operate_state_(NOT_ACTIVE) {            // 模式C：值初始化
```

| 模式 | 写法 | 含义 | 适用于 |
|------|------|------|--------|
| **A. 引用绑定** | `config_(config)` | "看别人的"——只保存别名，不复制 | const & 成员（输入信号、配置） |
| **B. 对象创建** | `timer_(config.xxx)` | "造自己的"——用参数构造新对象 | 有状态的工具（IncrementTimer） |
| **C. 值初始化** | `flag_(false)` | "设初始值"——简单赋值 | bool、enum 等基本类型 |

##### 8.15.2 const & = "看别人的" vs 非const非& = "自己的"

```cpp
private:
  // ── "看别人的"（const &）📖 ──
  const Config& config_;                           // 只读引用，数据在外面
  const BoolSignal& is_sw_abnormal_;               // 只读引用，数据在外面
  const SwitchMomentaryState& wiper_sw_state_;     // 只读引用，数据在外面

  // ── "自己的"（非const非&）🔧 ──
  ElectricalOnOffState washer_relay_order_;         // 自己的输出，可以写
  IncrementTimer wiper_finish_timer_;               // 自己的计时器，可以改
  bool is_wiper_sw_turned_off_;                     // 自己的标志，可以改
```

**记忆口诀**：
- `const &` = 玻璃窗后面的展示品，只能看不能摸（数据属于别人）
- 没有 `const` 也没有 `&` = 自己口袋里的东西，随便用（数据属于自己）

##### 8.15.3 Config vs IncrementTimer 的本质区别

| | Config | IncrementTimer |
|---|---|---|
| **本质** | 一张纸条（3个数字） | 一个工具（有状态+有方法） |
| **是否有方法** | ❌ 纯数据 struct | ✅ Update(), Clear(), IsTimeUp()... |
| **是否有状态** | ❌ 数值固定不变 | ✅ time_ 每周期在变化 |
| **存储方式** | `const Config&` 只读引用 | `IncrementTimer timer_` 独立对象 |
| **为什么不放Config里** | — | Config 是 const&，不能调 Update() 改状态 |
| **类比** | 订购单（写着"30秒后响"） | 真正的厨房计时器（在滴答走） |

**关键理解**：Config 是告诉 IncrementTimer "你应该怎么配置自己"的参数。
IncrementTimer 拿到 Config 后就变成独立的工作工具，和 Config 再无直接关系。

##### 8.15.4 两个不同的 Type 枚举（项目约定）

```cpp
// 在 wiper_control_model_function.h 内部
class wiper_operate_state {
  enum Type { NOT_ACTIVE, WIPER_ACTIVE, WASHER_AND_WIPER_ACTIVE };
};

// 在信号对象定义中（另一个文件）
class EnumSwitchMomentaryState {
  enum Type { OFF, OFF_ON, ON, ON_OFF };
};
```

两个完全不同的类，碰巧都有一个叫 `Type` 的内部枚举。
- `wiper_operate_state::Type` = 雨刷的运行状态（3个值）
- `EnumSwitchMomentaryState::Type` = 开关的瞬时状态（4个值）

**项目约定**：所有枚举 class 的内部枚举都叫 `Type`。
通过 `类名::Type` 的前缀区分 → 这就是 inner class 做命名空间隔离的好处。

##### 8.15.5 inner class 用于命名空间隔离

```cpp
// wiper_operate_state 是 Function 类的 private inner class
class Function {
 private:
  class wiper_operate_state {    // ← inner class
    enum Type { ... };
  };
};
```

**为什么不用全局枚举？**
- 全局枚举 `NOT_ACTIVE` 可能和其他模块的 `NOT_ACTIVE` 冲突
- 放在 inner class 里 → `wiper_operate_state::NOT_ACTIVE`
- 带前缀的全名，不会和别的模块撞名
- **类比**：小区名（wiper_operate_state）+ 门牌号（NOT_ACTIVE）= 不会寄错

##### 8.15.6 变量名倒着读的技巧

```
is_wiper_sw_turned_off_
↓  ↓        ↓       ↓
是 雨刷开关 已经变成 OFF(关)
= "雨刷开关已经关掉了吗？" = "开关曾经松开过吗？"
```

**注意**：`off` 不是"按下"，是"关/松开"！
很多人第一眼看成"开关被打开"→ 其实是"开关被关掉（松开）"。
嵌入式代码中变量名就是最好的注释，但要一个词一个词拆开读。

##### 8.15.7 历史标志 vs 当前计时器：不矛盾

```cpp
// 进入清洗的条件（看起来矛盾但不矛盾）：
if (is_wiper_sw_turned_off_ && washer_and_wiper_sw_order_timer_.IsTimeUp())
```

| 条件 | 含义 | 时间点 |
|------|------|--------|
| `is_wiper_sw_turned_off_` | 开关**曾经**松开过 | **过去** — 历史记录 |
| `washer_and_wiper_sw_order_timer_.IsTimeUp()` | 开关**现在**按住够久了 | **现在** — 实时状态 |

**完整故事**：
1. 开机时开关处于 OFF → "松开计时器"累积到超时 → `is_wiper_sw_turned_off_ = true`（记录"开关曾经正常松开过"）
2. 后来操作员按住开关 → "按住计时器"开始累积
3. 按住够久 → `washer_and_wiper_sw_order_timer_.IsTimeUp() = true`
4. 两个条件都满足 → 进入清洗模式

**不矛盾**：一个是"过去发生过的事"（flag保持true），另一个是"现在正在发生的事"（计时器刚超时）。

##### 8.15.8 ON_OFF（松手检测）的设计理由

**为什么检测松手（ON_OFF ↓）而不是按下（OFF_ON ↑）？**

```
时间线：
  按下(OFF_ON) ─── 按着(ON) ─── 松手(ON_OFF)
       ↑                              ↑
   按多久还不知道                   到这里才知道是"快按"还是"长按"
```

如果在 OFF_ON（按下瞬间）就判断 → 不知道操作员打算按多久 → 不知道该启动雨刷还是清洗。
等到 ON_OFF（松手瞬间）→ 此时已经知道按了多久（按住计时器有没有超时） → 能正确分流。

**但是**：代码实际上两条路径的触发时机不同：
- 点按（剧情1）：等到 ON_OFF（松手）才触发 WIPER_ACTIVE
- 长按（剧情2）：不等松手，按住够久就直接触发 WASHER_AND_WIPER_ACTIVE

这正是"松手检测"和"按住计时"两条独立路径的精妙之处。

##### 8.15.9 星形拓扑状态机（Star Topology）

```
                 ┌────────────────────┐
                 │     NOT_ACTIVE     │  ← 中央枢纽
                 └──┬──────────────┬──┘
                    │              │
                    ↓              ↓
          ┌─────────────┐  ┌──────────────────────┐
          │WIPER_ACTIVE │  │WASHER_AND_WIPER_ACTIVE│
          └──────┬──────┘  └──────────┬───────────┘
                 │                    │
                 └────────────────────┘
                    ↑ 全部回到中央 ↑
```

**特征**：所有状态只能从 NOT_ACTIVE 出发，最终只能回到 NOT_ACTIVE。
没有 WIPER_ACTIVE ↔ WASHER_AND_WIPER_ACTIVE 的直接跳转。

**为什么不允许直接跳转？三个理由：**

| 理由 | 说明 |
|------|------|
| **1. 不需要** | 使用场景上，操作员不会"正在刷雨刷的时候突然要清洗" |
| **2. 不安全** | 直接跳转需要处理"旧状态的计时器怎么办"，增加出错风险 |
| **3. 更复杂** | 直接跳转 = 5条迁移路径（3+2），星形 = 4条（2+2），更简单 |

**设计原则**：**不需要 + 不安全 + 更复杂 = 不做**。这是工业控制系统的黄金法则。

##### 8.15.10 Manager 循环调用机制（10ms 轮询）

```
[RTOS]（操作系统）
    │
    │ 每 10ms 调用一次
    ↓
user_main()                          ← UserMainTask.cc
    │
    ├── system_state.Update()        ← 更新系统状态
    │
    └── main_task_control.UpdateWithSystemStateTransition()
            │
            └── main_task_manager.Update()    ← Manager 树根节点
                    │
                    ├── special_manager.Update()
                    │       ├── module_A.Update()
                    │       └── module_B.Update()
                    │
                    ├── main_app_manager.Update()
                    │       ├── node00_01.Update()
                    │       │       ├── wiper_control.Update()  ← 我们的 Module 8
                    │       │       └── ...
                    │       └── node00_02.Update()
                    │               └── ...
                    │
                    └── ... 其他 Manager 分支
```

**关键代码（UserMainTask.cc）**：
```cpp
void user_main() {                   // OS 每 10ms 调用
    system_state.Update();
    main_task_control.UpdateWithSystemStateTransition();
}
```

**Manager::Update() 内部（manager.h）**：
```cpp
void Manager::Update() {
    for (每个注册的 module : module_array_) {
        module.Update();             // 按注册顺序依次调用
    }
}
```

没有 `.cc` 源码（Manager 已编译进 Middleware 库），但从 `.h` 声明可以确认逻辑。

##### 8.15.11 Polling（轮询）= "没有真正的等待"

**嵌入式的核心理解**：代码里没有"等待"，只有"检查后跳过"。

```
周期1: Update() → timer还没到 → 什么都不做 → return（花了几微秒）
周期2: Update() → timer还没到 → 什么都不做 → return
周期3: Update() → timer还没到 → 什么都不做 → return
...
周期50: Update() → timer到了！→ 执行状态切换 → return
```

每次 Update() 都完整运行并 return，CPU 不会"停在那里等"。
所谓的"等500ms才算停" = timer 跑了 50 个周期（50 × 10ms = 500ms），每次检查都说"还没到"。

**类比**：
- ❌ 不是：站在门口等快递员来（阻塞等待）
- ✅ 是：每天出门看一次信箱，有就拿走没有就回家（轮询）

这就是 **Cooperative Scheduling（协作式调度）** 的核心：
每个模块保证自己快速执行完并返回，不能卡住整个系统。

##### 8.15.12 WASHER_AND_WIPER_ACTIVE 只开 washer_relay 的硬件原因

```
状态 WASHER_AND_WIPER_ACTIVE:
  wiper_relay = OFF  ← 为什么不开雨刷？
  washer_relay = ON  ← 只开清洗？
```

**硬件电路设计**：washer 继电器在物理上同时控制两个设备（并联）：
```
washer_relay = ON
    ├── 喷水泵（pump）  → 喷水
    └── 雨刷电机（wiper motor）  → 刷雨刷
```

所以只需要开 washer_relay，就能同时喷水+刷雨刷。
如果同时开 wiper_relay 和 washer_relay → 雨刷电机收到**双路供电** → 可能电流过大损坏。

**不是代码bug，是正确的硬件控制逻辑。**

### 模块9：shift_change_model_function（换挡计数器） ⭐新概念：模块复用 + MDO非易失性存储 + 边沿检测

> **文件位置**：
> - 头文件：`module/wlh/include/shift_change_model_function.h`
> - 实现：`module/wlh/src/shift_change_model_function.cc`
> - 命名空间：`logic::wl::vehicle_observer::shift_change_model`

#### 9.1 模块功能概述

**不是控制换挡，而是记录换挡次数。**

```
为什么需要计数？
├── 变速箱是机械系统 → 换挡次数 ≈ 磨损程度
├── 寿命管理：工程师需要知道"这台推土机换了多少次挡"
├── 保养预测：到一定次数 → 需要维护
└── 数据存在MDO（非易失性存储）→ 关机重启不丢失

类比：手机步数计数器
├── 每走一步 → +1          每换一次挡 → +1
├── 存在手机里 → 关机不丢    存在MDO里 → 关机不丢
└── APP里看步数             工程师用诊断工具读换挡次数
```

#### 9.2 变速箱挡位（EnumTmState）

定义在 `C:\SAIGYO\Application\1_14_0\signal_object\include\tm_state.h`（不在user_x里）

```
6个稳定挡位 + 3个过渡状态：

前进：FL(低) → FM(中) → FH(高)
倒退：RL(低) → RM(中)
空挡：NN

过渡态（换挡中的中间状态）：
├── FLFM_DIRECT → FL↔FM 直连过渡中
├── FMFH_DIRECT → FM↔FH 直连过渡中
└── RLRM_DIRECT → RL↔RM 直连过渡中

输入信号叫 tm_state_WITHOUT_direct → 去掉了过渡态
→ 只看稳定挡位 → 换挡完成后才计数
```

#### 9.3 输入/输出/配置

**3个输入**：
| 变量名 | 类型 | 含义 |
|--------|------|------|
| `config_` | `Config` | 配置：我负责监视哪个源挡位 |
| `tm_state_without_direct_` | `TmState` | 当前变速箱挡位（去掉DIRECT过渡态） |
| `shift_counter_mdo_` | `Uint32ArrayMemoryData&` | ★MDO数组（6格，非const → 可写） |

**Config结构体**：只有1个字段
```cpp
struct Config {
    signal_object::EnumTmState::Type record_target_tm_state_before_shift;
    // "我负责监视从哪个挡位换走的情况"
    // 例：如果值是FL → 这个实例只数"从FL换走"的次数
};
```

**无输出方法**：这个模块不提供 OutputRef()，它的"输出"是写到MDO里的数据。

**私有成员**：
| 变量名 | 类型 | 含义 |
|--------|------|------|
| `previous_tm_state_` | `EnumTmState::Type` | 上一次Update时的挡位（边沿检测用） |

#### 9.4 核心逻辑 — Update() 逐行

```cpp
void Function::Update() {
  // 条件1：挡位变了吗？（当前 ≠ 上次）
  // 条件2：是从我负责的挡位换走的吗？（上次 == config里的值）
  if ((tm_state_without_direct_.GetValue() != previous_tm_state_)
      && (previous_tm_state_ == config_.record_target_tm_state_before_shift)) {

    // 确定换到了哪个挡位 → 对应MDO数组的第几格
    size_t record_index;
    switch (tm_state_without_direct_.GetValue()) {
      case EnumTmState::NN: record_index = 0; break;  // 换到空挡
      case EnumTmState::FL: record_index = 1; break;  // 换到前进低
      case EnumTmState::FM: record_index = 2; break;  // 换到前进中
      case EnumTmState::FH: record_index = 3; break;  // 换到前进高
      case EnumTmState::RL: record_index = 4; break;  // 换到倒退低
      case EnumTmState::RM: record_index = 5; break;  // 换到倒退中
      default:              record_index = 0; break;
    }

    // 读出来 → +1（但别溢出）→ 写回去
    if (shift_counter_mdo_.GetValue(record_index) < UINT_MAX) {
      record_count = shift_counter_mdo_.GetValue(record_index) + 1;
    } else {
      record_count = shift_counter_mdo_.GetValue(record_index); // 到顶了不加
    }
    shift_counter_mdo_.SetValue(record_count, record_index);
  }

  // 记住当前挡位 → 下次Update时变成"上一次挡位"
  previous_tm_state_ = tm_state_without_direct_.GetValue();
}
```

#### 9.5 溢出保护详解

```
UINT_MAX = 4,294,967,295（uint32的最大值）

正常情况（99.99%的时间）：
  当前值 1053 < 4294967295 → +1 → 变成1054

极端情况（推土机跑了100年）：
  当前值 4294967295 < 4294967295？ → 不！等于！
  → 不+1，原样写回 → 停在最大值

为什么？
  如果不保护：4294967295 + 1 = 0（uint32溢出归零）
  → 100年的数据全部丢失
  
类比：汽车里程表到999999公里 → 不再转 → 不会归零
```

#### 9.6 六个实例 = 一个类用六次（模块复用）

config_app.h 中定义了6套配置：
```
shift_change_from_n_model_config       = { NN }  → 监视从空挡换走
shift_change_from_fwd_lo_model_config  = { FL }  → 监视从前进低速换走
shift_change_from_fwd_mid_model_config = { FM }  → 监视从前进中速换走
shift_change_from_fwd_hi_model_config  = { FH }  → 监视从前进高速换走
shift_change_from_rvs_lo_model_config  = { RL }  → 监视从倒退低速换走
shift_change_from_rvs_mid_model_config = { RM }  → 监视从倒退中速换走
```

接线文件创建了6个实例，全部用同一个类 + 同一个输入信号，只有Config和MDO不同：
```
所有6个实例都挂在 main_app_manager_node01_27
所有6个实例的tm_state都来自 tm_state_convert_function.TmStateWithoutDirectRef()
每个实例有自己独立的MDO数组（6格）

6个实例 × 每个6格 = 36个计数器 = 完整的"从X换到Y"统计表
```

#### 9.7 完整走一遍举例

```
场景：操作员从 前进低速(FL) 拨到 前进中速(FM)
当前实例：shift_change_from_fwd_lo_model（config = FL）

Update #1（挡位还是FL）：
  当前FL ≠ 上次FL？ → 不！一样 → 什么都不做

Update #2（操作员刚拨了挡杆，挡位变成FM）：
  当前FM ≠ 上次FL？ → 是！挡位变了！
  上次FL == config里的FL？ → 是！归我管！
  当前FM → switch → record_index = 2
  MDO[2]的值 = 1053 → 1053 < UINT_MAX → 1053+1 = 1054
  写回MDO[2] = 1054
  previous = FM

Update #3（挡位还是FM）：
  当前FM ≠ 上次FM？ → 不！→ 什么都不做

结果：FL→FM的计数从1053变成1054。永久存储，关机不丢。
同时其他5个实例也收到Update但都不触发（config不匹配）。
```

#### 9.8 新概念总结

| # | 新概念 | 说明 |
|---|--------|------|
| 1 | **模块复用** | 同一个类创建6个实例，只有Config不同。参数化复用 = 一份代码解决6种情况 |
| 2 | **MDO（Memory Data Object）** | 非易失性存储。写到Flash/EEPROM → 掉电不丢失。vs signal_object存在RAM里掉电就没了 |
| 3 | **Uint32ArrayMemoryData** | MDO的一种：uint32类型的数组。用GetValue(index)/SetValue(val, index)读写 |
| 4 | **MDO的定义位置** | Middleware层：`C:\SAIGYO\Middleware\1_3_0-S6\memory_data\uint32_array_memory_data.h` |
| 5 | **MDO是typedef** | `typedef ArrayMemoryDataObjectInterface<uint32> Uint32ArrayMemoryData` → 模板填空：T=uint32 |
| 6 | **纯虚接口** | `virtual T GetValue(size_t index) const = 0` → 只定义方法签名，实现在更底层（连接硬件） |
| 7 | **边沿检测模式** | previous_tm_state_ 记住上次状态 → 对比当前状态 → 只在变化瞬间触发一次 |
| 8 | **溢出保护** | `if (value < UINT_MAX)` → 到最大值就不再+1 → 防止归零 |
| 9 | **无输出方法的模块** | 这个模块不提供OutputRef() → "输出"是写入MDO → 外部通过诊断工具读取 |
| 10 | **非const引用参数** | `Uint32ArrayMemoryData&`（无const）→ 模块可以写入MDO。之前的信号输入都是`const &`只读 |

#### 9.9 和之前模块的对比

| | shift_change（换挡计数）| vehicle_completely_stop（停车判断）| wiper_control（雨刷）|
|---|---|---|---|
| 核心功能 | 记录换挡次数 | 判断是否停车 | 控制雨刷/清洗 |
| 有无Config | 有（1个字段） | 有（2个字段） | 有（4个计时器） |
| 内部状态 | previous_tm_state_ | timer_ | 4个timer_ + wiper_mode_ |
| 输出方式 | ★写MDO（无OutputRef） | signal_object(OutputRef) | signal_object(OutputRef) |
| 实例数 | ★6个（模块复用） | 1个 | 1个 |
| 数据持久性 | ★掉电不丢失 | 掉电丢失 | 掉电丢失 |

---

### 模块10：clutch_engagement_model_function（离合器接合品质监视） ⭐新概念：门控标志(Gate/Door)机制 + 直方图分桶 + ErrorSnap触发 + 物理离合器共用

> **核心一句话**：每次换挡瞬间，测量离合器接合时的转速差，分桶计数存MDO，转速差过大时触发ErrorSnap报警。

#### 10.1 模块定位

```
文件位置：
  头文件：module/wlh/include/clutch_engagement_model_function.h
  源文件：module/wlh/src/clutch_engagement_model_function.cc

功能：监视离合器在换挡瞬间的"接合品质"
  → 换挡时离合器的相对转速差（relative angular speed）反映接合是否平滑
  → 转速差越大 → 接合越粗暴 → 可能导致离合器磨损/损坏
  → 极端情况（转速差 ≥ 450rpm）→ 触发 ErrorSnap（异常快照记录）

类比：
  就像医院里的心率监护仪 —— 平时只记录数据存档，
  但一旦检测到心率异常冲高，就自动触发紧急报警+记录当时的完整数据快照。
```

#### 10.2 Config 结构体（9个字段）

```cpp
struct Config {
  uint32   record_thresh_clutch_off_time;         // 离合器断开时间阈值（单位：tick，即10ms）
  float32  record_thresh_tm_oil_temp;             // 变速箱油温阈值（单位：°C）
  float32  record_thresh_clutch_relative_rev[5];  // 直方图分桶边界（5个值划分4个桶）
  EnumTmState::Type record_target_tm_state_before_shift;  // 换挡前的目标挡位
  EnumTmState::Type record_target_tm_state_after_shift;   // 换挡后的目标挡位
};
```

**逐个字段解读：**

| # | 字段 | 类型 | 实际值 | 含义 |
|---|------|------|--------|------|
| 1 | record_thresh_clutch_off_time | uint32 | 50U | 离合器必须断开≥50个tick（500ms）才算"充分断开"。防止短暂断开的假信号 |
| 2 | record_thresh_tm_oil_temp | float32 | 50.F | 油温必须≥50°C。冷机时离合器行为不同，不纳入统计 |
| 3 | record_thresh_clutch_relative_rev[0] | float32 | 0.F | 桶0下界：0 rpm |
| 4 | record_thresh_clutch_relative_rev[1] | float32 | 150.F | 桶0上界/桶1下界：150 rpm |
| 5 | record_thresh_clutch_relative_rev[2] | float32 | 300.F | 桶1上界/桶2下界：300 rpm |
| 6 | record_thresh_clutch_relative_rev[3] | float32 | 450.F | 桶2上界/桶3下界：450 rpm |
| 7 | record_thresh_clutch_relative_rev[4] | float32 | FLT_MAX | 桶3上界：无上限（FLT_MAX ≈ 3.4×10³⁸） |
| 8 | record_target_tm_state_before_shift | EnumTmState | 因实例而异 | 这个实例监视的"换挡起点" |
| 9 | record_target_tm_state_after_shift | EnumTmState | 因实例而异 | 这个实例监视的"换挡终点" |

**直方图分桶示意：**
```
转速差(rpm):  0        150       300       450       ∞(FLT_MAX)
              |---桶0---|---桶1---|---桶2---|---桶3---|
              | 正常    | 轻微    | 较大    | 严重    |
              | 平滑接合| 稍有冲击| 明显冲击| ★触发报警|
```

**为什么用 FLT_MAX 而不是某个具体数字？**
```
FLT_MAX = float32 能表示的最大值 ≈ 3.4 × 10^38
意思是"桶3没有上限" —— 不管转速差多大，都落入桶3
这是一种防御性编程：不会有任何值落在所有桶之外
```

#### 10.3 核心成员变量

```cpp
private:
  EnumTmState::Type previous_tm_state_;                              // 上一拍的挡位（值拷贝，快照）
  bool is_interval_cond_satisfied_;                                   // 门控标志（开关/门）
  const uint32& clutch_off_time_;                                     // 离合器断开时间（const引用，活窗口）
  const float32& tm_oil_temperature_;                                 // 变速箱油温（const引用，活窗口）
  Uint32ArrayMemoryData& clutch_engagement_counter_mdo_;             // MDO数组（4个桶的计数器）
  const EnumTmState::Type& tm_state_for_taget_engagement_clutch_;    // 当前挡位（const引用，活窗口）← 注意原码有typo: taget应为target
  const float32& clutch_relative_angular_speed_;                      // 离合器相对转速差（const引用）
  const Config& config_;                                              // 配置参数
  bool is_snap_trigger_on_;                                           // ErrorSnap触发标志
```

**关键区分：值拷贝 vs 引用绑定**

```
构造函数：
  previous_tm_state_(obj.GetValue())     ← .GetValue() 返回值 → 值拷贝
                                            这是一个"快照"，之后和原始信号脱钩
                                            → 用来做"上一拍记忆"
  
  tm_state_for_taget_engagement_clutch_(obj) ← 引用绑定，不调用.GetValue()
                                                这是一个"活窗口"，始终看到最新值
                                                → 用来看"当前这一拍是什么挡位"

记忆方法：
  GetValue() → 拍照（拍完照片和真人无关了）
  直接引用  → 监控摄像头（实时看到现场）
```

#### 10.4 is_interval_cond_satisfied_ —— 门控标志机制（本模块最难的部分）

```
这个bool变量的行为像一扇"门"：

  门关着 (false) → 等待离合器断开足够久，如果满足条件就开门
  门开着 (true)  → 等待检测到目标换挡完成，检测到了就关门

核心代码（简化版）：
```

```cpp
// === 开门逻辑 ===
if (!is_interval_cond_satisfied_) {
    // 门关着 → 检查开门条件
    if (clutch_off_time_ >= config_.record_thresh_clutch_off_time) {
        is_interval_cond_satisfied_ = true;   // 开门！
    }
}

// === 关门逻辑 ===
if (previous_tm_state_ == config_.record_target_tm_state_after_shift) {
    is_interval_cond_satisfied_ = false;      // 关门！
}
```

**为什么需要这个门？—— 防虚假换挡信号**

```
问题场景：
  换挡过程中，挡位信号可能出现短暂抖动/中间状态
  比如 FL → FLFM_DIRECT → FM，中间的FLFM_DIRECT是过渡状态
  
  如果没有门控：
    FL → FLFM_DIRECT  → 检测到"挡变了！" → 误触发记录
    FLFM_DIRECT → FM  → 又检测到"挡变了！" → 再次误触发
    
  门控的作用：
    只有在离合器断开足够长时间(≥500ms)后才开门
    → 短暂的过渡状态不会满足条件 → 过滤掉虚假换挡
    
类比：
  就像银行保险箱的双重门 ——
  外门（时间条件）：你必须等保安验证完身份（离合器充分断开）
  内门（挡位条件）：验证完才能进去办事（检测换挡）
  办完事门自动关上（换挡完成 → 关门 → 等下一次）
```

**为什么关门条件是 `previous == after_shift` 而不是 `previous != current`？**

```
假设用 previous != current：
  时间线：FL(稳定) → FLFM_DIRECT(过渡) → FM(到达)
  
  在 FLFM_DIRECT 这一拍：
    previous = FL, current = FLFM_DIRECT
    previous != current → true → 关门！
    → 门刚开就被关上了！还没来得及看到 FM 就关了！
    
用 previous == after_shift（例如 FM）：
  FL拍：  previous=某个值, 不是FM → 不关门
  FLFM拍：previous=FL → 不是FM → 不关门（好！门还开着）
  FM拍：  previous=FLFM_DIRECT → 不是FM → 不关门（四条件门通过，完成记录！）
  下一拍： previous=FM → 是FM！ → 关门！
  
  → 门精确地在"换挡被记录后的下一拍"关闭
  → 过渡状态不会提前关门
```

#### 10.5 "拍"（tick/cycle）概念

```
每次调用 Update() 就是"一拍"
系统每 10ms 调用一次 → 1秒 = 100拍

Update() 的最后一行：
  previous_tm_state_ = tm_state_for_taget_engagement_clutch_;
  // 把"当前挡位"拍照存为"上一拍的挡位"
  // → 下次Update()进来时，previous就是上一拍的值了

这实现了"一拍延迟"的边沿检测：
  拍N：   current = FL,   previous = FL   → 没变化
  拍N+1： current = FM,   previous = FL   → 变了！→ 检测到换挡边沿
  拍N+2： current = FM,   previous = FM   → 没变化了
```

#### 10.6 Update() 全流程详解

```
Update() 每 10ms 被调用一次，完整逻辑如下：

┌─────────────────────────────────────────────┐
│ 步骤1：门控标志更新                            │
│                                             │
│ if (!is_interval_cond_satisfied_)           │
│   if (off_time >= 阈值)  → 开门              │
│                                             │
│ if (previous == after_shift) → 关门          │
└──────────────────────┬──────────────────────┘
                       ↓
┌─────────────────────────────────────────────┐
│ 步骤2：四条件门（全部同时满足才通过）              │
│                                             │
│ ① 油温 ≥ 50°C              （排除冷机）       │
│ ② is_interval_cond_satisfied_ == true（门开着）│
│ ③ previous == before_shift   （上一拍是起点挡）  │
│ ④ current == after_shift     （当前是终点挡）   │
│                                             │
│ 99.99%的时间：条件不满足 → 跳到最后一步          │
└──────────────────────┬──────────────────────┘
                       ↓ （极少触发）
┌─────────────────────────────────────────────┐
│ 步骤3：转速单位变换                            │
│                                             │
│ record_value = |相对转速差| × 60 / (2π)       │
│   → 从 rad/s 转换为 rpm                      │
│   → fabs() 取绝对值（不关心旋转方向）            │
└──────────────────────┬──────────────────────┘
                       ↓
┌─────────────────────────────────────────────┐
│ 步骤4：直方图分桶                              │
│                                             │
│ for i = 0 to 3:                             │
│   if (阈值[i] ≤ record_value < 阈值[i+1])    │
│     record_index = i                         │
│     break                                    │
│                                             │
│ 分桶结果：                                    │
│   桶0：[0, 150)    rpm → 正常               │
│   桶1：[150, 300)  rpm → 轻微               │
│   桶2：[300, 450)  rpm → 较大               │
│   桶3：[450, ∞)    rpm → 严重 ★触发ErrorSnap │
└──────────────────────┬──────────────────────┘
                       ↓
┌─────────────────────────────────────────────┐
│ 步骤5：MDO计数器递增                           │
│                                             │
│ value = MDO.GetValue(record_index)          │
│ if (value < UINT_MAX)   // 溢出保护           │
│   MDO.SetValue(value + 1, record_index)     │
│                                             │
│ MDO = 非易失性存储 → 掉电不丢失               │
│ UINT_MAX保护 → 到最大值不再+1 → 防归零         │
└──────────────────────┬──────────────────────┘
                       ↓
┌─────────────────────────────────────────────┐
│ 步骤6：ErrorSnap触发判断                       │
│                                             │
│ if (record_index == 桶数-1)   // 即桶3        │
│   is_snap_trigger_on_ = true  // 触发报警     │
│ else                                         │
│   is_snap_trigger_on_ = false                │
│                                             │
│ 只有最严重的桶（≥450rpm）才触发ErrorSnap        │
│ → 记录当时的完整系统快照用于事后分析             │
└──────────────────────┬──────────────────────┘
                       ↓
┌─────────────────────────────────────────────┐
│ 步骤7：更新previous（每拍都执行）               │
│                                             │
│ previous_tm_state_ = current_tm_state       │
│ → 为下一拍的边沿检测做准备                      │
└─────────────────────────────────────────────┘
```

#### 10.7 唯一输出：IsSnapTriggerOnRef()

```cpp
const bool& IsSnapTriggerOnRef() const {
    return is_snap_trigger_on_;
}
```

```
这是整个模块唯一的对外输出方法。

输出什么？
  → is_snap_trigger_on_ 的 const 引用
  → true = 这一拍检测到严重的离合器接合（转速差 ≥ 450rpm）
  → false = 其他所有情况

谁在用这个输出？
  → instantiation_error_snap_module.h
  → 6个实例的 IsSnapTriggerOnRef() 分别接到 ErrorSnap 模块
  → ErrorSnap 收到 true 时会触发快照记录

注意：与 shift_change 模块不同
  shift_change → 无输出方法，只写MDO
  clutch_engagement → 有输出方法(IsSnapTriggerOnRef)，同时也写MDO
  → 这个模块是"双输出"：MDO统计计数 + ErrorSnap实时告警
```

#### 10.8 六个实例的配置（config_app.h）

```
config_app.h 中定义了6套配置，每套监视一个特定的换挡方向：

实例1: fwd_mid_to_fwd_lo     → before=FM, after=FL   → 前进中速→前进低速（降挡）
实例2: rvs_lo_to_rvs_mid     → before=RL, after=RM   → 倒退低速→倒退中速（升挡）
实例3: fwd_lo_to_fwd_mid     → before=FL, after=FM   → 前进低速→前进中速（升挡）
实例4: fwd_hi_to_fwd_mid     → before=FH, after=FM   → 前进高速→前进中速（降挡）
实例5: fwd_mid_to_fwd_hi     → before=FM, after=FH   → 前进中速→前进高速（升挡）
实例6: rvs_mid_to_rvs_lo     → before=RM, after=RL   → 倒退中速→倒退低速（降挡）

所有6个实例的阈值参数完全相同：
  off_time = 50U (500ms)
  oil_temp = 50.F (50°C)
  rpm bins = {0, 150, 300, 450, FLT_MAX}
  
区别只在 before_shift 和 after_shift 的挡位组合不同。
```

#### 10.9 接线详解 —— 物理离合器共用（★关键领域知识）

```
接线文件：instantiation_app_userland.h（约28240-28317行）

6个实例分布在两个manager节点上：
  main_app_manager_node01_28：实例1-4
  main_app_manager_node01_29：实例5-6

每个实例的输入信号：
  tm_state:     tm_state_convert_function.TmStateForTagetEngagementClutchRef()
  oil_temp:     tm_oil_temp.TemperatureRef()
  off_time:     ???_clutch_off_time.OffTimeRef()     ← 离合器断开时间
  relative_rev: ???_clutch.ClutchRelativeAngularSpeedRef() ← 离合器相对转速

其中 off_time 和 relative_rev 的来源——离合器选择——乍看很困惑：

  fwd_mid_to_fwd_lo  (FM→FL): FlrmRef()   ← FLRM离合器
  rvs_lo_to_rvs_mid  (RL→RM): FlrmRef()   ← FLRM离合器
  fwd_lo_to_fwd_mid  (FL→FM): FmRef()     ← FM离合器
  fwd_hi_to_fwd_mid  (FH→FM): FmRef()     ← FM离合器
  fwd_mid_to_fwd_hi  (FM→FH): FhRef()     ← FH离合器
  rvs_mid_to_rvs_lo  (RM→RL): RlRef()     ← RL离合器

为什么 FM→FL 用的不是FL离合器而是FLRM离合器？

这涉及行星齿轮变速箱的物理设计：
```

```
╔══════════════════════════════════════════════════╗
║           行星齿轮变速箱的离合器布局                ║
╠══════════════════════════════════════════════════╣
║                                                  ║
║  物理离合器：  控制的挡位：                          ║
║  ─────────   ──────────                          ║
║  FLRM离合器 → FL（前进低速）+ RM（倒退中速）         ║
║  FM离合器   → FM（前进中速）                        ║
║  FH离合器   → FH（前进高速）                        ║
║  RL离合器   → RL（倒退低速）                        ║
║                                                  ║
║  关键发现：FL和RM共用一个物理离合器！                 ║
║  → 所以代码里叫 FLRM，不是 FL 或 RM               ║
║  → 这是因为行星齿轮组的机械结构特性                   ║
║  → 同一组离合器片执行不同方向时产生不同挡位            ║
║                                                  ║
║  换挡时"接合"的是目标挡位的离合器：                   ║
║  FM→FL：目标是FL → FL的离合器接合 → FlrmRef()      ║
║  RL→RM：目标是RM → RM的离合器接合 → FlrmRef()      ║
║  FL→FM：目标是FM → FM的离合器接合 → FmRef()        ║
║  FH→FM：目标是FM → FM的离合器接合 → FmRef()        ║
║  FM→FH：目标是FH → FH的离合器接合 → FhRef()        ║
║  RM→RL：目标是RL → RL的离合器接合 → RlRef()        ║
║                                                  ║
╚══════════════════════════════════════════════════╝

规律：监视的是"目标挡位"对应的离合器，不是"起始挡位"的。
因为换挡 = 释放旧离合器 + 接合新离合器。
我们要监视的是"接合品质" → 当然看新离合器！
```

#### 10.9.1 行星齿轮变速箱基础知识（深挖背景）

> 理解 FLRM 共用离合器的物理原因，需要先理解行星齿轮变速箱的结构。

##### 基本结构：一个行星齿轮组有4个核心部件

```
        ┌─────────────────────┐
        │    Ring Gear 齿圈    │  ← 最外圈的内齿轮（齿在内壁上）
        │  ┌───────────────┐  │
        │  │  ○   ○   ○    │  │  ← Planet Gears 行星轮（通常3~4个）
        │  │    ╲ │ ╱      │  │
        │  │  ───●───      │  │  ← Sun Gear 太阳轮（中心小齿轮）
        │  │    ╱ │ ╲      │  │
        │  │  ○   ○   ○    │  │
        │  └───────────────┘  │
        └─────────────────────┘
              ↑
          Carrier 行星架（连接所有行星轮的框架）

齿圈是"内齿轮"：齿长在圆环内壁上
  普通齿轮：齿在外面       齿圈：齿在里面
      ╱╲╱╲              ┌──────────────┐
     ╱    ╲             │╲╱╲╱╲╱╲╱╲╱╲╱│
    │      │            │              │
     ╲    ╱             │╱╲╱╲╱╲╱╲╱╲╱╲│
      ╲╱╲╱              └──────────────┘
    外齿轮                  内齿轮

因为齿圈直径最大 → 齿数Zr总是大于太阳轮齿数Zs
```

##### 核心原理：固定不同部件 → 不同齿比

三个部件中，一个输入，一个输出，第三个被离合器/制动器锁住：

```
╔════════════╦═══════════════════╦═══════════════════════╦══════════════╗
║  固定部件   ║ 输入 → 输出        ║ 速比                   ║ 效果         ║
╠════════════╬═══════════════════╬═══════════════════════╬══════════════╣
║ 齿圈固定   ║ 太阳轮 → 行星架    ║ Zs/(Zs+Zr) ≈ 0.3     ║ 同向+大幅减速 ║
║ 行星架固定  ║ 太阳轮 → 齿圈     ║ -Zs/Zr ≈ -0.43       ║ 反转+减速    ║
║ 太阳轮固定  ║ 齿圈 → 行星架     ║ Zr/(Zs+Zr) ≈ 0.7     ║ 同向+轻微减速 ║
║ 全部锁定   ║ 整体同转           ║ 1.0                   ║ 直连（1:1）  ║
╚════════════╩═══════════════════╩═══════════════════════╩══════════════╝
（数值示例：Zs=30齿, Zr=70齿）

注意：输入输出反接则可以加速（小驱大=减速，大驱小=加速）
工程机械不用加速挡 — 要的是大扭矩（铲土/推土），不是高转速。
最快挡（FH）也就是全锁定1:1直连。超速挡(overdrive)是乘用车省油才用的。
```

##### 什么是齿轮的"加速/减速"？（避免和车辆加减速混淆）

```
★重要区分：两个完全不同的"加减速"★

① 齿轮的加速/减速 = 输出轴比输入轴转得快还是慢
   输入1000rpm → [齿轮组] → 输出300rpm  → 齿轮"减速"
   输入1000rpm → [齿轮组] → 输出3000rpm → 齿轮"加速"
   跟车速无关，只看两根轴的转速比。

② 车辆的加速/减速 = 车速变快还是变慢
   踩油门 = 车辆加速；踩刹车 = 车辆减速

两者完全独立：
  装载机挂FL低速挡（齿轮"减速"配置）→ 踩油门 → 车照样在加速
  只是齿轮帮你"用转速换力气"（rpm降低，扭矩放大）
```

##### 齿轮减速/加速的本质 = 大轮带小轮 vs 小轮带大轮

```
小齿轮(20齿) 驱动 大齿轮(60齿)：
    ○  →  ◯        小的转1圈 → 大的只走 20/60 = 1/3圈
   20齿   60齿      输出更慢 → 减速 → 但力矩变大3倍（省力）

大齿轮(60齿) 驱动 小齿轮(20齿)：
    ◯  →  ○        大的转1圈 → 小的走 60/20 = 3圈
   60齿   20齿      输出更快 → 加速 → 但力矩变小到1/3（费力）

记忆口诀：小驱大 → 减速大力 | 大驱小 → 加速小力

自行车类比：
  前盘(小) → 后飞轮(大) = 低速挡：踩很多圈走一点，但爬坡有力
  前盘(大) → 后飞轮(小) = 高速挡：踩几圈走很远，但爬坡没力

"爬坡有力/没力"说的是车轮推地面的力（输出端），不是脚上的力（输入端）
实际感受正好相反：
  低速挡：脚踩着轻松（每脚力小）但要踩很多圈 → 车轮力大 → 爬得上
  高速挡：脚踩着沉重（每脚力大）但蹬不动    → 车轮力不够 → 爬不上
本质 = 杠杆原理：低速挡 = 长杠杆（移动距离大但省力）
```

##### 为什么齿圈固定时是减速？（以此为例理解能量分流）

```
齿圈固定（不能动），太阳轮转动（输入）

太阳轮带动行星轮"自转"
→ 但行星轮同时和齿圈啮合
→ 齿圈不能动 → 行星轮只能沿齿圈内壁"滚动爬行"
→ 行星轮一边自转，一边绕太阳轮公转
→ 行星架跟着公转运动 → 输出

关键：行星轮的运动被分成了两部分：
  太阳轮的旋转能量
      ├── 行星轮自转（在齿圈上打转，不输出）← 被"分流"了
      └── 行星轮公转（带动行星架转，输出）   ← 只有这部分有用

只有公转部分变成输出 → 输出转速 < 输入转速 → 减速

用数字：Zs=30, Zr=70
  n_carrier / n_sun = Zs / (Zs + Zr) = 30/100 = 0.3
  太阳轮1000rpm → 行星架300rpm → 减速到0.3倍

能量守恒 → 转速降低 → 扭矩必然增大：
  T_carrier = T_sun × (n_sun / n_carrier) = T_sun × 100/30 ≈ 3.3 × T_sun
  转速降到0.3倍，扭矩放大3.3倍 — 装载机低速挡需要的：慢速但力量大
```

##### 行星轮的公转和自转一定是同时发生的

```
这是被结构强制的，不是可选的。

原因：行星轮同时和太阳轮、齿圈两个东西啮合

不可能只公转不自转：
  如果行星轮像摩天轮车厢一样不转
  → 齿会和齿圈/太阳轮"硬刮"，齿数不对应
  → 物理上啮合不上 → 卡死

不可能只自转不公转：
  行星轮自转时同时推太阳轮和齿圈
  → 只要不是全锁死，就会产生力
  → 行星架被推着走 → 变成公转

类比：地球绕太阳公转(365天一圈) + 同时自转(24小时一圈)
区别：地球是惯性"碰巧"的，行星轮是齿啮合"强制"的。

分配比例由哪个部件被固定来决定：
  固定件小（太阳轮）→ 分流少 → 减速轻微(0.7)
  固定件大（齿圈）  → 分流大 → 减速严重(0.3)
```

##### 多组行星齿轮串联 → 装载机的6个挡位

```
实际变速箱 = 多组行星齿轮串联 + 多个离合器组合：

发动机 ──→ [行星齿轮组1] ──→ [行星齿轮组2] ──→ 输出轴
              ↑                    ↑
           离合器A              离合器B
           离合器C              离合器D

挡位由离合器接合的组合决定：

╔═══════╦══════╦══════╦══════╦══════╗
║ 挡位   ║ FLRM ║  FM  ║  FH  ║  RL  ║
╠═══════╬══════╬══════╬══════╬══════╣
║  FL   ║  ●   ║      ║      ║      ║  ← FLRM接合 + 前进方向
║  FM   ║      ║  ●   ║      ║      ║
║  FH   ║      ║      ║  ●   ║      ║
║  RL   ║      ║      ║      ║  ●   ║
║  RM   ║  ●   ║      ║      ║      ║  ← FLRM接合 + 倒退方向
║  NN   ║      ║      ║      ║      ║  ← 全部释放
╚═══════╩══════╩══════╩══════╩══════╝

FL和RM接合的是同一个物理离合器(FLRM)，
前进/倒退由另一个机构（方向离合器/其他行星组）控制。

类比：同一个灯泡开关(=FLRM离合器)
  白天开灯 → 照明(=FL) | 晚上开灯 → 夜灯(=RM)
  开关相同，上下文不同 → 功能不同
```

#### 10.10 ErrorSnap 输出接线

```
接线文件：instantiation_error_snap_module.h

6个实例的输出分别接到 ErrorSnap 模块的不同通道：
  第212行：clutch_engagement_fwd_mid_to_fwd_lo.IsSnapTriggerOnRef()
  第226行：clutch_engagement_rvs_lo_to_rvs_mid.IsSnapTriggerOnRef()
  第240行：clutch_engagement_fwd_lo_to_fwd_mid.IsSnapTriggerOnRef()
  第254行：clutch_engagement_fwd_hi_to_fwd_mid.IsSnapTriggerOnRef()
  第268行：clutch_engagement_fwd_mid_to_fwd_hi.IsSnapTriggerOnRef()
  第282行：clutch_engagement_rvs_mid_to_rvs_lo.IsSnapTriggerOnRef()

ErrorSnap 的作用：
  当任一实例触发（转速差 ≥ 450rpm）时，
  ErrorSnap会记录当时瞬间的完整系统状态快照：
  → 各传感器值、各模块状态、时间戳等
  → 存储到非易失性存储供售后诊断使用
  
类比：飞机的黑匣子 → 出事时自动快照记录
```

#### 10.11 EnumTmState 挡位枚举值

```
来自 tm_state.h：

稳定挡位（6个）：
  NN  空挡
  FL  前进低速
  FM  前进中速
  FH  前进高速
  RL  倒退低速
  RM  倒退中速

过渡挡位（3个）：
  FLFM_DIRECT  前进低速↔中速的过渡状态
  FMFH_DIRECT  前进中速↔高速的过渡状态
  RLRM_DIRECT  倒退低速↔中速的过渡状态

过渡挡位的存在正是门控标志设计的原因：
  如果没有门控，过渡状态会产生虚假的换挡边沿
  门控确保只在"充分断开后的真正换挡"时才记录
```

#### 10.12 与 shift_change 模块的对比

| | shift_change（换挡计数）| clutch_engagement（离合器接合品质）|
|---|---|---|
| 监视什么 | 换了几次挡 | 换挡时离合器转速差多大 |
| 触发条件 | 挡位变了 | 挡位变了 + 油温够高 + 门控通过 |
| 门控标志 | 无 | ★有（is_interval_cond_satisfied_） |
| 数据处理 | 直接计数 | ★先分桶再计数 |
| MDO | 6个桶（每个挡位一个） | 4个桶（每个转速区间一个） |
| 输出方法 | 无（只写MDO） | ★IsSnapTriggerOnRef()（ErrorSnap告警） |
| 实例数 | 6个（每个"从X挡换走"一个） | 6个（每个"从X挡换到Y挡"一对） |
| Config字段数 | 1个 | ★9个 |
| 复杂度 | ★简单（入门级） | ★★★中等偏上 |

#### 10.13 新概念总结

| # | 新概念 | 说明 |
|---|--------|------|
| 1 | **门控标志(Gate/Door)** | bool标志充当"门"：满足时间条件开门 → 满足完成条件关门。用来过滤噪声/虚假信号。比简单的`if`判断更精妙 |
| 2 | **直方图分桶(Histogram Binning)** | 把连续的数值范围切成若干区间（桶），统计每个区间的事件次数。工业界常用的数据分析方法 |
| 3 | **FLT_MAX 作为无穷大** | float32最大值(≈3.4×10³⁸)用作直方图最后一个桶的上界 = "无上限"。防御性编程，确保所有值都能分进某个桶 |
| 4 | **UINT_MAX 溢出保护** | 在+1之前检查是否已到uint32最大值(4,294,967,295)。防止溢出归零导致数据丢失 |
| 5 | **ErrorSnap触发** | 特定条件下设置bool标志 → 通过引用输出 → ErrorSnap模块读取 → 触发系统快照。用于事后故障分析 |
| 6 | **物理离合器共用(FLRM)** | FL和RM共享同一个物理离合器片组。行星齿轮变速箱的特性。代码里的命名(FlrmRef)反映了这个物理事实 |
| 7 | **双输出模式** | 同一个模块既写MDO（统计存储）又提供OutputRef（实时告警）。比shift_change多了一个输出维度 |
| 8 | **const引用 vs 值拷贝** | 构造函数中 `member_(obj)` = 引用绑定(活窗口)，`member_(obj.GetValue())` = 值拷贝(快照)。初始化列表里的微小差异导致完全不同的运行时行为 |
| 9 | **过渡挡位(DIRECT状态)** | FLFM_DIRECT等过渡状态是换挡过程中的中间状态。理解它们的存在是理解门控设计的前提 |
| 10 | **rad/s → rpm 转换** | `rpm = |rad_per_sec| × 60 / (2π)`。转速有两种单位制，代码中做了转换 |

#### 10.14 完整走一遍举例

```
场景：操作员从 前进低速(FL) 换到 前进中速(FM)
监视实例：clutch_engagement_fwd_lo_to_fwd_mid（config: before=FL, after=FM）
当前状态：门关着(false)，油温60°C，离合器已断开800ms(80 ticks)

═══ 拍1000（准备阶段，门还关着）═══
  门控检查：
    门关着 → 检查开门条件
    off_time(80) >= 阈值(50) → 满足！ → 开门！ is_interval_cond_satisfied_ = true
    previous(FL) == after_shift(FM)? → 不！ → 不关门
  
  四条件门：
    ① 油温60°C ≥ 50°C      ✓
    ② 门开着                 ✓
    ③ previous(FL) == FL     ✓
    ④ current(FL) == FM      ✗ ← 还没换挡呢！
    → 不通过，跳到最后

  更新：previous = FL

═══ 拍1001（操作员拨了挡杆，进入过渡状态）═══
  门控检查：
    门开着 → 跳过开门检查
    previous(FL) == FM? → 不！ → 不关门
  
  四条件门：
    ③ previous(FL) == FL     ✓
    ④ current(FLFM_DIRECT) == FM  ✗ ← 还在过渡！
    → 不通过

  更新：previous = FLFM_DIRECT

═══ 拍1002（到达目标挡位！）═══
  门控检查：
    门开着 → 跳过开门检查
    previous(FLFM_DIRECT) == FM? → 不！ → 不关门（好！门还开着）
  
  四条件门：
    ① 油温60°C ≥ 50°C                  ✓
    ② 门开着                             ✓
    ③ previous(FLFM_DIRECT) == FL        ✗ ← ！！！
    
    等等——条件③不通过？previous是FLFM_DIRECT不是FL啊！
    
    → 实际上这说明：只有 "前一拍恰好还在FL、这一拍直接跳到FM" 的情况才会触发
    → 如果中间经过了FLFM_DIRECT过渡态，这个条件确实不满足
    → 这是额外的滤波：只记录"干净的"瞬间换挡，过渡太慢的不记录

  更新：previous = FM

═══ 拍1003（换挡完成后的安定状态）═══
  门控检查：
    previous(FM) == FM? → 是！ → 关门！ is_interval_cond_satisfied_ = false
  
  四条件门：
    ② 门关着 → 不通过
  
  更新：previous = FM

═══ 另一种情况：快速换挡（无过渡状态）═══
  如果系统在一个tick内直接从FL跳到FM（无FLFM_DIRECT中间状态）：
  
  该拍：
    previous(FL) == FM? → 不！ → 不关门
    ① 油温 ✓  ② 门开 ✓  ③ previous(FL)==FL ✓  ④ current(FM)==FM ✓
    → 全部通过！
    
    转速差 = |相对角速度| × 60/(2π) = 比如 280rpm
    分桶：280 在 [150, 300) → 桶1
    MDO[1] 的值 5327 < UINT_MAX → 5327+1 = 5328
    桶1 ≠ 桶3 → is_snap_trigger_on_ = false
    
  下一拍：
    previous(FM) == FM → 关门
```

#### 10.15 设计哲学总结

```
clutch_engagement 的设计体现了几个嵌入式控制的核心原则：

1. 防御性编程：FLT_MAX确保覆盖全范围，UINT_MAX防溢出
2. 信号去抖：门控机制过滤虚假换挡信号
3. 关注点分离：统计(MDO) 和 告警(ErrorSnap) 分开处理
4. 参数化复用：6个实例，同一份代码，只有Config不同
5. 引用语义：const&实现零拷贝实时数据读取
6. 领域驱动：变量命名和逻辑设计直接反映物理机械结构(FLRM)
```

---

### 模块11：brake_lamp_model_function（刹车灯控制） ⭐新概念：迟滞比较器(Hysteresis) + 传感器降级 + 完整信号链路追溯

> **核心一句话**：三条制动信号各自经过迟滞比较器判断"是否在刹车"，再根据传感器健康状况选择信任哪条信号，最终决定刹车灯亮不亮。

#### 11.1 模块定位

```
文件位置：
  头文件：module/wlh/include/brake_lamp_model_function.h
  源文件：module/wlh/src/brake_lamp_model_function.cc

功能：控制刹车灯继电器的 ON/OFF
  → 操作员踩刹车 → 油压升高 → 超过阈值 → 刹车灯亮
  → 松开刹车 → 油压降低 → 低于另一个更低的阈值 → 刹车灯灭
  → 两个阈值不同 = 迟滞(Hysteresis) → 防止在边界反复闪烁

类比：
  空调控制：温度 > 26°C 才开冷气，温度 < 23°C 才关冷气
  24~26°C 之间：维持当前状态，不抖动
```

#### 11.2 新工具类：HysteresisFloat32（迟滞比较器） ⭐第2个重要utility工具

```
位置：utility/1_0_0/hysteresis/
  ├── hysteresis.h                  ← 模板类 Hysteresis<T>（Config/Update/GetState）
  ├── hysteresis_implementation.h   ← 实际比较逻辑（static方法）
  └── hysteresis.cc

typedef Hysteresis<float32> HysteresisFloat32;  ← 第110行

和 IncrementTimer 的对比（两大utility工具）：
  IncrementTimer：时间维度防抖 → "条件持续满足500ms了吗？"
  Hysteresis：    数值维度防抖 → "值超过阈值了吗？（开和关用不同阈值）"
```

**Hysteresis 内部结构：**

```cpp
template<typename T>
class Hysteresis {
  struct Config {
    T on_thresh;     // 开启阈值（值超过这个才开）
    T off_thresh;    // 关闭阈值（值低于这个才关）
  };
  
  // 构造：state_ = false（初始灭），阈值从Config读入
  // Update(new_data)：调用 HysteresisImplementation::Update() 更新 state_
  // GetState()：返回 state_（bool）
  
private:
  bool state_;       // 当前状态（true=激活, false=未激活）
  T on_thresh_;      // 开启阈值
  T off_thresh_;     // 关闭阈值
};
```

**HysteresisImplementation::Update() 核心逻辑：**

```
正常模式（on_thresh > off_thresh，如 0.59 > 0.392）：

  当前灭着(state=false)：
    值 > on_thresh(0.59)  → 开！state = true
    值 ≤ on_thresh(0.59)  → 继续灭

  当前亮着(state=true)：
    值 < off_thresh(0.392) → 关！state = false
    值 ≥ off_thresh(0.392) → 继续亮

  示意图：
    值:  0     0.392     0.59      1.0
         |-------|---------|---------|
         | 确定灭 | 死区    | 确定亮  |
                  |(维持现状)|
                  
  还支持反向模式（on_thresh < off_thresh），用于"值越小越该触发"的场景
```

#### 11.3 Config 结构体（3个字段）

```cpp
struct Config {
  utility::HysteresisFloat32::Config is_brake_press_active_config;                    // 刹车油压迟滞
  utility::HysteresisFloat32::Config is_brake_pedal_rate_active_config;               // 踏板速率迟滞
  utility::HysteresisFloat32::Config is_collision_detection_brake_press_active_config; // 碰撞检测油压迟滞
};
```

**config_app.h 第5837行的实际值：**

| 迟滞对象 | on_thresh（开灯） | off_thresh（灭灯） | 单位 |
|----------|-------------------|--------------------|----|
| 刹车油压 | 0.59F | 0.392F | MPa |
| 踏板速率 | 46.8F | 39.5F | 速率单位 |
| 碰撞检测油压 | 0.59F | 0.392F | MPa |

#### 11.4 七个输入信号

| # | 参数 | 类型 | 来自（接线文件） | 含义 |
|---|------|------|-----------------|------|
| 1 | brake_pedal | Rate | brake_stroke_recognize.OutputRef() | 刹车踏板速率 |
| 2 | brake_press | Pressure | brake_pressure_sensor.PressureRef() | 刹车油压 |
| 3 | collision_detection_brake_press | Pressure | ods_brake_pressure_sensor.PressureRef() | 碰撞检测刹车油压 |
| 4 | is_diag_normal_brake_press_sensor | BoolSignal | 诊断模块 | 油压传感器健康？ |
| 5 | is_diag_normal_brake_stroke_sensor | BoolSignal | 诊断模块 | 行程传感器健康？ |
| 6 | is_diag_normal_collision_detection_brake_sensor | BoolSignal | 诊断模块 | 碰撞检测传感器健康？ |
| 7 | manager | Manager | main_app_manager_node01_05 | 生命周期管理 |

#### 11.5 成员变量分类

```
Private 成员按类型分三类：

1. 引用成员（const &，活窗口，不拥有数据）：
   brake_press_                                    → 油压值
   brake_pedal_                                    → 踏板速率
   collision_detection_brake_press_                → 碰撞检测油压
   is_diag_normal_brake_press_sensor_              → 油压传感器健康？
   is_diag_normal_brake_stroke_sensor_             → 行程传感器健康？
   is_diag_normal_collision_detection_brake_sensor_→ 碰撞传感器健康？
   config_                                          → 配置参数

2. 工具对象（owned，自己创建的工具）：
   is_brake_press_active_                          → 油压迟滞比较器
   is_brake_pedal_rate_active_                     → 踏板速率迟滞比较器
   is_collision_detection_brake_press_active_      → 碰撞检测迟滞比较器

3. 输出成员（owned，自己创建的输出信号）：
   brake_lamp_relay_                               → 刹车灯继电器状态
```

**⚠️ 命名问题**：`is_brake_press_active_` 用了 `is_` 前缀，看起来像 bool，但实际是 HysteresisFloat32 对象。这不符合业界命名规范（Google C++ Style Guide：`is_` 前缀只给 bool）。读代码时要看类型声明，不能只看变量名。

#### 11.6 Update() 全流程详解

```
┌─────────────────────────────────────────────────────────┐
│ 第1段：三条信号各自做迟滞判断（第37-70行）                    │
│                                                         │
│ 油压 → Hysteresis(0.59/0.392) → brake_press_state(ON/OFF)        │
│ 踏板速率 → Hysteresis(46.8/39.5) → brake_pedal_state(ON/OFF)     │
│ 碰撞油压 → Hysteresis(0.59/0.392) →                              │
│             + 传感器健康？                                         │
│             ├─ 不健康 → 强制OFF（不信任坏传感器）                    │
│             └─ 健康 → 用迟滞结果 → collision_state(ON/OFF)         │
└───────────────────────────┬─────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│ 第2段：传感器降级选择（第72-81行）★首次看到降级代码         │
│                                                         │
│ 油压传感器健康？                                           │
│ ├─ 健康 → service_brake = brake_press_state（优先用油压）  │
│ └─ 坏了 → 行程传感器健康？                                  │
│           ├─ 健康 → service_brake = brake_pedal_state      │
│           │        （降级：退而求其次用踏板）                  │
│           └─ 也坏了 → service_brake = OFF                  │
│                     （都坏了→灭灯→安全侧）                   │
└───────────────────────────┬─────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│ 第3段：最终输出 = OR 合并（第83-93行）                      │
│                                                         │
│ service_brake_state == ON  OR  collision_state == ON     │
│ → 任一为ON → 刹车灯亮                                     │
│ → 都是OFF → 刹车灯灭                                      │
│                                                         │
│ 打包输出：                                                │
│ brake_lamp_relay_ = ElectricalOnOffState(output, VALID)  │
└─────────────────────────────────────────────────────────┘
```

**传感器降级策略的优先级：**
```
★ 油压传感器（最可靠，直接测量制动力）
  ↓ 坏了
★★ 踏板行程传感器（间接测量，但比没有好）
  ↓ 也坏了
★★★ OFF（宁可灯不亮也不乱亮 = 安全侧设计）
```

**碰撞检测是独立通道：**
```
碰撞检测不参与降级选择（它有自己独立的传感器健康检查）
与 service_brake 用 OR 合并 → 碰撞紧急制动时灯也会亮
```

#### 11.7 bool → enum 类型转换

```
Hysteresis.GetState() 返回 bool (true/false)
但下游需要 EnumElectricalOnOffState (ON/OFF)

所以代码手动转换：
  if (GetState())   → ON     // true  → 继电器通电
  else              → OFF    // false → 继电器断电

  true/false = 通用工具的语言（Hysteresis不知道自己在判断什么）
  ON/OFF    = 业务领域的语言（继电器、灯、阀门等电气设备）
```

#### 11.8 信号类型继承链（★解开了一个一直存在的谜团）

```
brake_press_.GetValue() 能调用，是因为：

  brake_lamp_model_function.h 第13行：  #include "pressure.h"
                                              ↓
  pressure.h 第10行：                   #include "numeric_signal.h"
    class Pressure : public NumericSignal<float32>
                                              ↓
  numeric_signal.h 第10行：             #include "signal_object.h"
    class NumericSignal<T> : public SignalObject<T>
                                              ↓
  signal_object.h 第28行：
    T GetValue() const { return value_; }    ← 就在这里！

  完整继承链：
  SignalObject<float32>  ← GetValue()在这定义
    ↑ 继承
  NumericSignal<float32> ← 中间层（目前是空壳）
    ↑ 继承
  Pressure               ← 你在代码里用的类型

  #include 是传递的：A include B，B include C → A 能看到 C
  但每个文件只 #include 自己直接用到的（规范做法）
```

所有信号类型都在同一个目录下：
```
Middleware/1_3_0-S6/signal_object/
├── signal_object.h           ← 基类模板（GetValue在这里）
├── numeric_signal.h          ← 数值信号中间层
├── pressure.h                ← 压力信号
├── rate.h                    ← 速率信号
├── bool_signal.h             ← 布尔信号
├── electrical_on_off_state.h ← 电气开关状态（ON/OFF枚举 + 信号对象typedef）
└── ...
```

#### 11.9 完整信号链路：从脚踩刹车到灯亮（★首次完整追溯）

```
┌─────────────────────────────────────────────────────────┐
│ 第1层：物理硬件                                           │
│  脚踩刹车 → 液压活塞 → 油管压力变化                        │
│  → 压力传感器 → 模拟电压信号(0~5V)                        │
└─────────────────────┬───────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ 第2层：OS/驱动层（KICSiOS）                               │
│  RH850芯片AD转换器 → GetAinVoltageFloat(channel)         │
│  → 返回 raw_voltage（如 2.34V）                          │
└─────────────────────┬───────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ 第3层：设备驱动层（PressureSensorByAnalogInput）          │
│  ① 低通滤波(LPF)：去除电气噪声                            │
│  ② 范围检查：电压太低(断线?)→LowFault / 太高(短路?)→HighFault │
│  ③ 插值查表：电压→压力（6点查表：x=电压, y=MPa）           │
│     → 如 2.34V → 0.72 MPa                               │
│  ④ 打包：Pressure(0.72, VALID)                           │
│  ⑤ 输出：PressureRef() → 返回内部 Pressure 对象的 const&  │
└─────────────────────┬───────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ 第4层：接线文件连接                                       │
│  instantiation_app_userland.h 第16346行：                 │
│  /* brake_press : */ brake_pressure_sensor.PressureRef() │
│  → 传给 brake_lamp_model 构造函数的 brake_press 参数       │
│  → .cc 第21行：brake_press_(brake_press) 引用绑定         │
│  → brake_press_ 从此是传感器内部 Pressure 对象的"别名"     │
└─────────────────────┬───────────────────────────────────┘
                      ↓
┌─────────────────────────────────────────────────────────┐
│ 第5层：应用层（brake_lamp_model_function.cc）             │
│  .cc 第38行：                                            │
│  is_brake_press_active_.Update(brake_press_.GetValue())  │
│  → GetValue() 穿过引用取出 0.72                          │
│  → Hysteresis: 0.72 > on_thresh(0.59) → state_ = true   │
│  → GetState() → true → brake_press_state = ON            │
│  → 降级选择 → OR合并 → brake_lamp_relay_ = ON             │
└─────────────────────────────────────────────────────────┘
```

#### 11.10 10ms调度链路：从OS定时器到你的模块（★首次完整追溯）

```
RH850硬件定时器中断（每10ms）
  → KICSiOS调用 user_main()                    [UserMainTask.cc:23]
    → main_task_control.UpdateWithSystemStateTransition()
      → main_task_manager.Update()              [Middleware层]
        → main_app_manager.Update()             [instantiation_app_userland.h:912]
          → main_app_manager_node01.Update()    [:14123]
            → node01_00.Update()                ← brake_pressure_sensor 先执行（读传感器）
            → ...
            → node01_05.Update()                [:16079]
              → brake_lamp_model.Update()       ← 后执行（拿到最新值判断）

证据来源：
  ① UserMainTask.h 中 ASCII时序图明确标注 "<-------10msec----->"
  ② config_app.h 中所有 sampling_time = 10U（定时器以10ms为单位计数）
  ③ node编号越小越先执行 → 保证数据生产者先于消费者
```

#### 11.11 .cs 与 .h 文件的镜像关系

```
tool/AppBuilder/Middlewares/utility/Hysteresis.cs  ← PC端（AppBuilder工具用）
utility/1_0_0/hysteresis/hysteresis.h              ← 芯片端（实际运行）

.cs 文件只有 Config 结构体（on_thresh, off_thresh），没有逻辑
.h 文件有 Config + Update + GetState 完整实现

两者字段完全一样，但没有 #include 关系
→ 是人工保持同步的"镜像"
→ .cs 通过 [CppType("utility", "hysteresis")] 标注对应关系
→ .cs 是 PC 端的影子，.h 是芯片端的本体
```

#### 11.12 新概念总结

| # | 新概念 | 说明 |
|---|--------|------|
| 1 | **迟滞比较器(Hysteresis)** | 开和关用不同阈值 → 中间有死区 → 防止边界状态反复触发。项目第2个重要utility（第1个是IncrementTimer） |
| 2 | **传感器降级(Sensor Fallback)** | 之前学过概念，首次看到实现代码。优先级：油压→踏板→OFF |
| 3 | **完整信号链路** | 首次追通从物理传感器→AD转换→滤波→查表→Pressure对象→const&引用→GetValue()→Hysteresis→ON/OFF的全链路 |
| 4 | **10ms调度链路** | 首次追通从OS定时器→user_main()→Manager树→模块.Update()的全链路 |
| 5 | **信号类型继承链** | Pressure→NumericSignal→SignalObject，GetValue()在最顶层基类定义。#include是传递的 |
| 6 | **bool→enum转换** | 工具层用bool(true/false)，业务层用enum(ON/OFF)，中间需要手动翻译 |
| 7 | **Buffer = 插值查表** | PressureSensorByAnalogInput的Buffer是电压→压力的6点查找表 |
| 8 | **命名不规范** | `is_` 前缀的变量不一定是bool，可能是工具对象。要看类型声明 |
| 9 | **.cs镜像** | AppBuilder的.cs文件是.h文件的PC端影子，字段相同但无直接关联 |

#### 11.13 与之前模块的对比

| | back_lamp（模块2）| brake_lamp（模块11）|
|---|---|---|
| 功能 | 倒车灯 | 刹车灯 |
| 输入数 | 4 | 7（多了传感器+诊断） |
| Config | 无 | 有（3个迟滞配置） |
| 内部状态 | 无 | 3个Hysteresis对象 |
| 传感器降级 | 无 | ★有（油压→踏板→OFF） |
| 逻辑类型 | 纯组合 | 组合+迟滞（有记忆） |
| 输出 | 1个 | 1个 |
| 复杂度 | ★简单 | ★★中等 |

| 工具对比 | IncrementTimer（模块5学的） | Hysteresis（模块11学的） |
|---------|---------------------------|------------------------|
| 位置 | utility/timer/ | utility/hysteresis/ |
| 防抖维度 | 时间（持续够久才算数） | 数值（开关阈值不同） |
| Config | sampling_time, time_up_time | on_thresh, off_thresh |
| 关键方法 | Update() + Clear() + IsTimeUp() | Update(val) + GetState() |
| 状态重置 | 需要显式 Clear() | 无Clear，靠值自然变化 |

---

## 十五·五、动手写代码：从读懂到写出来 / Hands-on Coding Practice

> 2026年4月17日。读了11个模块之后，第一次自己动手写嵌入式 C++ 代码。
> 核心体会：**读10个模块不如自己写1个模块。**

### 15.5.1 第一次编译操作

#### 改 config 值验证编译流程

```
操作：
1. 打开 config_app.h
2. 把 brake_lamp 的 on_thresh 从 0.59F 改成 0.6F
3. 在 app_logic/bin/ 下执行 make makelib
4. 编译通过 → 说明改数值字面量不影响编译（类型没变）
5. 改回 0.59F

学到的：
├── 改数值字面量（0.59→0.6）→ 类型没变（还是 float）→ 一定能编译通过
├── 编译器只检查类型匹配，不在乎具体数值
└── 改完记得改回去，这是真实项目的config
```

#### 改完 .h 后 make 说 "Nothing to be done"

```
原因：
├── make 通过文件时间戳判断是否需要重编
├── make 只追踪 .cc → .o 的依赖关系
├── 这个项目的 makefile 没有声明 .h 的依赖
├── 所以 .h 改了，make 不知道

解决：
├── make clean   → 删掉所有 .o
├── make makelib → 强制从头编译
└── 规则：改了 .h 就要 clean + rebuild

本质：
├── .cc 改了 → make 自动检测 ✅
├── .h 改了 → make 检测不到 ❌，需要手动 clean
├── .o 不存在 → 必须编译（clean 的原理）
```

### 15.5.2 第一个编译错误：中文注释断行

```
错误信息：
  clutch_engagement_model_function.cc, line 62: error #7: unrecognized token

原因：
  }) {  // rev_4 = FLT_MAX 的意思就是：只要转速差 ≥ 450rpm，不管多大都算 bin3。
   ，所以这等于没有上限。        ← 编译器以为这是代码！
                                   // 注释只对当前一行有效，换行后失效

修复：
  把注释合并到一行，一个 // 覆盖全部

教训：
  ├── // 注释只管当前行，换行必须重新加 //
  ├── Green Hills 编译器遇到 UTF-8 中文 → 不认识 → "unrecognized token"
  └── 写中文注释要特别注意不要断行
```

### 15.5.3 Make 编译原理

```
Make 的核心逻辑（一句话版）：
"如果目标文件比源文件旧，就重新生成。"

详细流程：
Step 1: 读 makefile → 建立依赖关系图
Step 2: 比较时间戳

  .o 比 .cc 新 → "不用重编" → "Nothing to be done"
  .cc 比 .o 新 → "需要重编" → 执行编译命令
  .o 不存在   → "必须编译" → 执行编译命令

为什么改 .h 不生效：
  makefile 只写了：
    %.o : %.cc         ← 只依赖 .cc
  没写：
    %.o : %.cc %.h     ← 应该也依赖 .h（但这个项目没做）

解决方法（3种）：
  ① make clean + make（暴力但可靠）        ← 我们用的
  ② 在 makefile 里声明 .h 依赖（优雅但项目没做）
  ③ 用 -MMD 让编译器自动生成依赖（现代做法）
```

### 15.5.4 fan_control 影子模块（AI辅助写的）

```
目标：模仿 brake_lamp 结构，写一个更简单的风扇控制模块
简化：3个传感器 → 1个温度传感器，3个迟滞 → 1个迟滞

文件结构：
practice/
├── include/
│   └── fan_control_model_function.h     ← 声明
├── src/
│   └── fan_control_model_function.cc    ← 实现
└── build.bat                            ← 编译脚本

编译过程遇到的问题：
├── 第1次：cannot open "manager.h"
│   原因：Middleware 头文件分散在子目录（framework/, signal_object/ 等）
│   修复：-I 路径要写到每个子目录，不是一个统一的 include/
│
├── 第2次：cannot open "array_container.h"
│   原因：utility 也一样，有20个子目录各自是 include 路径
│   修复：看 utility.mk 里的 UTILITY_INC_DIRS，把所有子目录加上
│
├── 第3次：PowerShell 命令行太长，编译器卡住
│   修复：写 .bat 脚本，用 set INC= 分段拼接 -I 路径
│
└── 第4次：BUILD SUCCESS ✅

关键认知：
├── 真实项目的编译配置从来不是一个简单的路径
├── -I（include path）是编译命令最重要的参数之一
├── 查看 .mk 文件就能知道项目怎么组织 include 路径
└── .bat 只是"装命令的容器"，核心是理解编译器参数
```

### 15.5.5 reverse_buzzer 独立模块（自己写的） ★

```
目标：不看 fan_control，自己从头写一个倒车蜂鸣器模块
场景：车速低于阈值 → 蜂鸣器响，传感器坏了 → 不响

自己犯过的错误（全部是编译前被指出）：

① 缺少 #include
   现象：只写了 namespace 和 class，但没 #include 任何头文件
   教训：编译器不认识任何类型，除非你 #include 了它的声明

② 构造函数参数缺逗号
   错误：...is_diag_normal_speed_sensor framework::Manager& manager)
   正确：...is_diag_normal_speed_sensor, framework::Manager& manager)
                                        ↑ 逗号

③ ElectricalOnOffStateType vs ElectricalOnOffState
   错误：const signal_object::ElectricalOnOffStateType& （enum 值类型）
   正确：const signal_object::ElectricalOnOffState&     （SignalObject 类型）
   教训：Type 后缀是 enum 值，不带 Type 是 SignalObject 包装

④ class 结尾缺分号
   错误：}
   正确：};      ← C++ class 必须以 }; 结尾

⑤ 缺少 #endif
   #ifndef 和 #endif 必须配对

⑥ HysteresisSint32 vs HysteresisFloat32（类型不匹配）
   错误：utility::HysteresisSint32（因为"车速"感觉像整数）
   正确：utility::HysteresisFloat32（因为 Velocity::GetValue() 返回 float32）
   教训：选 Hysteresis 类型 → 看输入信号的 GetValue() 返回什么类型
         不能凭直觉猜，要去看 .h 文件确认

⑦ RegistModule vs RegisterModule
   错误：manager.RegisterModule(*this);   ← 标准英语
   正确：manager.RegistModule(*this);     ← 项目实际用的（日式英语）
   教训：框架 API 的名字要查，不能凭英语直觉猜

⑧ Update() vs update()（大小写）
   错误：void Function::update()
   正确：void Function::Update()
   教训：C++ 区分大小写，必须和 .h 声明完全一致

⑨ GetState() vs GetValue()（Hysteresis 的方法）
   错误：is_velocity_normal_.GetValue()    ← Hysteresis 没有这个方法
   正确：is_velocity_normal_.GetState()    ← 返回 bool
   教训：Hysteresis 是"判断工具"不是"信号容器"
         SignalObject 有 GetValue()（取值）
         Hysteresis 有 GetState()（取判断结果）

⑩ 输出方法写在 Update() 里面
   错误：void Update() { ... ReverserBuzzerRelayRef() const { ... } }
   正确：ReverserBuzzerRelayRef() 是独立方法，在 Update() 外面
   教训：C++ 不允许在函数体内定义成员方法

⑪ .cc 里缺少 Function:: 前缀
   错误：const ... & ReverserBuzzerRelayRef() const
   正确：const ... & Function::ReverserBuzzerRelayRef() const
   教训：.cc 文件在 class 外面，编译器需要知道方法属于哪个类

⑫ EnumElectricalOnOffState vs ElectricalOnOffState（输出赋值）
   错误：reverser_buzzer_relay_ = signal_object::EnumElectricalOnOffState(output, ...)
   正确：reverser_buzzer_relay_ = signal_object::ElectricalOnOffState(output, ...)
   教训：Enum 是枚举类，ElectricalOnOffState 才是 SignalObject 类型
```

### 15.5.6 错误分类总结

```
语法类（C++ 基本功）：
├── 逗号、分号、大小写 → 多写就会减少
├── #include / #endif 配对 → 养成习惯
└── 函数不能嵌套定义 → C++ 规则

类型类（需要查源码确认）：
├── Hysteresis 用 Float32 还是 Sint32 → 看输入 GetValue() 返回类型
├── GetState() vs GetValue() → 看类的 .h 有哪些方法
├── EnumXxx vs Xxx → Enum 是裸枚举，不带 Enum 是 SignalObject
└── 规则：不确定就去看 .h

项目规范类（只能靠查不能靠猜）：
├── RegistModule vs RegisterModule → 框架 API 名字
├── Function:: 前缀 → .cc 里的规则
└── 这些错几次就记住了

编译环境类：
├── -I 路径必须覆盖所有头文件目录
├── Middleware/utility 各子目录分别是 include 路径
├── 改 .h 后要 make clean
└── .bat 脚本解决命令行过长问题
```

### 15.5.7 "读懂" vs "写得出" 的差距

```
读代码时觉得懂了的东西：           自己写时才发现不会的：
────────────────────             ────────────────────
"Config 在 .h 里定义"          →  忘了 #include hysteresis.h
"构造函数用初始化列表"          →  参数之间忘了加逗号
"Hysteresis 判断阈值"          →  不知道用 GetState 还是 GetValue
"Output 用 ElectricalOnOffState" →  分不清 Enum 前缀和 SignalObject 类型
".cc 里方法要加类名前缀"        →  忘了写 Function::

结论：
├── 读代码 = 知道"是什么"（理解）
├── 写代码 = 知道"怎么做"（技能）
├── 两者之间有巨大鸿沟
├── 这个鸿沟只有动手才能填
└── 犯错是最快的学习方式
```

### 15.5.8 parking_lamp 独立模块（纯记忆写的）★★

> 2026年4月21日。第二个独立写的模块。
> 挑战升级：关掉 Copilot + IntelliSense，不看任何参考代码，纯靠记忆。
> 从头写 .h + .cc + build.bat，一天完成并编译通过。

```
目标：驻车灯控制模块
场景：驻车压力 > 阈值 且 诊断正常 → 灯亮，否则灯灭
输入：Pressure（压力传感器）、BoolSignal（诊断状态）
输出：ElectricalOnOffState（继电器）
工具：HysteresisFloat32（迟滞判断）

文件结构：
parking_lamp_model_practice/
├── include/
│   └── parking_lamp_model_practice.h
├── src/
│   └── parking_lamp_model_practice.cc
└── build.bat
```

#### .h 编写过程中的错误

```
① class 继承写法混淆（:: vs :）
   错误：class Function::public framework::ModuleInterface
   正确：class Function : public framework::ModuleInterface
   教训：:: 是命名空间/类访问，: 是继承。两个符号完全不同

② const 引用声明位置错误
   错误：const& signal_object::ElectricalOnOffState&
   正确：const signal_object::ElectricalOnOffState&
   教训：const 修饰类型，& 跟在类型后面，不要多余的 &

③ ElectricalOnOffState vs EnumElectricalOnOffState 概念辨析
   问题：不理解两者的关系
   答案（三层类型链）：
     第1层：enum ElectricalOnOffStateType { OFF, ON }   ← 裸值
     第2层：class EnumElectricalOnOffState               ← 包装 enum 的容器类
     第3层：typedef SignalObject<...Type> ElectricalOnOffState  ← 带状态的信号对象
   规则：
     参数/成员/返回类型 → 用 ElectricalOnOffState（第3层）
     赋值时的枚举值   → 用 EnumElectricalOnOffState::ON/OFF（第2层）
     构造 SignalObject → 用 EnumSignalState::VALID/INVALID

④ Config 成员应该是 const 引用
   错误：Config config_;
   正确：const Config& config_;
   教训：Config 在模块生命周期内不变，用 const& 避免拷贝

⑤ namespace vs 文件夹的概念混淆
   误解："namespace 就是文件夹结构"
   真相：namespace 是 C++ 逻辑分组，和物理文件夹路径无关
         文件夹影响的是 #include 路径和 -I 编译参数
```

#### .cc 编写过程中的错误

```
⑥ const& 成员用赋值而非初始化列表
   错误：
     Function::Function(...) {
         config_ = config;           // ❌ const& 不能赋值
         pressure_ = pressure;
     }
   正确：
     Function::Function(...)
         : config_(config),          // ✅ 初始化列表，创建时绑定
           pressure_(parking_press),
     { ... }
   教训：const 引用成员只能在创建瞬间绑定，不能"先创建再赋值"
         → 必须用初始化列表（冒号语法）

⑦ 在构造函数中混淆声明与方法调用
   错误：
     utility::HysteresisFloat32 is_parking_press_active.GetValue();
     signal_object::ElectricalOnOffState parking_lamp_relay.GetState(Valid);
   正确：用初始化列表
     is_parking_press_active_(config.is_parking_press_active_config),
     parking_lamp_relay_(EnumElectricalOnOffState::OFF, EnumSignalState::VALID)
   教训：构造函数初始化列表是"创建对象"，不是"声明+调方法"

⑧ 缺少 void 返回类型
   错误：Function::Update() { ... }
   正确：void Function::Update() { ... }
   教训：C++ 函数必须声明返回类型，即使返回空也要写 void

⑨ Hysteresis 当成结构体手动比较阈值
   错误：
     if (parking_press_ > is_parking_press_active_.On_thresh)
   正确：
     is_parking_press_active_.Update(parking_press_.GetValue());
     if (is_parking_press_active_.GetState()) { ... }
   教训：Hysteresis 是个"黑箱工具"——喂值进去（Update），拿结果出来（GetState）
         不需要（也不能）手动访问它内部的阈值

⑩ output 变量声明在函数外面（全局变量）
   错误：把 output 声明放到 Update() 外面
   正确：声明在 Update() 内部
   教训：变量的作用域要最小化，局部变量放在使用它的函数内

⑪ BackLampRef() 缺 Function:: 前缀 + const + 括号多余
   错误：
     const signal_object::ElectricalOnOffState& BackLampRef()
     { return parking_lamp_relay_(); }
   正确：
     const signal_object::ElectricalOnOffState& Function::BackLampRef() const
     { return parking_lamp_relay_; }
   教训：
     ├── .cc 里方法必须加 Function:: 前缀
     ├── .h 里声明了 const，实现也必须加 const
     └── parking_lamp_relay_ 不是函数，不加 ()

⑫ Typo: onfig → config
   错误：is_parking_press_active_(onfig.is_parking_press_active_config)
   正确：is_parking_press_active_(config.is_parking_press_active_config)
   教训：没有自动补全的时候特别容易打错，要仔细检查变量名
```

#### 代码注释中的试错痕迹（清空前保存的学习记录）

> 最终编译通过的代码中留有注释掉的错误写法，记录了写代码过程中的真实试错路径。

```
⑬ Hysteresis::Update() 参数混淆 — Config vs 传感器读数
   注释掉的错误：
     is_parking_press_active_.Update(config.is_parking_press_active_config);
   正确：
     is_parking_press_active_.Update(parking_press_.GetValue());
   教训：
     ├── Config 是构造时传入的，用来设置 Hysteresis 的阈值（on_thresh / off_thresh）
     ├── Update() 需要的是"当前传感器实际读数"，不是配置参数
     ├── 类比：体温计的阈值(37.5°C)在出厂时设好 = Config
     │        每次量体温时测到的温度(36.8°C) = Update() 的参数
     └── 一句话：Config 设阈值，Update 喂实际值

⑭ 枚举值路径多余一层（ElectricalOnOffStateType::）
   注释掉的错误：
     signal_object::EnumElectricalOnOffState::ElectricalOnOffStateType::OFF
   正确：
     signal_object::EnumElectricalOnOffState::OFF
   教训：
     ├── EnumElectricalOnOffState 本身就是包装枚举值的类
     ├── OFF/ON 直接作为 EnumElectricalOnOffState 的静态成员
     ├── 不需要再通过 ElectricalOnOffStateType:: 去访问
     └── 错误原因：把三层类型链的层次搞混了
         第1层 ElectricalOnOffStateType 是 enum 定义
         第2层 EnumElectricalOnOffState 把 enum 值暴露为类的静态常量
         → 用第2层访问即可，不需要再经过第1层

⑮ 裸写 VALID（缺少完整命名空间）
   注释掉的错误：
     parking_lamp_relay_(signal_object::EnumElectricalOnOffState::OFF, VALID);
   正确：
     parking_lamp_relay_(signal_object::EnumElectricalOnOffState::OFF,
                         signal_object::EnumSignalState::VALID);
   教训：
     ├── VALID 不是全局常量，是 signal_object::EnumSignalState 类的成员
     ├── 必须写完整路径：signal_object::EnumSignalState::VALID
     └── 虽然有 include 传递链使得编译器认识 EnumSignalState，
         但使用时仍需显式限定命名空间

⑯ Include guard 拼写错误
   错误：#ifndef PARKING_LAMP_MODEL_PARATICE    ← PARATICE
   正确：#ifndef PARKING_LAMP_MODEL_PRACTICE    ← PRACTICE
   教训：
     ├── Include guard 拼错不会导致编译失败（只是名字不同而已）
     ├── 但如果将来另一个文件也拼对了 PRACTICE → 两个 guard 名字不同
     │   → 不会冲突，反而失去了防重复 include 的保护
     └── 没有自动补全时，长单词容易拼错，要特别注意

⑰ 同名 namespace 嵌套两层
   错误：
     namespace parking_lamp_model_practice {
     namespace parking_lamp_model_practice {   ← 重复了
   更好的写法：
     namespace parking_lamp_model_practice {
     namespace detail {   ← 内层用不同名字（或只用一层）
   影响：
     ├── 语法上合法（C++ 允许同名 namespace 嵌套）
     ├── 但导致 using 声明变成：
     │   using logic::wl::parking_lamp_model_practice::parking_lamp_model_practice::Function;
     │   ← 出现两次同名，不直观
     ├── 实际项目中看到的模式：外层=功能分类，内层=模块名
     │   如 logic::wl::vehicle_information::vehicle_completely_stop_model::Function
     └── 教训：namespace 层次要有不同的语义，不要机械重复
```

#### reverse_buzzer 代码中的额外遗留问题

```
⑬' Include guard 拼写不完整
    错误：#ifndef REVERSE_BUZZER_MODE_H     ← 缺了 L（MODE → MODEL）
    更好的：#ifndef REVERSE_BUZZER_MODEL_H
    教训：
      ├── Include guard 命名应该和文件名/类名对应
      ├── 拼写错误在 guard 名中不会报错但容易混淆
      └── 建议：guard 名 = 文件名全大写 + _H（如 REVERSE_BUZZER_MODEL_H）

⑭' 注释拼写（modeule → module）
    错误：/* Practice modeule - Reverse Buzzer */
    正确：/* Practice module - Reverse Buzzer */
    教训：注释虽然不影响编译，但面试官看代码时会注意拼写
```

#### 与 reverse_buzzer 对比（进步点）

```
reverse_buzzer（第1次）：              parking_lamp（第2次）：
──────────────────                  ──────────────────
忘写所有 #include                 →  #include 全部写对 ✅
class 结尾漏分号                  →  没漏 ✅
构造函数参数缺逗号                →  参数写对 ✅
RegistModule 拼错                →  直接写对 ✅
Update 大小写错                  →  大小写正确 ✅
把 OutputRef 写在 Update 里面     →  独立写出 BackLampRef ✅
Include guard 缺 L (MODE)        →  拼写错但完整 (PARATICE) ⚠️

两次都犯的共性错误（顽固弱点）：
├── ElectricalOnOffState 三层类型链混淆 → 两次都犯
├── 枚举值路径写法 → 两次都需要修正
└── Include guard 拼写 → 两次都有问题
    → 这些是需要刻意练习的重点
```

#### 新学到的概念

```
1. include 依赖链传递：
   electrical_on_off_state.h → #include signal_object.h → #include enum_signal_state.h
   所以 include 了 electrical_on_off_state.h 之后，EnumSignalState::VALID 自动可用
   不需要重复 include

2. const& 成员初始化的强制性：
   const 引用在 C++ 中必须在声明时绑定 → 唯一的机会就是初始化列表
   这不是风格偏好，是语言规则

3. 关掉 Copilot/IntelliSense 的价值：
   ├── 强迫自己思考每个符号从哪来
   ├── 暴露了真正不理解的概念（类型链、初始化列表语义）
   ├── typo 率大幅上升（但这是可以接受的代价）
   └── 建议：学习时关掉，工作时开着

4. Hysteresis 的两阶段使用模式：
   ├── 构造时：传 Config（包含 on_thresh / off_thresh 阈值）
   ├── 运行时：Update(传感器实际读数) → GetState() 取结果
   ├── Config 设"标准" → Update 喂"实际值" → GetState 取"判断结果"
   └── 和 IncrementTimer 的模式一致：构造(Config) → Update() → IsTimeUp()

5. 枚举值的正确访问路径速查：
   ├── ON/OFF 枚举值       → signal_object::EnumElectricalOnOffState::ON
   ├── VALID/INVALID 状态  → signal_object::EnumSignalState::VALID
   ├── 构造 SignalObject   → signal_object::ElectricalOnOffState(枚举值, 状态)
   └── 记忆口诀："Enum类::值" 取枚举，"不带Enum" 构造对象
```

### 15.5.9 每日重复练习法（4月22日开始）

> 从 parking_lamp 开始，尝试每天清空前一天写的代码，重新从记忆写一遍。

```
方法：
├── 每天早上清空 .h 和 .cc 文件内容
├── 不看任何参考代码，纯靠记忆重写
├── 写完后编译验证
├── 记录每次犯的新/旧错误

为什么有效：
├── 加深肌肉记忆 — API用法、include顺序、初始化列表语法会越来越熟练
├── 暴露理解盲区 — 卡住的地方就是还没真正理解的地方
├── 注意到细节变化 — 每次可能犯不同的错→暴露不同的弱点
├── 强化"写得出"能力 — 读10遍不如写1遍
└── 类比：音乐家每天练同一首曲子→追求的不是"会弹"而是"不用想就能弹"

进度记录：
├── Day 1 (4/21)：首次纯记忆写 parking_lamp → 编译通过，犯了17个错误（详见15.5.8）
├── Day 2 (4/22)：重写 → 初稿20个错误 → 自己修正 → 编译通过 ✅（详见15.5.11）
│   ├── 进步：Update()核心逻辑全对（Hysteresis用法、枚举路径、VALID路径）
│   ├── 初稿新问题：const位置偏差(6处)、构造函数参数vs成员变量混淆、命名空间分类
│   ├── 自己修好了：14个错误自行修正，最终剩8个后全部修通
│   └── 修正过程中学到：成员声明后不加const、struct也要分号、manager.方法()用点不用::
├── Day 3 (4/23)：重写 → 初稿仅6个错误 → 修1处const → 编译通过 ✅
│   ├── 5个顽固弱点全部修好：继承:、namespace嵌套、void、Function::、class分号
│   ├── 新错误：BoolSignal成员/参数漏const(2处)、kcommon漏.h、多了signal_object.h缺module_interface.h
│   ├── 逻辑错误：is_diag_normal少下划线+用GetState而非GetValue（已在提交前自行修正）
│   └── 趋势：错误从"结构性"变成"细节性"，说明框架已经内化
├── Day 4 (4/24)：升级版——清空重写 Hysteresis + IncrementTimer ✅
│   ├── 第一次提交（用昨天代码直接编译）：不算数，没有重写
│   ├── 第二次提交（清空重写）：3个错误（1编译 + 2逻辑）→ 修改后编译通过
│   ├── 编译错误：Manager参数缺& → framework::Manager& manager
│   ├── 逻辑错误1：局部变量遮蔽成员变量（加类型名变成声明而非赋值）
│   │   ❌ signal_object::ElectricalOnOffState parking_lamp_output_(output, ...);  ← 局部变量
│   │   ✅ parking_lamp_output_ = signal_object::ElectricalOnOffState(output, ...); ← 赋值成员
│   ├── 逻辑错误2：漏写 timer_.Update()（Timer三步曲少了第一步）
│   ├── 正确的：Clear条件用 || 写对了（昨天的逻辑错误已修正）
│   └── 里程碑：首次双工具组合（Hysteresis + Timer），编译错误仅1个
├── Day 5 (4/25)：（待追加）
└── （持续追加）

何时升级：
├── 同一个模块连续写3次都0错误 → 换一个更复杂的模块（如带IncrementTimer的）
├── 或者尝试不同的模块场景（但相同的代码模式）
└── 最终目标：拿到一个需求描述，不看参考就能写出完整的 .h + .cc
```

### 15.5.10 三个练习模块的对比（能力进化轨迹）

```
模块名           日期        方式           #include  初始化列表  类型链    编译   总错误数
─────────       ────────   ───────────    ────────  ─────────  ──────   ────   ────────
fan_control      4/17       AI辅助写        需帮助      需帮助     需帮助    4次    大量
reverse_buzzer   4/17-18    自己写(可看参考) 全忘        全忘       混淆     多次    12个
parking_lamp#1   4/21       纯记忆写        全对✅      基本对     仍混淆    少次    17个
parking_lamp#2   4/22       清空重写        漏1个⚠️    概念偏差   全对✅   编译通过   20→0个
parking_lamp#3   4/23       清空重写        漏1个⚠️    正确✅    全对✅   编译通过   6→0个  ✅ 目标达成！
parking_lamp#4   4/24       清空重写(+Timer) 全对✅      正确✅    全对✅   编译通过   1编译+2逻辑  Timer逻辑新挑战

Day 1→Day 2 变化分析：
├── 错误总数增加（17→20）但错误质量变了
│   ├── Day 1：包含核心逻辑错误（Hysteresis参数、枚举路径、裸VALID）
│   └── Day 2：核心逻辑全对✅，错误集中在语法细节（const位置、分号、括号类型）
├── 类比：弹钢琴 — Day 1 旋律弹错，Day 2 旋律对了但指法还不干净
└── 说明：理解层面在进步，语法层面需要更多肌肉记忆

进步清单（reverse_buzzer → parking_lamp#1 → parking_lamp#2）：
├── #include 全部写对（之前全忘）                       ✅ 肌肉记忆建立
├── class }; 分号没漏（之前漏了）                       ⚠️ Day2又漏了→还不稳定
├── 构造函数参数逗号没漏                                ✅
├── RegistModule 拼写直接写对（之前拼成RegisterModule）  ✅
├── Update() 大小写正确                                ✅
├── OutputRef 独立写出（之前塞进Update里）               ✅
├── Hysteresis.Update(sensor.GetValue()) 参数正确       ✅ Day2新进步！Day1传错config
├── 枚举路径 EnumElectricalOnOffState::ON 正确          ✅ Day2新进步！Day1多余层级
├── signal_object::EnumSignalState::VALID 完整路径      ✅ Day2新进步！Day1裸写VALID
├── ElectricalOnOffState(output, state) 输出构造正确    ✅ Day2新进步！
└── Include guard 拼写（PARATICE→PRACTICE）             ✅ Day2写对了

仍存在的顽固弱点（需要每天刻意注意）：
├── :: vs : 继承语法 → Day1犯了，Day2又犯了
│   记忆口诀："继承用单冒号 : 访问用双冒号 ::"
├── namespace 重复嵌套 → Day1犯了，Day2又犯了
│   规则：一个文件中同名 namespace 只写一次
├── void 返回类型遗漏 → Day1犯了，Day2又犯了
│   规则：.cc 中方法定义必须写返回类型
├── Function:: 前缀遗漏 → Day1犯了，Day2又犯了
│   规则：.cc 中所有方法定义都要 Function:: 前缀（OutputRef 特别容易忘）
├── class }; 结尾分号 → Day2又漏了
│   规则：class/struct 的 } 后面必须有分号

Day2 新暴露的概念盲区：
├── const 位置理解偏差 → 参数名后面加 const 是语法错误
│   ├── ✅ const Config& config        ← const在类型前
│   ├── ❌ const Config& config const  ← 参数后加const无意义
│   └── 只有方法声明末尾的 const 表示"不修改对象"
├── 构造函数参数 vs 成员变量混淆
│   ├── 参数 = 从外部传入：config, pressure, is_diag_normal, manager
│   ├── 成员 = 类内部持有：hysteresis(由config构造), output(用默认值构造)
│   └── 初始化列表中：参数→绑定引用, 成员→用值构造
├── 命名空间分类混乱
│   ├── framework:: → 基础框架类（ModuleInterface, Manager）
│   ├── signal_object:: → 信号类型（ElectricalOnOffState, BoolSignal, Pressure, Enum类）
│   └── utility:: → 工具类（HysteresisFloat32, IncrementTimer）
├── struct Config 用了() 而非 {}
│   └── struct/class 的成员定义体用花括号 {}，圆括号 () 是函数参数列表
└── #include 缺引号、文件名写成了不同模块名
    └── #include "parking_lamp_model_practice.h" ← 必须加引号，必须是本模块文件名
```

---

### 15.5.11 Day 2 详细错误记录（4月22日）

> 清空文件后纯记忆重写。核心逻辑大幅进步，但暴露了新的概念盲区。

#### .h 文件错误（13处）

```
① include guard 缺 _H
   写的：#ifndef PARKING_LAMP_MODEL_PRACTICE
   正确：#ifndef PARKING_LAMP_MODEL_PRACTICE_H

② 漏了 #include "electrical_on_off_state.h"
   → 输出类型 ElectricalOnOffState 需要这个头文件

③ namespace 重复嵌套（第3次犯！）
   写的：namespace parking_lamp_model_practice { namespace parking_lamp_model_practice {
   正确：只需要一层

④ :: vs : 继承语法（第3次犯！）
   写的：class Function::public ...
   正确：class Function : public ...

⑤ 基类命名空间+拼写错误
   写的：signal_objec::ModuleInterface
   正确：framework::ModuleInterface
   问题：signal_objec 少了 t，且应该是 framework 命名空间

⑥ struct Config 用了圆括号
   写的：struct Config( ... );
   正确：struct Config { ... };

⑦ HysteresisFloat32:Config 单冒号
   写的：HysteresisFloat32:Config
   正确：HysteresisFloat32::Config

⑧ 参数名后面加了多余的 const（×3处）
   写的：const Config& config const, const Pressure& pressure const, ...
   正确：const Config& config, const Pressure& pressure, ...
   根因：const 在类型前面表示"不可修改"，参数名后面加 const 是语法错误

⑨ Manager 类型和命名空间错误
   写的：signal_object::ModuleInterface manager
   正确：framework::Manager& manager
   问题：类型错(ModuleInterface→Manager)，命名空间错(signal_object→framework)，缺&

⑩ 成员声明后面加了多余的 const（×3处）
   写的：const Config& config_ const;
   正确：const Config& config_;

⑪ 输出成员变量缺下划线后缀
   写的：parking_lamp_output
   正确：parking_lamp_output_

⑫ class 结尾缺分号
   写的：}
   正确：};
```

#### .cc 文件错误（7处）

```
⑬ #include 缺引号+文件名错
   写的：#include parking_lamp_model_function.h
   正确：#include "parking_lamp_model_practice.h"

⑭ 构造函数参数包含了成员变量
   写的：Function::Function(..., utility::HysteresisFloat32 is_parking_press_active_,
                               signal_object::ElectricalOnOffState parking_lamp_output)
   正确：这两个是成员变量，不是参数！应该只有 config, pressure, is_diag_normal, manager
   根因：没有区分"从外部传入的"和"类内部持有的"

⑮ RegistModule 的调用对象错
   写的：signal_object::RegistModule(*this)
   正确：manager.RegistModule(*this)
   根因：RegistModule 是 Manager 的方法，不是 signal_object 的

⑯ 缺 void 返回类型（第3次犯！）
   写的：Function::Update(){
   正确：void Function::Update(){

⑰ 枚举变量类型的命名空间错
   写的：utility::EnumElectricalOnOffState::ElectricalOnOffStateType output;
   正确：signal_object::EnumElectricalOnOffState::ElectricalOnOffStateType output;
   根因：Enum类型在 signal_object 命名空间，不在 utility

⑱ OutputRef 方法缺 Function:: 前缀（第3次犯！）
   写的：const signal_object::ElectricalOnOffState& ParkingLampRef() const{
   正确：const signal_object::ElectricalOnOffState& Function::ParkingLampRef() const{

⑲ 输出成员变量用参数初始化而非默认值
   写的：parking_lamp_output_(parking_lamp_output)   ← 把不存在的参数传入
   正确：parking_lamp_output_(signal_object::EnumElectricalOnOffState::OFF,
                             signal_object::EnumSignalState::VALID)
   根因：输出型成员变量应该用默认值初始化（OFF + VALID）
```

#### Day 2 做对的部分（重要！）

```
✅ Update() 核心逻辑完全正确：
   is_parking_press_active_.Update(pressure_.GetValue());     ← Hysteresis参数正确！
   if (is_diag_normal_.GetValue() && is_parking_press_active_.GetState()) ← 条件正确！
   output = signal_object::EnumElectricalOnOffState::ON;      ← 枚举路径正确！
   parking_lamp_output_ = signal_object::ElectricalOnOffState(output, signal_object::EnumSignalState::VALID);
   ← 输出构造正确！VALID路径正确！

✅ if/else 结构正确
✅ using 声明正确
✅ 初始化列表的 config_(config) 等引用绑定写法正确
✅ 枚举变量声明（类型名长但基本对了，只是命名空间错了）
```

#### Day 2 自行修正过程

```
第1轮提交（初稿）→ 20个错误
    ↓ 查看反馈后自行修改
第2轮提交 → 修好14个，剩8个
    ↓ 继续修改
第3轮（最终版）→ 编译通过 ✅

自行修好的错误（说明这些概念已经开始内化）：
├── include guard 加上 _H ✅
├── 补上 electrical_on_off_state.h ✅
├── namespace 重复嵌套 → 去掉一层 ✅
├── :: vs : 继承语法 ✅（终于改对了！）
├── 基类命名空间 framework::ModuleInterface ✅
├── struct Config 改用 {} ✅
├── HysteresisFloat32::Config 双冒号 ✅
├── 构造函数参数去掉多余 const ✅
├── Manager 类型和命名空间 ✅
├── parking_lamp_output_ 加下划线 ✅
├── #include 加引号和正确文件名 ✅
├── 构造函数参数去掉成员变量 ✅
├── 补上 void 返回类型 ✅
├── 补上 Function:: 前缀 ✅

需要看反馈才改对的错误（最后8个）：
├── struct Config 结尾分号（和 class 一样要 ;）
├── 成员声明后面的 const（×3处）← 最顽固的概念盲区
├── Pressure 缺 signal_object:: 命名空间
├── class 结尾 }: → };（冒号写成分号）
├── using 声明重复命名空间
├── 构造函数漏了 manager 参数
├── manager::RegistModule → manager.RegistModule（:: vs .）
```

### 15.5.12 Day 3 详细错误记录（4月23日）

> 清空文件后纯记忆重写。初稿仅6个错误，修1处const后编译通过。

#### 初稿错误（6处）

```
.h 文件：
① kcommon 漏了 .h → #include "kcommon" → "kcommon.h"
② BoolSignal 参数和成员声明漏了 const → 输入信号必须是 const&
③ 多了 #include "signal_object.h"，少了 #include "module_interface.h"
   → signal_object.h 会被其他头文件间接包含，不需要直接include
   → module_interface.h 提供基类 ModuleInterface，必须直接include

.cc 文件：
④ is_diag_normal 漏了下划线 → is_diag_normal_（成员变量有 _ 后缀）
⑤ BoolSignal 用了 GetState() → 应该是 GetValue()
   记忆口诀：Signal家族 → GetValue() / 工具类 → GetState()/IsTimeUp()
⑥ 构造函数和Update()结尾多了分号 → 函数定义 } 后不需要分号（只有class/struct需要）
```

#### Day 3 做对的部分

```
✅ 5个顽固弱点全部修好：
   ├── 继承语法 : （不再写 ::）
   ├── namespace 不再重复嵌套
   ├── void 返回类型没漏
   ├── Function:: 前缀没漏
   └── class }; 分号没漏

✅ 核心逻辑连续第2天全对：
   ├── Hysteresis.Update(pressure_.GetValue())
   ├── 枚举路径 signal_object::EnumElectricalOnOffState::ON
   ├── VALID 完整路径
   └── 输出构造 ElectricalOnOffState(output, state)

✅ 修正能力：看到编译错误后自行定位到 const 不一致的问题并修复
```

### 15.5.13 六个基础概念深度理解（4月23日）

> Day 3 练习后提出的6个"为什么"问题，标志着从"模仿"转向"理解"。

#### Q1: 构建系统文件（.mk / .d）

```
文件类型        作用                              需要关心吗？
─────────    ───────────────                    ─────────
.mk          Makefile构建脚本：定义编译器、源文件目录、头文件路径    了解即可
.d           依赖关系文件：编译器自动生成，记录 .o 依赖哪些 .h      不需要关心
user.mk      项目级构建入口：USRSRCDIR 列出所有源文件目录           面试可能问到

完整构建链：
  .mk 定义规则 → Make 工具读取 → 调用 cxrh850 编译每个 .cc → 生成 .o → 链接 → 固件

Makefile 学习优先级判断：
├── 写模块代码练习 → ❌ 不需要，build.bat 够用
├── 面试 Junior 岗 → ⚠️ 了解概念即可，不需要能写
├── 入职后工作     → ✅ 到时候再学
└── 面试时只需能说：
    "The project uses Makefiles with the Green Hills cxrh850 cross-compiler.
     Source directories and include paths are defined in .mk files.
     Each .cc is compiled to a .o, then linked into the final firmware."

build.bat 对应的 Makefile 本质：
  cxrh850 -c source.cc -I ../include_path -o output.o
  ├── -c    → 只编译不链接
  ├── -I    → 头文件搜索路径（#include 能找到文件靠这个）
  └── -o    → 输出目标文件
```

#### Q2: Include Guard (#ifndef) 的作用

```
问题：如果 file_a.h 和 file_b.h 都 #include "common.h"
      而 main.cc 同时 #include "file_a.h" 和 "file_b.h"
      → common.h 的内容被复制了两次 → 编译器报 "redefinition" 错误

解决：
  #ifndef COMMON_H      ← 第1次：符号未定义 → 执行下面的内容
  #define COMMON_H      ← 定义符号
  ... 文件内容 ...
  #endif
                        ← 第2次：符号已定义 → 跳过全部 → 不会重复定义

不写会怎样？ → 间接include导致同一个class/struct定义出现两次 → 编译错误
命名规范：    → 文件名全大写 + _H（如 PARKING_LAMP_MODEL_PRACTICE_H）
```

#### Q3: 输入 const& vs 输出普通成员

```
输入信号：const signal_object::Pressure& pressure_
├── const → 只读（本模块不能修改别人的输出）
├── &     → 引用（不拷贝，指向外部对象，能看到实时更新的值）
└── 合起来 → "我只看你的值，不改它"

输出信号：signal_object::ElectricalOnOffState parking_lamp_output_
├── 没有 const → 本模块的 Update() 要修改它
├── 没有 &    → 本模块自己拥有这个对象（不是引用别人的）
└── 合起来 → "这是我自己的数据，我负责更新"

一句话：输入 = 看别人的（const& 只读引用）；输出 = 自己的（可写成员）
```

#### Q4: Config 为什么嵌套在 class 里面

```
class Function {
    struct Config { ... };    ← 嵌套定义
};

原因：
├── Config 只为这个 Function 服务 → 放里面表示专属
├── 外部用 Function::Config → 自带归属关系
├── 不同模块各有 Config → 同名但不冲突
│   parking_lamp::Function::Config
│   fan_control::Function::Config

为什么用 struct？
├── struct 默认 public → Config的成员需要从外部赋值
├── class 默认 private → 还要写 public: → 多此一举
└── 嵌入式惯例：纯数据容器 = struct，有行为 = class
```

#### Q5: manager.RegistModule(*this) 为什么在构造函数里

```
含义：
├── manager → 系统模块管理器（像"花名册"）
├── RegistModule() → "把我注册进去"
├── *this → "我自己"

为什么在构造函数里？
├── 模块创建就要注册 → Manager 才知道要调你的 Update()
├── 不注册 → Update() 永远不被调用 → 模块是"死的"
├── 时机：在初始化列表之后 → 确保成员变量都准备好了

系统启动流程：
  INSTANTIATION 宏创建 Function 对象
    → 初始化列表（绑定引用 + 构造工具类）
    → manager.RegistModule(*this)  ← "报到！"
    → Manager 记住这个模块
    → 每个周期(10ms) Manager 遍历花名册 → 调每个模块的 Update()
```

#### Q6: Update() 不传参数怎么拿到输入值

```
关键：输入值在构造时就通过 const& 绑定了

构造时：
  Function(..., const Pressure& pressure, ...)
      : pressure_(pressure)     ← pressure_ 绑定到外部对象

运行时：
  void Function::Update() {
      pressure_.GetValue()      ← 直接用成员变量，读到的是最新值
  }

为什么不用传参数？
├── pressure_ 是引用 → 不是拷贝，是"指向"外部对象
├── 外部对象每周期被上游模块更新
├── 你通过 .GetValue() 读到的自动就是最新值
└── 构造时绑定一次 → 运行时自动获取 = 依赖注入(DI)的精髓
```

### 15.5.14 IncrementTimer 工具模式分析（4月23日）

> 通过分析 afjs_lever_stroke_cross_check_diagnosis_model 模块学到的第二种工具模式。

#### IncrementTimer vs HysteresisFloat32 对比

```
                   HysteresisFloat32              IncrementTimer
用途              值的回差判断（防抖动）          时间的防抖判断（防瞬变）
Config内容        on_thresh, off_thresh           sampling_time, initial_time, threshold_time
构造              hysteresis_(config.xxx)         timer_(config.xxx)               ← 相同模式
运行时更新        Update(sensor.GetValue())       Update()                         ← Timer不需要传参！
判断结果          GetState() → bool               IsTimeUp() → bool               ← 方法名不同
重置              没有 Clear                      Clear()                          ← Timer特有
模式              "喂值→判断"                     "计时→超时→判断"

记忆口诀：
  Signal家族  → GetValue()              取信号的值
  Hysteresis → Update(值) + GetState()  喂值+取判断
  Timer      → Update() + IsTimeUp()   计时+取超时    (+Clear()重置)
```

#### IncrementTimer 的三步用法

```
1. timer_.Update()        ← 每个周期调用，内部计数器+1
2. timer_.Clear()         ← 条件满足时清零（重新开始计时）
3. timer_.IsTimeUp()      ← 到时间了吗？→ bool

实际代码模式：
  timer_.Update();                           // 先更新
  if (偏差在容忍范围 || 被屏蔽) {
      timer_.Clear();                        // 条件不成立 → 清零
  }
  if (timer_.IsTimeUp()) {                   // 持续超时 → 确认故障
      output = true;
  }

为什么需要 Timer？→ Debounce（防抖）
├── 传感器偏差可能是瞬间波动（噪声）
├── 只有持续超过阈值一段时间才算真正故障
└── Timer = "等一等再判断"
```

#### 从分析模块中学到的新模式

```
1. 环形缓冲区 (Ring Buffer)：
   array[3] = array[2]; array[2] = array[1]; array[1] = array[0]; array[0] = new;
   → 保存最近N个历史值，用于平滑/比较

2. FailureSignal 类型：
   EnumFailureState::NO = 没故障 / ::CHECK = 检查中
   → 比 BoolSignal 多状态的故障信号类型

3. 故障屏蔽 (Mask)：
   上游传感器本身故障时 → 屏蔽下游的交叉检查 → 避免误报
   → "只在前提条件正常时才做判断"的安全设计原则

4. 故障锁存 (Latch)：
   if (已经故障 || timer超时) → output = true
   → 一旦判定故障就保持 → 不会自动恢复 → fail-safe原则

5. 普通成员变量（bool, float32, float32[]）：
   不是所有成员都是 const& 引用！
   ├── const& → 外部注入的输入信号
   ├── 普通成员 → 模块内部的计算中间值/状态变量
   └── 数组 → 环形缓冲区等需要记住历史的场景
```

#### 面试话术

```
Q: "你了解传感器冗余设计吗？"
A: "Yes. In our system, we use cross-check diagnosis for redundant sensors.
    Two sensors measure the same physical quantity. If their readings diverge
    beyond a tolerance threshold, we use a debounce timer to filter out
    transient noise. Only if the gap persists beyond the timer threshold
    do we flag it as a genuine failure. We also mask the check when
    upstream dependencies like CAN communication are already in fault state,
    to avoid cascading false positives."

Q: "什么是 debounce？"
A: "Debounce means: don't react to the first occurrence — wait to see if the
    condition persists. We implement it with an IncrementTimer that counts up
    every cycle. If the condition clears, we Clear() the timer. If the timer
    reaches its threshold (IsTimeUp()), then we confirm the event."
```

### 15.5.15 诊断/故障模块体系分析（4月23日 方向D）

> 通过分析 fault_state_detection_model 和 diagnosis_check_model 学到的故障处理架构。

#### 故障状态四状态机

```
每个可能出故障的部件都有一个 FailureSignal，值为四种状态之一：

signal_object::EnumFailureState::
├── NO       没故障，一切正常
├── CHECK    正在检查中（还不确定）
├── OCCUR    故障已确认发生
└── RECOVER  故障恢复中

状态转移图：
              ┌──────┐
       正常运行│  NO  │←────────────────────┐
              └──┬───┘                      │
                 │ 检测到异常               │ 恢复确认
              ┌──▼───┐                      │
       等待确认│CHECK │                      │
              └──┬───┘                      │
                 │ 持续异常(防抖通过)         │
              ┌──▼───┐     ┌──────────┐     │
       故障！  │OCCUR │────→│ RECOVER  │─────┘
              └──────┘     └──────────┘
                           异常消失，等待恢复确认
```

#### 三层故障处理架构

```
Layer 1: 传感器/硬件层            Layer 2: 诊断判断层              Layer 3: 故障报告层
──────────────────              ──────────────                  ──────────────
传感器读数                       diagnosis_check_model          failure_indicator.h
├── 电压范围检查（断线/短路）      ├── 拿到 bool 标志               ├── 250+ 故障代码
├── 两个传感器交叉比对            ├── 喂给 FailureDetector          ├── 显示在 CANape 上
└── CAN 通信超时检查             └── 输出 FailureSignal            └── 记录到诊断日志
```

#### 核心组件：FailureDetector（第三种工具类）

```
abnormality::FailureDetector — 和 Hysteresis、IncrementTimer 并列的第三种工具

构造：FailureDetector(config, fault_mask)
更新：Update(bool flag)    ← 传入"是否有故障"的判断结果
输出：SignalRef()          ← 返回 FailureSignal（四状态）
内部：自动处理 NO→CHECK→OCCUR→RECOVER 的状态转移 + 防抖 + 屏蔽

三种工具对比：
  HysteresisFloat32  → Update(传感器值) → GetState()   → bool     "值过没过阈值？"
  IncrementTimer     → Update()        → IsTimeUp()   → bool     "时间到了没？"
  FailureDetector    → Update(bool)    → SignalRef()  → 四状态    "故障确认了没？"
```

#### 两个诊断模块对比

```
模块1: diagnosis_check_model::FailureCheckFunction（简单版）
────────────────────────────────────────────────
  输入：BoolSignal diagnosis_check_flag（某个检查结果）
  处理：failure_.Update(diagnosis_check_flag_.GetValue())  ← 直接把bool喂给检测器
  输出：FailureSignal（四状态）
  → 极简！防抖、状态转移、屏蔽全部由 FailureDetector 内部处理

模块2: fault_state_detection_model::Function（高级版 - Template Method）
────────────────────────────────────────────────
  特殊：用了 virtual 函数！
  ├── virtual bool IsFaultCondition()    ← 子类重写"什么算故障"
  ├── virtual bool IsRecoverCondition()  ← 子类重写"什么算恢复"
  └── Update() 里用 switch-case 根据当前状态决定逻辑

  Template Method 设计模式：
  ├── 基类定义流程骨架（Update的switch-case状态机）
  ├── 子类只需要定义"什么是故障"和"什么是恢复"
  └── 不同传感器诊断 = 继承基类 + 重写两个方法

  Update() 的状态机逻辑：
    当前是 NO 或 CHECK 状态：
      → fault_detector_flag = IsFaultCondition()
      → 只看"有没有故障"
    当前是 OCCUR 或 RECOVER 状态：
      → fault_detector_flag = IsFaultCondition() || !IsRecoverCondition()
      → 只要还有故障 OR 还没完全恢复 → 继续保持故障标志
      → 这就是"故障锁存 + 恢复确认"的双重安全设计
```

#### 故障屏蔽 (Fault Mask) 详解

```
每个 FailureDetector 都接收一个 FaultTransitionMaskOrder：
├── 允许外部条件暂时屏蔽故障转移
├── 例如：冷启动时传感器读数不稳 → 屏蔽诊断 → 等系统稳定后再开始检查
├── 或者：上游 CAN 通信故障 → 屏蔽下游交叉检查（避免误报级联）
└── 安全设计原则："不在不确定的条件下做诊断判断"
```

#### 故障代码体系

```
三个 failure_indicator.h 文件定义了所有故障代码：

failure_indicator.h        → 变速箱/通用系统故障 (~250个代码)
├── 速度传感器不匹配
├── 电磁阀开路/短路/接地
├── 传感器接线故障
├── CAN通信超时
├── 压力异常

engm_failure_indicator.h   → 发动机控制故障 (~150个代码)
├── 电池电压异常
├── 燃油喷射系统故障
├── EGR/涡轮传感器故障

pdc_failure_indicator.h    → 停车检测控制器故障 (~40个代码)
├── 摄像头/雷达故障
├── 蜂鸣器输出故障
```

#### 工具类全景（目前掌握的三种）

```
工具类              用途              核心API                      输出
─────────         ─────            ────────                    ─────
HysteresisFloat32  值的回差过滤       Update(值)+GetState()        bool
IncrementTimer     时间防抖过滤       Update()+Clear()+IsTimeUp()  bool
FailureDetector    故障状态管理       Update(bool)+SignalRef()      四状态FailureSignal
```

#### 设计模式全景（目前识别到的7种）

```
├── Composite Pattern     → Manager 树（Manager本身也是ModuleInterface）
├── Dependency Injection  → 构造函数注入 const& 引用
├── Strategy Pattern      → A/B 传感器切换
├── State Machine         → 故障四状态、操作模式切换
├── Template Method       → fault_state_detection 基类 + virtual 子类重写
├── Observer Pattern      → const& 引用观察其他模块输出
└── Star Topology FSM     → 所有状态经过中心空闲状态
```

#### 方向D 面试话术

```
Q: "你的项目中故障是怎么处理的？"
A: "We have a three-layer fault handling architecture:
    1. Hardware/sensor level: voltage range checks, wiring diagnostics
    2. Diagnostic module level: FailureDetector components that manage
       a four-state machine — NO, CHECK, OCCUR, RECOVER — with built-in
       debouncing and fault masking
    3. Reporting level: 250+ failure codes displayed via CANape diagnostics
    The key design principle is that faults are never declared immediately.
    They go through CHECK state with debounce timing, and recovery also
    requires confirmation. This prevents false positives from sensor noise."

Q: "什么是故障屏蔽(fault masking)？"
A: "Fault masking allows us to temporarily suppress diagnostic checks
    under known unreliable conditions — like cold start, low battery voltage,
    or upstream communication failures. The idea is: don't diagnose when
    your diagnostic inputs themselves might be unreliable."

Q: "你提到了 Template Method 模式？"
A: "Yes. Our fault detection base class defines the diagnostic state machine
    flow in Update(), but declares IsFaultCondition() and IsRecoverCondition()
    as virtual methods. Each specific sensor's diagnostic module inherits from
    this base and overrides just those two methods. This separates the 'how to
    manage fault states' from 'what counts as a fault for this specific sensor'."
```

### 15.5.16 交叉检查模块深度问答（4月23日）

> 逐行读 afjs_lever_stroke_cross_check_diagnosis_model 时产生的深入问题。

#### Q1: 故障屏蔽条件的取反逻辑

```
代码：
  is_cross_check_mask_ = !(CAN正常 && sensor1没故障 && sensor2没故障 && 没屏蔽命令)

为什么取反？
├── 括号里 = "可以安全检查吗？"（所有前提条件正常 → true）
├── mask 的含义 = "需要屏蔽吗？"（需要跳过检查 → true）
├── 两者刚好相反 → 取反
│
├── 一切正常 → !(true)  → mask=false → 不屏蔽 → 正常检查 ✅
└── 任何一个异常 → !(false) → mask=true → 屏蔽 → 跳过检查 ✅

等价于德摩根定律：
  !(A && B && C && D) = !A || !B || !C || !D
  = "任何一个不正常就屏蔽"

为什么不直接写 || ？
  → 括号里写"理想状态"更清晰，取反就是"需要屏蔽"
  → 读代码时：先理解括号里的"一切正常"条件，再取反
```

#### Q2: Timer 判断中的 || 为什么导致 Clear

```
代码：
  if ((|偏差| ≤ 容忍值) || is_cross_check_mask_) {
      timer.Clear();
  }

|| 意思是"满足任何一个条件就清零"：
├── 条件A：偏差在范围内（传感器正常）→ 不需要报 → Clear
├── 条件B：被屏蔽了（不适合检查）    → 不应该报 → Clear
├── 都不满足：偏差大 AND 没被屏蔽    → 计时器继续涨 → 最终报故障

|| 扩大了"不报警"的范围 → 安全原则：宁可漏报也不误报
```

#### Q3: 为什么用传感器1的历史范围比较（不直接比当前值）

```
问题：为什么不直接 |sensor1 - sensor2| > tolerance？

原因：传感器有响应时间差！
├── 操作员快速推杆 → sensor1先反应，sensor2慢0.01秒
├── 同一瞬间两个读数可能暂时不一样 → 不是故障，是正常延迟
├── 直接比较 → 快速操作时误报

解决：用 sensor1 的历史范围（[1][2][3] 的 max/min）建立"正常运动带"
├── sensor2 在这个范围内 → gap=0 → 正常（允许"跟得慢"）
├── sensor2 偏离范围 → 有偏差 → 开始计时（不允许"跟不上"）

为什么不包含 [0]（本周期最新值）？
├── [0] 是刚读到的，可能含噪声尖峰
├── [1][2][3] 是经过时间验证的历史值 → 更稳定
└── 用已验证的历史做标准 → 更可靠

一句话：范围比较允许"跟得慢"，但不允许"跟不上"
        这就是区分"正常响应延迟"和"真故障"的关键
```

#### Q4: 为什么先 Update 再 Clear（不能反过来）

```
正确顺序（代码用的）：
  timer.Update()    先+1
  if (正常) timer.Clear()  再判断是否清零

  → 正常时：0→+1→Clear→0  最终=0 ✅
  → 异常时：0→+1（不Clear）  最终=1 ✅

反过来：
  if (正常) timer.Clear()  先判断
  timer.Update()    再+1

  → 正常时：0→Clear→0→+1  最终=1 ❌ 明明正常却显示1！
  → 异常时：0→（不Clear）→+1  最终=1

问题：反过来做的话，正常和异常的 timer 值一样（都是1），分不清！

比喻：考勤打卡
  正确：先记迟到，没迟到的话划掉 → 没迟到=0记录
  反过来：先划掉，再记 → 没迟到也显示1次 → 冤枉！

一句话：先 Update 再 Clear → 保证"正常时 timer 一定是0"
```

#### Q5: SignalObject 的统一构造模式

```
所有 SignalObject 都是 (值, 信号状态)：

BoolSignal:              (false,                         EnumSignalState::VALID)
ElectricalOnOffState:    (EnumElectricalOnOffState::OFF, EnumSignalState::VALID)
Pressure:                (0.0F,                          EnumSignalState::VALID)
FailureSignal:           (EnumFailureState::NO,          EnumSignalState::VALID)

第1个参数 = "值是什么？"  → 类型不同（bool / enum / float32）
第2个参数 = "可信吗？"    → 永远是 VALID 或 INVALID

值怎么写？看类型：
├── bool 类型    → false / true         ← C++ 内置关键字，直接写
├── 自定义枚举   → Enum...::OFF / ::NO  ← 需要完整命名空间路径
├── 浮点数       → 0.0F                 ← 数字字面量

为什么 false 不需要命名空间？
├── false 是 C++ 语言关键字，全局可用
├── EnumElectricalOnOffState::OFF 是项目自定义的 → 需要完整路径

SignalObject 的设计思想：
  不只传"值"，还传"这个值可不可信"
  在安全系统中，"数据不可信" 和 "数据是什么" 一样重要
```
                          ↑你到这里了

.o 能证明：语法正确 ✅  类型系统对 ✅  头文件完整 ✅  框架API对 ✅
.o 不能：直接运行、烧录MCU（需要链接所有模块 + main + 启动代码）
对练习来说：编译出 .o = 验证通过，足够了
```

### 15.5.17 Day 4 详细记录——清空重写升级版 Hysteresis + Timer（4月24日）

**任务**：清空代码，从零重写 parking_lamp 升级版（Hysteresis + IncrementTimer）。
**注意**：第一次提交用的是昨天的代码直接编译，不算练习。以下是清空重写的真实结果。

**结果：3 个错误（1 编译 + 2 逻辑）→ 修改后编译通过**

**编译错误：Manager 参数缺 &**
```
❌ framework::Manager manager      ← 值传递，Manager不可复制
✅ framework::Manager& manager     ← 引用传递

这个错误之前没犯过，属于手滑。
Manager 是框架级单例对象，永远用引用传递，不复制。
```

**逻辑错误1：局部变量遮蔽成员变量（新错误类型！）**
```
❌ signal_object::ElectricalOnOffState parking_lamp_output_(output, signal_object::EnumSignalState::VALID);
   ↑ 这是声明了一个新的局部变量，和成员 parking_lamp_output_ 同名
   ↑ 函数结束后局部变量销毁，成员变量永远是初始值 OFF

✅ parking_lamp_output_ = signal_object::ElectricalOnOffState(output, signal_object::EnumSignalState::VALID);
   ↑ 这是赋值给成员变量

区别：
├── 有类型名 = 声明新变量（即使名字和成员相同）
├── 没类型名 = 赋值给已有变量
└── 编译器不报错（变量遮蔽是合法的C++），只能靠理解发现

记忆口诀："Update里赋值不加类型名"
```

**逻辑错误2：漏写 timer_.Update()**
```
Timer 三步曲必须完整：
1. timer_.Update();      ← 漏了这步！没有递增，IsTimeUp()永远false
2. 条件判断 → timer_.Clear();
3. timer_.IsTimeUp() → 输出判断

记忆口诀："Timer三步：Update、Clear、IsTimeUp，一步都不能少"
```

**写对的部分（进步确认）**
```
✅ Clear 条件用 || 而非 &&（之前试错中学到的逻辑已修正）
✅ #include "increment_timer.h"
✅ Config 中 Timer 和 Hysteresis 的 Config 都正确
✅ 初始化列表中 Timer 初始化正确
✅ 所有 namespace、继承、void、Function:: 前缀、class 分号都正确
   （Day 1-3 顽固弱点全消灭）
```

**错误性质分析**
```
Day 1-3 错误：语法/结构（编译器能抓到）
Day 4 错误：  1个语法手滑 + 2个逻辑（编译器抓不到的更危险）

局部变量遮蔽 = C++ 的经典陷阱：
├── 编译器完全不报错
├── 行为完全错误（成员变量没被更新）
├── 只有理解"声明 vs 赋值"的区别才能避免
└── 实际工作中也是常见 bug 来源
```

---

## 十六、方向B：数据流全链路追踪

> 追踪一个信号从物理传感器到最终消费者的完整旅程，理解模块间如何串联。

### 追踪对象：revolution（转速）→ is_stop（是否停车）

### 完整数据流图

```
硬件层                设备驱动层              诊断选择层              故障切换层              判断层              下游消费者
───────           ──────────            ──────────            ──────────           ──────           ──────────

变速箱输出轴        tm_output_a_rev        tm_output_convert_     tm_output_convert_    vehicle_          12+个模块：
齿轮传感器A ──→    (device::Rotational     model_tm_output_       model_tm_output_      completely_       parking_brake,
齿轮传感器B ──→     SpeedSensorByDc        revolution_            direction_and_        stop_model        vehicle_permission,
(DC脉冲信号)        PulseInput)            function               revolution_switch_                      shift_change,
                   tm_output_b_rev         (选A还是B？)           function                               neutral_safety...
                                                                  (正常还是跛行？)
```

### 每一层详解

#### ① 硬件层：齿轮传感器
- 变速箱输出轴上有齿轮，齿轮旁边安装电磁传感器
- 齿轮每转过一个齿就产生一个电脉冲（DC Pulse）
- **两个传感器（A和B）测同一根轴** — 冗余设计

#### ② 设备驱动层：脉冲 → 转速
```
对象名：tm_output_a_rev, tm_output_b_rev
类型：device::RotationalSpeedSensorByDcPulseInput
```
- **输入**：DC脉冲频率（硬件直接提供）
- **输出**：`RotationalSpeedRef()`（转速 rpm）, `FrequencyRef()`, `PulseRefreshStateRef()`
- **Config**：包含 `teeth_number`（齿数） — 频率 ÷ 齿数 = 转速
- **不在 user_x/module/wlh/ 里** — 这是 device 设备驱动层模块（Middleware 提供）

#### ③ 诊断选择层：选 A 还是 B？
```
对象名：tm_output_convert_model_tm_output_revolution_function
类型：TmOutputRevolutionFunction（业务模块）
```
- **输入**：
  - `revolution_a` — 传感器A的转速
  - `revolution_b` — 传感器B的转速
  - `is_diag_normal_a` / `is_diag_normal_b` / `is_diag_normal_a_b` — 诊断状态
  - `pulse_refresh_state_a` / `_b` — 脉冲刷新状态
  - `frequency_a` / `frequency_b` — 原始频率
  - `is_agc_complete` — 自动增益校准完成
- **输出**：`OutputRevolutionRef()` — 选择后的转速
- **逻辑**：A正常用A，A故障用B，AB都故障用某种组合

#### ④ 故障切换层：正常还是跛行？
```
对象名：tm_output_convert_model_tm_output_direction_and_revolution_switch_function
类型：TmOutputDirectionAndRevolutionSwitchFunction（业务模块）
```
- **输入**：
  - `revolution_normal` — 正常模式转速（来自③）
  - `revolution_limp_home` — 跛行模式估算转速
  - `is_enable_limp_home` — 是否启用跛行模式
  - `is_revolution_sensor_failure` — 转速传感器是否故障
  - `direction_normal` — 正常方向
- **输出**：`OutputRevolutionRef()` — 最终转速
- **逻辑**：传感器正常→用正常转速，传感器全坏→切换到跛行估算转速

#### ⑤ 判断层：停了吗？
```
对象名：vehicle_completely_stop_model
类型：你已经学过的模块！
```
- **输入**：`revolution`（来自④）
- **Config**：threshold=40rpm, time=500ms
- **输出**：`IsStopRef()` — bool（停了/没停）

#### ⑥ 下游消费者：12+ 个模块
在接线文件中搜索 `vehicle_completely_stop_model.IsStopRef()` 出现了 12+ 次，被以下类型的模块使用：
- `vehicle_permission_function` — 车辆许可（引擎启停安全判断）
- `parking_brake` 相关 — 驻车制动（停稳了才能锁住）
- `shift_change` 相关 — 挡位切换（停稳了才能换挡）
- `neutral_safety` 相关 — 空挡安全

### 追踪方法（如何自己做数据流追踪）

```
上游追踪（信号从哪来）：
步骤1: 搜索 "目标实例名," （带逗号！精确定位 INSTANTIATION 块）
步骤2: 读 INSTANTIATION 块，看 /* 参数名 : */ 后面的值 → 那就是信号来源
步骤3: 信号来源的格式是 "对象名.输出方法()"
步骤4: 再搜索 "对象名," → 找到它的 INSTANTIATION → 重复步骤2
步骤5: 反复追踪直到找到 device:: 层（硬件入口）或 network:: 层（CAN总线入口）

下游追踪（信号被谁用）：
步骤1: 搜索 "目标实例名.输出方法()" → 所有匹配行就是消费者
步骤2: 读每个匹配行的上下文（±5行）→ 看是哪个模块的什么参数

搜索技巧：
├── 搜 "实例名,"（带逗号）比搜纯实例名更精确 → 直接命中 INSTANTIATION 定义行
├── 搜 "实例名."（带点号）→ 找所有使用该实例输出的地方
├── 注释中的 /* param_name : */ 是理解参数含义的关键
└── 一句话总结：搜 INSTANTIATION 找接线 → 参数注释找上游 → 搜 .输出方法() 找下游 → 读 .cc 看逻辑
```

### 下游消费者完整列表（17处引用）

| 行号 | 消费模块 | 参数名 | 用途 |
|------|---------|--------|------|
| L3749 | vehicle treatment model | — | EDS/AS 车辆许可检查 |
| L4108 | `auto_idle_stop_vehicle_permission_function` | `is_vehicle_completely_stop` | 自动怠速停止许可 |
| L4154 | `parking_brake_model::IgnitionJudgeFunction` | `is_tm_output_rev_stop` | 驻车制动点火判定 |
| L4315 | `motor_2_revolution_estimation_model` | `is_vehicle_completely_stop` | Motor2 转速估算 |
| L4479 | `so_switch_model::Function<BoolSignal>` | `true_so` | 布尔逻辑切换（故障屏蔽） |
| L8431 | `neutralizer_setting_model::SwitchValueFunction` | `is_vehicle_completely_stop` | 空挡器设置控制 |
| L8458 | `neutralizer_mode_select_model::Function` | `is_vehicle_completely_stop` | 空挡器模式选择 |
| L8545 | `warming_up_model::Function` | `is_vehicle_complete_stop` | 暖机逻辑 |
| L9237 | `limp_home_permit_function` | `is_vehicle_stopping` | **跛行模式许可（反馈回路！）** |
| L10682 | `neutral_safety_model::ActivateJudgeFunction` | `vehicle_completely_stop` | 空挡安全互锁 |
| L11273+ | 其他多个模块 | — | SO切换、故障屏蔽等 |
| L17094 | — | `is_vehicle_stop` | 车辆停止判断 |
| L22332 | — | `input1` | 逻辑与运算输入 |

### 反馈回路发现 ⭐

```
vehicle_completely_stop_model.IsStopRef()
    ↓ (L9237: is_vehicle_stopping)
limp_home_permit_function.IsLimpHomeEnableRef()
    ↓ (L1707: is_enable_limp_home)
tm_output_convert_model_..._switch_function.OutputRevolutionRef()
    ↓ (L3570: revolution)
vehicle_completely_stop_model  ← 回到起点！

这是一个反馈回路：停车判定的输出影响跛行模式的许可，
跛行模式又影响转速信号的选择路径，而转速信号是停车判定的输入。
这种反馈在控制系统中很常见，但需要注意时序（谁先Update）避免循环依赖。
```

### 关键设计原则总结

| 原则 | 在这条链路中的体现 |
|------|-------------------|
| **冗余传感器** | A/B 双传感器测同一根轴 |
| **诊断切换** | 根据传感器健康状态自动选择 A/B |
| **跛行回家(Limp Home)** | 传感器全坏→用估算值慢慢开回修 |
| **单一职责** | 4个模块各做一步，不是1个做4步 |
| **松耦合** | 每个模块只知道自己的输入类型，不知道来自谁 |

---

## 十七、方向C：构建系统（从源码到烧写文件）

> 理解项目如何编译、链接、生成最终烧写文件。

### 19.1 整体架构：4个库 + 1次链接

```
源码目录                              编译器(cxrh850)         链接
───────                             ──────────            ────
module/wla/src/*.cc (自动生成模块) ──→ module_wla.lib  ─┐
module/wlh/src/*.cc (手写模块)    ──→ module_wlh.lib  ─┤
system/network/src/*.cc (CAN通信) ──→ network.lib     ─┼──→ .abs → .mot2
system/memory/src/*.cc (内存数据) ──→ memory.lib      ─┘
                                                       ↑ 
                                          + Middleware/[RTOS]/utility 的代码
```

**分别编译成4个 .lib（库文件），最后一起链接成 .mot2。**

### 19.2 构建入口：Shell 脚本

所有脚本位于 user_x/ 根目录：

```
user_x/
├── module_update.sh          ← 总入口（调用下面两个）
├── module_wla_update.sh      ← 编译自动生成模块 → module_wla.lib
├── module_wlh_update.sh      ← 编译手写模块 → module_wlh.lib（你最常用的！）
├── module_wla_listup.csx     ← C# 脚本，列出 wla 模块清单
├── module_wlh_listup.csx     ← C# 脚本，列出 wlh 模块清单
└── modules_by_handcode.csv   ← 手写模块列表
```

另外两个库的编译脚本在 system/ 目录下：
```
system/
├── memory_update.sh          ← 编译内存数据库 → memory.lib
└── network_update.sh         ← 编译 CAN 通信库 → network.lib
```

### 19.3 日常编译命令

```bash
# 最常用：编译手写模块（修改 .cc 后运行这个）
sh module_update.sh 4        # 4 = 并行编译数（加快速度）

# 只在改了 CAN/内存配置时才需要
sh system/memory_update.sh 4
sh system/network_update.sh 4
```

### 19.4 module_update.sh 逐行解析

```bash
#!/usr/bin/env bash           # 声明用 bash 执行
./module_wla_update.sh $1     # 先编译自动生成的模块
./module_wlh_update.sh $1     # 再编译手写的模块
```
`$1` 是你传入的参数（如 `4`），传递给子脚本。

### 19.5 module_wlh_update.sh 逐行解析（重点！）

```bash
#!/usr/bin/env bash

rm -f ./module/module_wlh.lib ./module/module_wlh.dba
# ① 删掉旧的库文件（强制重新生成）

(cd ./app_logic/bin ; make clean)
# ② 进入 bin 目录，清除所有 .o 编译中间文件

csi ./module_wlh_listup.csx
# ③ 运行 C# 脚本，扫描 module/wlh/src/ 生成模块清单
#    csi = C# Interactive（类似 python 命令行运行 .py）

(cd ./app_logic/bin ; make handlib -j$1 )
# ④ 编译所有手写模块 → module_wlh.lib
#    -j4 = 4个文件同时编译（并行加速）
```

**一句话**：清理 → 列清单 → 并行编译成库。

### 19.6 Makefile 文件结构

构建核心配置在 `app_logic/bin/` 下：

| 文件 | 作用 |
|------|------|
| `makefile` | **主 Makefile** — 全局配置（编译器版本、板子类型、输出文件名） |
| `user.mk` | **用户配置** — 源码目录、头文件路径、链接哪4个库 |
| `mkmodule_wl.mk` | **模块编译规则** — wla/wlh 的 .cc → .o → .lib 规则 |

另外还有：
| 文件 | 作用 |
|------|------|
| `system/network/network.mk` | 网络库编译规则 |
| `system/memory/memory.mk` | 内存库编译规则 |

### 19.7 Makefile 关键配置值

```makefile
KRONOS_VERSION = 5_13_1-P3          # [RTOS] 版本
BOARD_TYPE = [ControllerBoard]       # RH850 控制板
MIDDLEWARE_VERSION = 1_3_0-S6       # 中间件版本
UTILITY_VERSION = 1_0_0             # 工具库版本
IMAGENAME = App[Controller]_00_01  # 输出文件名
PRODUCT_NO = [REDACTED]             # 客户零件号
APP_VERSION = 02008000              # 软件版本号
```

### 19.8 编译器：cxrh850

```
cxrh850 = Green Hills 公司的 RH850 芯片专用 C++ 编译器（交叉编译器）
```
- 不是 GCC，不是 MSVC
- 在你的 PC（Windows）上运行，但产出的机器码只能在 RH850 芯片上跑
- 这叫"交叉编译"：编译平台 ≠ 运行平台

### 19.9 make 的常用 target

```bash
# 在 app_logic/bin/ 目录下
make handlib -j4     # 编译手写模块 → module_wlh.lib
make autolib -j4     # 编译自动生成模块 → module_wla.lib
make clean           # 清除编译中间产物（.o 文件）
make all             # 全量编译 + 链接 → .abs → .mot2
```

### 19.10 编译产物路径

| 文件 | 路径 | 说明 |
|------|------|------|
| `module_wlh.lib` | `module/lib/` | 手写模块库 |
| `module_wla.lib` | `module/lib/` | 自动生成模块库 |
| `network.lib` | `system/network/lib/` | CAN 通信库 |
| `memory.lib` | `system/memory/lib/` | 内存数据库 |
| `.abs` | `app_logic/bin/` | 可执行文件（含调试信息） |
| `.mot2` | `app_logic/bin/` | **最终烧写文件** |

### 19.11 K36 和 K37 共用代码

```
同一份源码，两个机型共用！
区别只在 app_logic/config_tl/ 目录下的配置参数。
├── config_tl/k36/    ← K36 机型的参数
└── config_tl/k37/    ← K37 机型的参数

tl_config_build.csx 脚本决定用哪个目录的配置。
```

### 19.12 日常开发完整流程

```
1. 修改 module/wlh/src/xxx_model_function.cc
2. 在 user_x/ 目录运行：sh module_update.sh 4
3. 等待编译完成（几分钟）
4. 新的 .mot2 出现在 app_logic/bin/
5. 用烧写工具把 .mot2 烧到 RH850 板子上
6. 开机测试
```

---

## 十七·五、核心概念速查卡（脱离项目也能用） / Key Concepts Quick Reference

> 忘了的时候翻这里，每个概念 5-10 行搞定。详细版见各章。

### 🔧 防抖（Debounce）

```
问题：传感器信号有噪声，在阈值附近快速跳变 → 误触发
方案：条件必须持续满足一段时间才算数

实现（IncrementTimer + Clear 模式）：
  每周期: timer += 10ms
  条件不满足: timer = 0（清零重来）
  timer ≥ 阈值: 输出 true（确认了！）

例子：转速 < 40rpm 持续 500ms → 才判定"车真的停了"
一句话：瞬间波动不算，持续够久才算。
详见：第十章（IncrementTimer源码）、第十四章14.2
```

### 🔧 冗余传感器切换

```
问题：传感器会坏，但 50 吨机器不能因此失控
方案：同一物理量用两个传感器测（A/B 双路）

切换逻辑（三层）：
  第1层 — AB都正常: max(A,B)；一路坏: 用好的那路；都坏: 输出0
  第2层 — 传感器正常: 用实测值；传感器全坏: 切到估算值（跛行模式）
  第3层 — 跛行估算: 用别的传感器(Motor1+Motor2) + 齿轮比反算出转速

设计原则：
  ├── 正常时用最准的（实测值）
  ├── 部分故障时降级（用剩余传感器）
  └── 全故障时兜底（估算值，慢慢开回去修）
一句话：永远有退路，绝不趴窝。
详见：第十七章（数据流追踪 ②③④ 层）
```

### 🔧 状态机设计

```
核心结构：switch-case + 枚举状态

三个设计要点：
  ① 星形拓扑 — 所有状态经过中央枢纽(NOT_ACTIVE)，不直接互跳
     原因：不需要 + 不安全（残留状态） + 更复杂 = 不做
  ② 三相执行 — transition(判断跳不跳) → entry(进入时初始化一次) → do(每周期执行)
     entry 和 do 的区别：Clear计时器(一次) vs Update计时器(每周期)
  ③ 状态记忆 — 上次输出 = 这次的当前状态（output_.GetValue()）

模板：
  switch (当前状态) {
      case 状态A: if (条件) → 跳到状态B; break;
      case 状态B: if (条件) → 跳回状态A; break;
  }
一句话：用枚举记"现在在哪"，用switch-case决定"下一步去哪"。
详见：第十五章模块7（边沿检测）、模块8（雨刷控制）
```

### 🔧 边沿检测（Edge Detection）

```
问题：开关按住1秒 = 100次ON信号 → 下游如果判断ON就触发100次
方案：把"按住"变成"按了一下"

  OFF → OFF_ON（刚按下，只存在1个周期）→ ON（一直按着）
  ON  → ON_OFF（刚松开，只存在1个周期）→ OFF（一直松着）

  OFF_ON = 上升沿 ↑  只在按下那一瞬间为true
  ON_OFF = 下降沿 ↓  只在松开那一瞬间为true

用途：下游判断 if (state == OFF_ON) → 只触发一次，不管按多久
一句话：把持续信号变成瞬间信号。
详见：第十五章模块7（switch_momentry_convert）
```

### 🔧 信号对象 = 容器（值 + 状态）

```
每个信号 = 值 + 是否可信
  BoolSignal(true, VALID)   ← "停了，而且这个判断是可信的"
  BoolSignal(false, INVALID) ← "没停，但这个数据不可信"

操作三步：
  拆包: value = signal.GetValue()           ← 取出值
  逻辑: if (value >= threshold) ...          ← 用值做判断
  装包: output_ = BoolSignal(result, VALID)  ← 把结果+状态塞回去

规则：业务模块(wlh层)永远输出 VALID，INVALID 来自底层（CAN超时、传感器断线）
一句话：光有值不够，还要知道值是否可信。
详见：第八章（信号对象）
```

### 🔧 松耦合接线机制

```
模块A 不知道 模块B 的存在（互不 #include）
模块只 include 信号类型（RotationalSpeed.h）和框架类（Manager.h）

连接方式：
  接线文件 include 了所有 400+ 个模块的 .h
  然后写: INSTANTIATION(B, ..., A.OutputRef(), ...)
  → A的输出作为B的输入传入构造函数 → 完成连接

INSTANTIATION 的参数 = 构造函数参数 = 全是输入
输出不在自己的 INSTANTIATION 里，在别人的 INSTANTIATION 里

找输入：看自己的 INSTANTIATION
找输出：搜 "实例名." → 看被谁引用
一句话：模块只管做菜，接线文件决定谁吃谁做的菜。
详见：第六章（INSTANTIATION宏）、第八章（模块间通信）
```

### 🔧 三个概念的关系

```
冗余传感器 → 给你可靠的原始数据
    ↓
防抖过滤 → 去掉噪声，确认"真的满足条件了"
    ↓
状态机 → 根据确认后的条件，控制系统模式切换
    ↓
边沿检测 → 把持续动作变成一次性触发
    ↓
信号对象 → 全程携带"值+是否可信"

目标：大型建设机械上，不能因为一个传感器故障或一次信号波动做出错误动作。
```

---

## 十七·六、CAN/J1939 通信基础（读代码够用版） / CAN & J1939 Basics for Code Reading

> 2026-05-14 学习记录。读CAN模块代码时补充的通信层基础知识。

### CAN是什么？

```
CAN = Controller Area Network = 车辆/工程机械内部的通信总线

一条CAN总线 = 一条群聊频道
每个ECU（控制器）= 群里的一个人
每条消息 = 一条群发消息，带有"话题ID"

特点：
├── 发送方不指定"发给谁" → 广播到总线上
├── 接收方自己决定"这个话题我要不要听"
└── 每条消息最多8字节（64bit）
```

### CAN帧的结构

```
一条CAN消息 = 一个帧（Frame）

┌──────────┬──────────────────────────┐
│  帧ID    │  数据（最多8字节）         │
│ (29bit)  │  [B0, B1, B2, B3, B4, B5, B6, B7] │
└──────────┴──────────────────────────┘

帧ID = 这条消息的"话题号"（例：0x00FEF500 = 环境条件）
数据 = 最多8字节 = 64bit，一个帧里可以塞多个信号
```

### J1939 = CAN之上的应用层协议

```
CAN  = 信封的格式（大小、地址位置）→ 规定怎么发8字节
J1939 = 信纸的内容格式（第几行写什么）→ 规定8字节里装什么
```

### PGN（Parameter Group Number）= 参数组编号

```
帧ID里包含PGN = 这条消息的话题

例：帧ID 0x00FEF500 → PGN = 0xFEF5 = 65269 = "Ambient Conditions"（环境条件）

J1939标准规定了每个PGN里装什么信号、每个信号在哪个byte。
```

### SPN（Suspect Parameter Number）= 具体参数编号

```
SPN = J1939给每个物理量取的编号，像学生的学号一样。

学号001 = 张三
学号002 = 李四

SPN 108 = 大气压
SPN 171 = 环境温度
SPN 190 = 发动机转速

一个PGN（信）里装多个SPN（段落）：
  PGN 65269 这封信 = SPN 108（大气压） + SPN 171（温度） + 其他

代码里不会直接看到"SPN 108"这个数字 → KDOG2工具已经把SPN查表结果翻译成了config。
```

### 物理值转换公式（核心！）

```
物理值 = 原始值 × resolution + offset

大气压：200 × 0.0005 + 0      = 0.1 kPa
温度：  9600 × 0.03125 + (-273) = 300 - 273 = 27°C
```

### 有效范围（Valid Range）

```
J1939规定uint8的特殊值：
  0~250   = 正常数据
  251     = 参数不可用
  252     = 错误
  253     = 保留
  254     = 参数异常
  255     = 未请求

所以 valid_range = {250, 0} 意思是：
  原始值 0~250 → VALID（正常）
  原始值 > 250 → INVALID（错误码，不是真实数据）
```

### 大端小端（Byte Order / Endianness）

```
问题：一个数字占2个byte时，先存高位还是先存低位？

数字 0x22B8 存到2个byte里：

大端（Big-endian）：先存高位（人的阅读顺序）
  byte3 = 0x22, byte4 = 0xB8

小端（Little-endian）：先存低位（反过来）
  byte3 = 0xB8, byte4 = 0x22

生活比喻：
  大端 = "2026/05/14"（年→月→日，从大到小）
  小端 = "14/05/2026"（日→月→年，从小到大）

CAN/J1939 规定用大端 → config里 false = 不是小端 = 大端
1个byte的信号不需要关心大端小端（只有1个byte，不存在顺序问题）
```

### 信号占多少bit？取决于精度需求

```
 8bit =     256个值 → 范围小（够装大气压：0~0.125kPa）
16bit =  65,536个值 → 范围大（够装温度：-273~1762°C）
32bit = ~42亿个值   → 范围巨大（里程表等）

实际工程中 1/2/8/16/32 bit 覆盖99%的情况。
3字节（24bit）理论存在但很少见 — C++没有uint24类型，处理麻烦。
```

### Config的6个字段速查表

```
以 ambient_air_press 为例：

{
    false,            // ① is_little_endian：false = 大端
    0.F,              // ② offset：物理值 = 原始值 × resolution + offset
    {250, 0},         // ③ valid_range：{上限, 下限}，原始值在范围内才VALID
    0,                // ④ default_value：初始值
    &resolution,      // ⑤ resolution指针：分辨率（如0.0005）
    {8, 0, 0},        // ⑥ bit_layout：{bit长度, byte起始, bit起始}
};

大气压 vs 温度对比：
字段  含义          大气压          温度
①    字节序        false(大端)     false(大端)
②    offset        0              -273
③    valid range   {250, 0}       {64255, 0}
④    default       0              0
⑤    resolution    0.0005         0.03125
⑥    bit layout    {8, 0, 0}      {16, 0, 3}
```

### CAN模块的完整数据流

```
CAN总线 → [8字节原始数据]
              ↓
         GetFrame(buffer, &size) → 拿到buffer + 状态(UPDATED/NOT_UPDATED)
              ↓
         DecodeStatus(buffer, config) → 检查原始值是否在valid range内 → VALID/INVALID
              ↓
         DecodeData(buffer, config) → 按bit_layout提取 → × resolution + offset → 物理值
              ↓
         signal_object(物理值, VALID/INVALID) → 其他模块通过Ref()拿去用
```

### C++语法补充：const_cast

```
const_cast<NetworkProxy&>(proxy_).GetFrame(buffer, &size);

读法（从内往外剥）：
  ① proxy_                    → 拿到proxy_
  ② const_cast<...>(proxy_)   → 去掉const限制
  ③ .GetFrame(buffer, &size)  → 调用GetFrame
  ④ proxy_state =             → 存返回值

为什么需要const_cast？
  proxy_ 声明为 const → 不能调用非const方法
  GetFrame() 没声明const → 需要const_cast绕过
  = 库设计的小瑕疵，实际工程中常见
```

### CAN ID 的分解（29-bit Extended ID）

```
CAN ID 就是4个字节拼起来的：

0x 00 FE F5 00
   ①  ②  ③  ④

①  优先级 + 保留位 + 数据页（第一字节）
②  PF（PDU Format）= 消息大类
③  PS（PDU Specific）= 具体编号 or 目标地址
④  SA（Source Address）= 发送者地址
```

### 从 CAN ID 提取 PGN

```
扔掉①和④，留中间两字节：

0x 00 FE F5 00  →  PGN = 0xFEF5 = 65269 = Ambient Conditions
0x 00 F0 04 00  →  PGN = 0xF004 = 61444 = EEC1（发动机控制器）
0x 0C F0 04 01  →  PGN = 0xF004（实际总线上的帧，优先级3，地址1）

代码里用 0x00...00（优先级和地址清零）= 模板/掩码，只匹配PGN部分。
```

### PF ≥ 240 vs PF < 240（广播 vs 点对点）

```
PF ≥ 240 (0xF0)：广播消息
  → PS = PGN的一部分 → PGN = PF拼PS
  → 你代码里的两个模块都是这种

PF < 240：点对点消息
  → PS = 目标设备地址（被占用了）
  → PGN = 只有PF（PS强制填00）
  → 例：CAN ID 0x00AC3B01 → PF=0xAC(172) < 240 → PGN = 0xAC00

PS 一字两用：广播时是PGN编号，点对点时是目标地址。
```

### PGN 和 SPN 的关系

```
PGN = 一帧消息的编号（"这封信叫什么"）
SPN = 一帧里每个信号的编号（"信里每段话的编号"）

关系是 J1939 标准文档规定的，没有计算公式，只能查表。

PGN 0xFEF5（环境条件）：
  ├─ SPN 108 = 大气压       → config里的 ambient_air_press
  └─ SPN 171 = 环境温度     → config里的 ambient_air_temperature

PGN 0xF004（EEC1）：
  ├─ SPN 190 = 发动机转速   → config里的 engine_speed
  └─ SPN 899 = 扭矩模式等   → 代码没用到

代码里不会出现 SPN 编号，KDOG2工具已经把查表结果翻译成了config变量名。
```

### CAN ID vs PGN vs SPN 类比

```
CAN ID  = 快递面单（含发件人、优先级等完整标识）
PGN     = 包裹类型（"天气数据包"）→ 识别消息内容
SPN     = 包裹里每件物品的编号 → 识别具体信号

CAN ID → 提取PGN → 找对应config → 按config解码 → 得到物理值
```

### Config = 处理规则，不是数据

```
config文件 = 说明书（怎么处理数据） → 编译时固定，永远不变
buffer     = 实际数据（运行时从总线收到的） → 每个周期都可能不同

config里没有任何实际数值，只有"去哪取、怎么算"的规则。
```

### 三个文件的关系总结

```
config（规则）──┐
               ├──→ 接线文件(instantiation) ──→ 构造对象
.h（声明）─────┘         │
                         │ 把 config 引用传给对象
                         ▼
                  .cc（实现）用 config 里的规则处理 buffer

config = J1939标准表的代码化摘抄
.h     = 变量和方法的声明
.cc    = 具体取数据+转换的实现逻辑
接线文件 = 把config和类连接起来
```

### 参考资料

```
CSS Electronics — J1939 Explained（已验证可用）：
https://www.csselectronics.com/pages/j1939-explained-simple-intro-tutorial
- 英文，图文并茂，免费
- 包含 EEC1 (PGN F004) 解码实例，和代码里的 config_0x00F00400.h 完全对应
- 包含 signal range 表（251-255特殊值），和 valid_range 概念对应
```

### CAN模块 vs Practice模块对比

```
你写过的模块：                    CAN模块：
输入 = 别的模块的signal_object    输入 = CAN总线8字节原始数据
处理 = if/else/状态机             处理 = 解码(提取bit→乘分辨率→加偏移)
输出 = signal_object              输出 = signal_object（一模一样）

CAN模块只是多了一步"原始字节→物理值"的转换。
转换完之后，输出的signal_object和你写的模块一模一样。
```

---

## 十八、学习路线记录

### 已完成
- [x] 理解四层架构（含 utility/ 共享库的位置）
- [x] 理解 Manager 树形结构
- [x] 理解 INSTANTIATION 宏
- [x] 理解 instantiation_app.h 总装配
- [x] 理解 instantiation_app_userland.h 接线
- [x] 理解模块固定代码模式（有Config版 vs 无Config版）
- [x] 理解 .h 和 .cc 的对应关系（声明 vs 实现、Function:: 前缀、初始化列表）
- [x] 理解模块间通信（XxxRef → 接线 → GetValue）
- [x] 读懂8个模块（4种基本模式 + 三相状态机集大成模块）
- [x] 理解 Config 机制和 config_app.h
- [x] 理解多输出模块（一个模块输出多个信号）
- [x] 理解正负速度值和方向判断
- [x] 理解操作意图 vs 物理现实（安全关键系统通用原则）
- [x] 理解多信号交叉验证
- [x] 理解 IncrementTimer 源码（Update/Clear/IsTimeUp/IsTimeUpNow）
- [x] 理解 IncrementTimer 溢出保护
- [x] 理解 IsTimeUp vs IsTimeUpNow（持续信号 vs 脉冲信号）
- [x] 理解防抖动/debounce 模式
- [x] 理解时序逻辑 vs 组合逻辑
- [x] 理解 C++ 编译与链接过程（.cc→.o→.abs→.mot2）
- [x] 理解 #include 和链接的区别
- [x] 理解 .d 依赖文件（用来找源文件位置）
- [x] 理解 C# AppBuilder 工具 vs C++ 芯片代码的区别
- [x] 理解 C 结构体初始化语法（花括号嵌套 = struct 嵌套）
- [x] 理解 /* */ 注释标签（config_app.h 中的写法）
- [x] 理解嵌套类型的 :: 写法（utility::IncrementTimer::Config）
- [x] 理解 using 简化命名空间
- [x] 理解成员变量命名规律（参数 config → 成员 config_ 加下划线后缀）
- [x] 理解 Manager 树的完整创建链（instantiation.h → middleware创建根节点 → app创建子节点）
- [x] 理解 Manager 类两个构造函数（无参=根节点，有参=子节点绑定父节点）
- [x] 理解 instantiation.h 总入口文件的 include 顺序（middleware 先于 app）
- [x] 理解工程设计原则：操作意图 ≠ 物理现实（挡位 vs 实际速度方向）
- [x] 理解工程设计原则：多信号交叉验证（保守策略，宁可误判不漏判）
- [x] 理解工程设计原则：防抖动防止噪声导致误动作
- [x] 理解信号三层含义：原始硬件→物理量→状态判断
- [x] 认识常见信号：FNR挡位、车速、转速、油温、刹车行程、驻车制动等
- [x] 理解状态机模式（switch-case + 枚举状态迁移）
- [x] 理解边沿检测（Edge Detection）：持续信号→瞬间信号，只触发1个周期
- [x] 理解信号对象 = 容器模式（枚举值 + VALID/INVALID，GetValue拆包，构造函数装包）
- [x] 理解信号类型分两处定义（中间件层 vs 应用层，SignalObjects.cs vs SignalObjectcs.cs）
- [x] 理解 INVALID 不在业务层产生（wlh 模块永远输出 VALID）
- [x] 方向B：完成数据流全链路追踪（revolution 从传感器到 is_stop 到 17 个下游模块）
- [x] 理解冗余传感器设计（A/B 双传感器测同一根轴）
- [x] 理解跛行回家模式（Limp Home）：传感器全坏→用估算值
- [x] 理解信号层次2内部也有多步处理（诊断选择→故障切换）
- [x] 理解单一职责流水线设计（4个模块各做一步）
- [x] 掌握接线文件逆向追踪方法（找 INSTANTIATION → 看参数来源 → 重复）
- [x] 搜索技巧：搜"实例名,"(带逗号)精确定位INSTANTIATION，搜"实例名."找消费者
- [x] 发现反馈回路：vehicle_completely_stop → limp_home_permit → switch_function → 回到起点
- [x] 完整记录17个下游消费者（具体模块名+参数名+用途）
- [x] 理解 INSTANTIATION 中只有输入没有输出（参数=构造函数参数=输入，输出在别人的INSTANTIATION里）
- [x] 理解模块.h/.cc 不互相include（只include信号类型/工具类/框架类，不include其他业务模块）
- [x] 理解松耦合的实现方式：接线文件是唯一知道"谁连谁"的地方
- [x] 方向C：理解构建系统（4个库 + 链接 → .mot）
- [x] 理解 Shell 构建脚本（module_update.sh → module_wlh_update.sh 流程）
- [x] 理解 Makefile 结构（makefile + user.mk + mkmodule_wl.mk）
- [x] 理解交叉编译器 cxrh850（PC上编译，RH850上运行）
- [x] 理解多机型共用代码机制（config_tl/ 目录区分机型）
- [x] 掌握日常开发流程（改 .cc → sh module_update.sh → 烧写 .mot）
- [x] 理解三相状态机（transition/entry/do 三步分离执行）
- [x] 理解 entry vs do 的分工（一次性初始化 vs 每周期持续执行）
- [x] 理解全局计时器 vs 状态内计时器（不同生命周期管理）
- [x] 理解单次触发标志（one-shot flag）：变true后永不回false
- [x] 理解安全层 override 模式（不干预状态机逻辑，只在最后拦截输出）
- [x] 理解 protected virtual 扩展点（基类默认实现，子类按需覆盖）
- [x] 理解松手检测（release detection）：检测下降沿区分点按和长按
- [x] 理解双条件退出门控（两个条件同时满足才允许状态退出）
- [x] 理解 IncrementTimer 的 time_up_time_ 成员（允许运行时动态修改阈值）
- [x] 理解模块上下游串联（Module 7 边沿检测 → Module 8 雨刷控制）

**April 13 Q&A 新增掌握：**
- [x] 理解 const & 成员 vs 非const非& 成员的本质区别（"看别人的" vs "自己的"）
- [x] 理解构造函数三种初始化模式（引用绑定、对象创建、值初始化）
- [x] 理解 Config（纯数据/参数） vs IncrementTimer（有状态工具/对象）的区别
- [x] 理解 inner class 用于命名空间隔离（避免枚举名冲突）
- [x] 理解同名 Type 枚举在不同类中不冲突（项目约定：类名::Type）
- [x] 理解变量名倒着读技巧（is_wiper_sw_turned_off_ = 开关曾经松开过吗）
- [x] 理解历史标志(past) vs 当前计时器(now) 不矛盾（one-shot flag + 实时计时器）
- [x] 理解 ON_OFF 松手检测的设计理由（要等松手才知道是点按还是长按）
- [x] 理解星形拓扑状态机（Star Topology）：所有状态经过中央枢纽
- [x] 理解星形拓扑设计原则："不需要 + 不安全 + 更复杂 = 不做"
- [x] 理解 Manager 循环调用机制（OS→user_main→Manager.Update→module.Update，10ms一次）
- [x] 理解 Polling 轮询概念（没有真正的等待，只有每周期检查一次）
- [x] 理解协作式调度（Cooperative Scheduling）：每个模块快速执行+返回
- [x] 理解 WASHER_AND_WIPER_ACTIVE 只开 washer_relay 的硬件原因（继电器并联驱动）
- [x] 理解 IncrementTimer::Update() 源码逐行逻辑（溢出保护 + 电平/边沿判断）
- [x] 能独立绘制正确的状态转移图（已验证）

**May 14 CAN/J1939 通信层学习：**
- [x] 理解CAN帧结构（帧ID + 8字节数据，广播机制）
- [x] 理解J1939 = CAN之上的应用层协议（规定8字节里装什么）
- [x] 理解PGN（参数组编号）和SPN（具体参数编号）概念
- [x] 理解物理值转换公式：物理值 = 原始值 × resolution + offset
- [x] 理解valid range（J1939的251-255是错误码/保留值）
- [x] 理解大端小端（字节序）：大端=先高后低，J1939用大端
- [x] 理解bit数决定精度范围（8bit=256值, 16bit=65536值）
- [x] 理解Config 6个字段含义（endian/offset/valid_range/default/resolution/bit_layout）
- [x] 理解NetworkProxy = CAN帧搬运工（GetFrame获取8字节）
- [x] 理解const_cast（去掉const限制，绕过库设计瑕疵）
- [x] 理解C++长语句阅读顺序（从最内层往外剥）
- [x] 完整阅读CAN模块3个文件（config/类声明/Update实现）
- [x] 理解CAN模块 vs Practice模块的区别（只多了原始字节→物理值转换步骤）

**May 15 Day5 — CAN模块.cc精读：**
- [x] 理解signal_object初始化为INVALID的原因（没收到数据前不可信）
- [x] 理解static_cast<float32>(0)（明确类型转换，等价于0.F）
- [x] 理解模板参数<float32, uint8>（输出类型+原始数据类型，适配不同bit长度）
- [x] 理解config通过接线文件传入.cc（.h是两边共同的"合同"）
- [x] 理解size是GetFrame强制要求的输出参数但模块没用到
- [x] 理解frame_receive_state_（通信健康状态，让下游知道帧有没有收到）
- [x] 理解两个if/else不能合并（每个信号的so_state独立，valid_range不同）
- [x] 理解DecodeData嵌套在Pressure构造里的阅读顺序（从内往外剥）
- [x] 理解CAN ID 4字节分解（①优先级 ②PF ③PS ④发送者地址）
- [x] 理解PGN提取方法：扔掉①④，留中间②③拼起来（PF≥240时）
- [x] 理解PF≥240=广播（PS是PGN一部分） vs PF<240=点对点（PS是目标地址）
- [x] 理解代码里CAN ID清零（0x00...00）= 模板/掩码，只匹配PGN
- [x] 理解PGN和SPN的关系：标准文档规定，没有计算公式，只能查表
- [x] 理解CAN帧 = CAN ID（标签）+ 8字节数据（内容），ID不含测量值
- [x] 理解resolution存在的原因：总线只能传整数，用分辨率做小数↔整数转换
- [x] 理解config = 处理规则（编译时固定），buffer = 实际数据（运行时变化）
- [x] 总结三文件关系：config(规则)+.h(声明)→接线文件→.cc(实现用config处理buffer)
- [x] 找到J1939参考资料：CSS Electronics J1939 tutorial（含EEC1解码实例，和第二个模块完全对应）

**May 18 Day6 — 复习編写練習 + CAN模块继续 + 共通モジュール学習：**
- [x] engine_overheat_monitor：两相状态机 + 2个HysteresisFloat32 + IncrementTimer，3轮通过
- [x] travel_lock_controller：边沿検出 + 迟滞器 + timer + 状態機 + diag，2轮通过
- [x] 主動加了 is_diag_normal 检查（spec没要求，安全思维）
- [x] 理解局部变量 vs 成员变量的生命周期差异（边沿検出的prev必须跨周期保留）
- [x] 理解 Update()/Clear()/IsTimeUp() 三者在时间线上的分工
- [x] Clear位置再次確認：进入新状态时Clear = "从现在开始计时"
- [x] CAN模块F00400完全読了（config + .h + .cc）、DecodeStatus vs DecodeData理解
- [x] engine_speed_ 信号下游追跡：12個の消費者特定（so_switch×6 + RTCDB送信 + snap変換 + shift count等）
- [x] so_switch_model 源码阅读：系统最常见共通モジュール，仅1行核心逻辑（三目运算）
- [x] 理解 C++ template 为什么必须把实现写在 .h 里（编译时类型未知→无法生成机器码）
- [x] 理解 template vs C# 泛型 vs C# 反射的区别（编译时代码生成 vs JIT vs 运行时查询）
- [x] 系统全12领域梳理：common/diagnosis/engine/vehicle_information/vehicle_observer/io/fan/steering/auxiliary_equipment/personal_customizer/etc/calibrate_service

**May 19 Day7 — 練習 + 共通/診断モジュール学習：**
- [x] idle_speed_controller：两相状态机 + HysteresisFloat32 + IncrementTimer + float演算 + diag，**1轮通过**
- [x] 提交前review发现3个问题（getValue大小写、Update传参、#include遗漏）→ 修正后1次编译通过
- [x] 理解局部变量必须每条路径都赋值（不能靠"上次的值"，和prev_fnr_同一个道理）
- [x] 主动质疑spec不完整（IDLE_UP诊断处理、进入状态时的output值）→ 逻辑自主思考能力提升
- [x] DiagFailureJudge + DiagNormalJudge 源码阅读：互为镜像（一个flag=false找true，一个flag=true找false）
- [x] MaskOr + MaskAnd 源码阅读：故障遮蔽组合（OR=任一成立就遮蔽, AND=全部成立才遮蔽）
- [x] 理解完整诊断链路：故障検出→Array打包→DiagFailureJudge翻译成bool→so_switch信号源切換→下游透明使用
- [x] 理解 FailureSignal 三状态：NORMAL / OCCUR / RECOVER（OCCUR和RECOVER都算"不正常"）
- [x] 理解 Array = 原始C数组 + 长度的包装，让for循环能用GetSize()知道遍历几个
- [x] 理解 so_switch 一个输出被多个下游共享（const&引用，多次读取不影响值）
- [x] 具体验证：engine_speed 的 so_switch 输出被7个下游模块读取，做不同的事（低转速遮蔽、风扇故障判定等）
- [x] 理解马达转速(tm_input_rev)没有so_switch — 本机传感器直接测量，不经过CAN通信，无需故障切换

**so_switch_model（SO切替モジュール）知识点：**
- 文件：module/wlh/include/so_switch_model_function.h（template、只有.h没有.cc）
- 核心：`output_ = input_.GetValue() ? true_so_ : false_so_;`
- 用途：故障时自动切换到安全値（フェイルセーフ）
- 设计模式：不在业务模块内判断故障，而是在信号链路中间插入switch节点，下游透明
- 模板调用语法：`Function<RotationalSpeed> name(input, true_so, false_so, manager);`
- 一个so_switch的OutputRef()可以被多个下游模块读取（const&引用共享）
- 每个信号有自己的安全值（engine=825RPM，其他信号不同）

**診断モジュール（diagnosis）知识点：**
- DiagFailureJudge：遍历FailureSignal数组 → 有OCCUR/RECOVER就输出 IsFailure=true
- DiagNormalJudge：遍历FailureSignal数组 → 全部NORMAL才输出 IsDiagNormal=true（互为取反）
- MaskOr：遍历FaultTransitionMaskOrder数组 → 任一ON就输出ON（用于故障遮蔽组合）
- MaskAnd：遍历FaultTransitionMaskOrder数组 → 全部ON才输出ON（有break提前退出）
- 遮蔽(マスク)用途：启动时/低转速时暂时不报告故障（因为此时故障可能是假的）

**完整诊断信号链路（engine_speed实例）：**
```
dg_failure_engine_comm_error（故障検出）
  → failure_signal数组（Array打包，长度1）
    → DiagFailureJudgeFunction（翻译成bool）
      → so_switch（true→825RPM安全値 / false→CAN真実値）
        → 7个下游模块（各自做不同判断，透明使用）
```

**C++ template 要点：**
- `template<typename SoType>` → SoType 是占位符，不是真实类型
- 具体类型在使用处才确定（如 `Function<RotationalSpeed>`）
- 编译器对每个具体类型生成一份独立代码（物理复制）= 零运行时开销
- 为什么必须在 .h：编译 .cc 时 SoType 未知→无法生成机器码；放 .h 让使用者 include 后当场生成
- 类比：template = 模具图纸（材料待定），使用者指定"用钢的"才能生产成品

**April 28 从零编写练习 — parking_lamp_model_practice：**
- [x] 从零编写三相状态机模块（OFF→WAIT→ON）+ HysteresisFloat32 + IncrementTimer
- [x] 经过3轮修正（21个错误），最终 GHS 编译通过 BUILD SUCCESS
- [x] 掌握 GHS C++03 限制：不支持 enum class → 用 plain enum + STATE_ 前缀

**May 7 编写练习 — fan_control（热身）+ overspeed_warning（中等挑战）：**
- [x] fan_control：简单模块（HysteresisFloat32 + if/else），仅1个错误（Manager大小写），1轮通过
- [x] overspeed_warning：两相状态机 + HysteresisFloat32 + IncrementTimer（最短保持时间模式）
- [x] overspeed_warning 第一轮10个错误 → 第二轮仅1个错误（成员变量漏下划线），2轮通过
- [x] 掌握 Config 只放配置参数，输入信号放构造函数参数（不再混淆）
- [x] 掌握迟滞器/timer 每周期都要 Update（放 switch 之前）
- [x] 掌握有迟滞器就不要自己比较阈值 → 用 GetState() 代替直接比较
- [x] 掌握 #include 依赖关系：signal_object.h 是基类不需要直接 include，量产模块只 include 具体类型
- [x] 掌握两相状态机 + timer保持模式（timer不是"等待进入"而是"保持最短时间"）

**May 8 一次通过挑战（失败）— hydraulic_temp_warning（不看参考代码）：**
- [x] 目标：不看任何旧代码，从空文件一次编译通过 → **未达成，实际经过13轮编译**
- [x] 8个独立错误，暴露了"以为会了但实际没稳"的知识点
- [x] **好消息**：状态机逻辑/switch结构/Update位置 = 零错误，逻辑层面已内化
- [x] **坏消息**：const回归（parking_lamp时学过但又犯了）、Manager值传递vs引用不清楚
- [x] 中间卡在第6-8轮（同样的错误连编3次）→ 说明有些错误在"试"而不是"懂了改"
- [x] 关键教训：overspeed_warning的低错误数可能因为参考了fan_control代码，去掉参考暴露真实水平

**May 8 hydraulic_temp_warning 的8个独立错误详细记录：**
```
1. 忘记 #include "increment_timer.h"         → include遗漏（新错误）
2. Config里 IncrementTimer::Config 语法写错   → 不熟悉嵌套类型语法（新错误）
3. Manager 值传递 → 应该是 Manager& 引用传递   → ★核心概念错误（新错误）
4. warning_lamp_output_ 初始化写了两次         → 初始化列表混乱（类似旧错误）
5. hydraulic_temp 漏下划线 → hydraulic_temp_   → ★老问题回归（overspeed也犯过）
6. getValue() → GetValue()（大小写）          → API大小写不熟（新错误）
7. warning_lamp_output_ 声明为 const          → ★const回归（parking_lamp犯过）
8. .h 和 .cc 构造函数签名不一致               → 同步错误（新错误）
```

**May 8 多输出模块练习（有参考）— engine_temp_monitor：**
- [x] 新模式：一个模块两个输出（BoolSignal暖机状态 + ElectricalOnOffState过热警告灯）
- [x] 两个 HysteresisFloat32（暖机迟滞器+过热迟滞器）+ IncrementTimer
- [x] 组合逻辑（暖机判断）+ 状态机（过热警告）混合在同一个 Update() 里
- [x] 约3轮通过（初始10错→修正→通过）
- [x] Manager& 引用传递终于写对了 ✅
- [x] 下划线全部正确 ✅
- [x] 仍然犯了：.h/.cc签名不一致（第4次）、Update()返回值误用

**May 8 无参考挑战 #2 — fuel_pressure_warning（不看任何代码）：**
- [x] 两相状态机 + HysteresisFloat32 + IncrementTimer（和hydraulic相同难度）
- [x] 初始9个错误 → **全部是语法/拼写，零逻辑错误** ✅
- [x] **2轮通过**（写→修→通过）← 比hydraulic的13轮大幅进步！
- [x] Manager& ✅、const ✅、下划线 ✅、Update()没当bool用 ✅
- [x] 仍然犯了：.h/.cc签名不一致（第5次。config参数漏了）、GatValue拼写、多余逗号/分号
- [x] 关键进步：所有"概念型错误"已消灭，剩下的全是"打字精度"问题

**May 11 Day1 无参考挑战 #3 — brake_oil_warning：**
- [x] 两相状态机 + HysteresisFloat32 + IncrementTimer
- [x] 初始9个错误 → 2轮通过
- [x] 下划线 ✅、const ✅、GetState()/GetValue() ✅
- [x] Manager& 引用传递 — **又犯了**（fuel_pressure写对过，这次又丢了）→ 需要加入检查清单
- [x] include 拼写错误2个（incement_timer、electircal_on_off_state）
- [x] STATE_ON 里漏了 output 赋值（逻辑遗漏1个）

**May 11 Day1 无参考挑战 #4 — exhaust_temp_warning：**
- [x] 两相状态机 + HysteresisFloat32 + IncrementTimer
- [x] **仅1个错误**：class 定义末尾 `}` 漏了分号 `;`
- [x] **2轮通过**（第1次差一个分号就是一次通过）
- [x] Manager& ✅、下划线 ✅、const ✅、签名一致 ✅、逻辑完全正确 ✅
- [x] 用了 `else if` 代替两个独立 `if`（比之前的写法更好）
- [x] 接近一次通过的里程碑

**May 12 Day2 无参考挑战 #5 — seat_heater_control（三相状态机）：**
- [x] 三相状态机（STATE_OFF → STATE_WAIT → STATE_ON）+ IncrementTimer
- [x] 场景：座椅加热防误触，开关ON后等待确认，timer到期才启动加热
- [x] **一次编译通过！零错误！** ← 里程碑达成
- [x] Manager& ✅、.h/.cc签名 ✅、下划线 ✅、const ✅、include ✅、class末尾分号 ✅
- [x] 用了 else if 链式判断（诊断异常 → 开关OFF → timer到期），逻辑清晰
- [x] 理解了"状态≠输出"：3个状态但只有2种输出值（OFF和WAIT对外都是OFF）

**May 12 Day2 无参考挑战 #6 — cabin_climate_control（三相状态机 + 多输出 综合最高难度）：**
- [x] 三相状态机 + HysteresisFloat32 + IncrementTimer + **2个输出**（cooling_relay + status_lamp）
- [x] 场景：驾驶室温控，温度过高→等待确认→启动制冷，status_lamp在WAIT和COOLING都亮
- [x] **仅1个错误**：`cabin_temp.GetValue()` 漏下划线 → `cabin_temp_.GetValue()`
- [x] 2轮通过（编译前审查发现错误→修正→通过）
- [x] 2个output变量分开声明、3个case各有2个赋值、2个getter — 全部正确
- [x] 用了 `||` 合并判断条件（`!diag || !overhigh → OFF`），比之前的写法更简洁

**May 12 Day2+Day3 — Git/GitHub 全流程学习：**

**May 13 Day3 — STAR故事 + 技能分级 + brake_press_detector：**
- [x] STAR故事4个全部展开成完整英文稿（骨架+90秒可背诵版），写入笔记21.5
- [x] 技能分级定义整理（Strong/Working/Familiar 含义+类比+面试回答模板），写入笔记21.9
- [x] 关键技能诚实表述：RH850（应用层）、CAN/J1939（Familiar）、RTOS（概念理解）各有面试应答模板
- [x] ATS关键词更新（加Git、hysteresis、edge detection等）

**May 13 Day3 无参考挑战 #7 — brake_press_detector（边沿检测，新模式）：**
- [x] 新模式！边沿检测（Edge Detection）：不是状态机，用 prev_ 变量对比检测变化瞬间
- [x] 场景：制动踏板踩下/松开的一瞬间输出ON（上升沿+下降沿检测）
- [x] **1轮编译通过！** 连续第3个一次编译通过（seat_heater→cabin_climate→brake_press_detector）
- [x] 发现 `=` vs `==` 致命逻辑bug — 编译不报错但运行全错（C/C++经典陷阱）
- [x] 不需要Config/HysteresisFloat32/IncrementTimer/状态机 — 最简结构
- [x] 学到：prev_变量模式 = IncrementTimer里的 time_up_now_ 原理

**May 13 Day3 无参考挑战 #8 — engine_stall_counter（计数器+阈值，新模式）：**
- [x] 新模式！计数器/累计器：统计事件次数，达到阈值触发输出
- [x] 场景：发动机异常熄火累计≥3次，点亮维修指示灯
- [x] **1轮编译通过！** 连续第4个一次编译通过
- [x] Config回来了（threshold阈值），uint32计数器
- [x] 审查时发现：中文逗号、config缺下划线、ON/OFF/VALID缺命名空间、&遗漏、重复条件 — 全部修正后编译一次通过
- [x] 逻辑简洁：计数+比较，不需要状态机/定时器/prev变量

**May 14 Day4 无参考挑战 #9 — operation_mode_arbiter（多输入优先级仲裁，新模式）：**
- [x] 新模式！多输入优先级：3个输入（emergency/manual/auto）→ 4个输出灯
- [x] 场景：紧急停止 > 手动操作 > 自动模式 > 待机，if-else if 顺序=优先级
- [x] **1轮编译通过！** 连续第5个（拼写修正：Booisignal→BoolSignal, MoudleInterface→ModuleInterface）
- [x] 逻辑bug发现：else if 里读了自己的输出（manual_lamp_output_）而不是输入（manual_request_）→ 已修正
- [x] 教训：判断条件要读输入信号，不是读自己的输出

**May 14 Day4 无参考挑战 #10 — drive_mode_controller（模式切换+Config+混合输出类型）：**
- [x] 模式切换：ECO/NORMAL/POWER 三种驾驶模式，不同模式输出不同数值
- [x] 新信号类型：Velocity（float32值）— 第一次使用
- [x] 混合输出：Velocity（转速上限）+ ElectricalOnOffState（POWER指示灯）
- [x] Config有3个float32参数（eco/normal/power_speed_limit）
- [x] **5轮编译通过**：.h里power_switch_类型写错(signal_object::power_switch)→Velocity/ElectricalOnOffState搞混→else赋值语法错（漏= Velocity）→每个分支漏给output赋OFF
- [x] 感悟：**自己写过才理解code review的重要性** — 概念全对但拼写/类型名手误难免
- [x] 面试素材："Code review isn't about questioning competence, it's about catching what the human eye naturally misses"

**May 14 Day4 无参考挑战 #11 — hydraulic_pump_controller（定时器组合+4状态，最高难度）：**
- [x] 新模式！3个IncrementTimer协作（startup_delay + min_run + cooldown）
- [x] 4状态状态机：IDLE → STARTUP_DELAY → RUNNING → COOLDOWN → IDLE
- [x] 场景：液压泵启动保护（延迟启动+最短运行+冷却等待，防止频繁开关损坏机械）
- [x] Config包含3个timer config → 构造函数初始化3个timer对象
- [x] 编译通过（多轮：类型错误 IncrementTimer::Config vs IncrementTimer）
- [x] **逻辑bug发现**：Clear放在Update开头 → 每周期清零 → timer永远计不上去
  - 正确做法：Clear在**状态转移那一刻**做，"准备好下一阶段的timer"
- [x] 理解了Config vs Timer的区别：Config是设定值（const），Timer是会跑的对象
- [x] 理解了接线文件/config_app.h/模块三者的关系：模块定义形状→config填数字→接线文件组装

**May 14 整日总结**：
- 完成3个新模块（operation_mode_arbiter, drive_mode_controller, hydraulic_pump_controller）
- 学到3种"编译过但逻辑错"的bug：`=`vs`==`、读output而非input、Clear位置错误
- 感悟：**编译通过≠逻辑正确**，这就是code review和单元测试存在的意义
- 理解了系统架构：模块→Config→接线文件→依赖注入的完整闭环
- CAN模块阅读：PGN 0xFEF500（Ambient Conditions），理解了3个文件（config/类声明/Update实现）
- CAN/J1939基础补充：帧结构、PGN/SPN概念、resolution+offset公式、大端小端、valid range、const_cast

**May 15 整日总結**：
- 完成CAN模块.cc精読（DecodeData嵌套阅读、signal_object初始化、模板参数、frame_receive_state_）
- J1939深度理解：CAN ID 4字节分解（优先级/PF/PS/SA）、PGN提取方法、PF≥240广播 vs <240点对点
- 理解PGN与SPN关系（标准文档查表，无计算公式）、CAN ID=标签 vs 数据=内容
- 理解resolution存在的本质原因（总线只传整数，需做小数↔整数转换）
- 总结三文件关系：config(规则)+.h(声明)→接线文件→.cc(用config处理buffer)
- 找到可用的J1939参考资料（CSS Electronics，含EEC1解码实例）
- 第2个CAN模块config已读（no_status_codec, engine_speed），.h/.cc待读

**May 12 Day2+Day3 — Git/GitHub 全流程学习：**
- [x] 完成 git init → config → add → commit → remote → push 完整流程
- [x] 理解核心概念：工作区 → 暂存区(add) → 本地仓库(commit) → 远程仓库(push)
- [x] 理解 branch/merge/checkout 分支操作（概念理解，本地实操过）
- [x] 创建 .gitignore 排除编译产物（*.o, *.obj, *.lib, build.bat）
- [x] 将练习代码上传到 GitHub 私有仓库（已完成，7个模块）
- [x] 理解公司代码 vs 个人代码分离原则（不上传公司信息、编译器路径、copyright）
- [x] 日常工作流掌握：`git add . → git commit -m "xxx" → git push`

**May 11 IncrementTimer 源码深度阅读（Utility/1_0_0/timer/increment_timer.cc）：**
- [x] 读懂了 IncrementTimer 完整实现（约100行）
- [x] 理解 Config 三参数：sampling_time(每周期增量), initial_time(初始值), time_up_time(到期阈值)
- [x] 理解 time_up_ vs time_up_now_ 的区别：
  - `time_up_` = 电平检测（Level Detection）：到期后一直 true
  - `time_up_now_` = 边沿检测（Edge Detection）：只在刚到期那一个周期 true
  - 实现技巧："先检查旧值，再更新"（标准边沿检测写法）
- [x] 理解溢出保护（Overflow Protection / Defensive Programming）：
  - `if (time_ > (uint_max - sampling_time))` → 钳位到最大值，防止加法溢出
  - uint32 最大值 ÷ 10ms ≈ 497天，建设机械真的可能连续运行这么久
- [x] 理解 Config 外部注入机制：config_app.h 里设定具体数值（如 3000U = 3秒），模块代码不硬编码

### 编写练习错误趋势

| 练习 | 日期 | 难度 | 错误数 | 编译轮次 | 有参考？ | 主要错误类型 |
|------|------|------|--------|---------|---------|------------|
| parking_lamp | 4/28 | 中等（三相状态机） | 21个 | 4轮 | 有 | 拼写、const滥用、enum前缀、结构混乱 |
| fan_control | 5/7 | 简单（if/else） | 1个 | 1轮 | 有 | 大小写（Manager→manager） |
| overspeed_warning | 5/7 | 中等（两相状态机） | 10→1个 | 2轮 | 有 | 初始化列表config_/config、迟滞器Update位置、阈值比较方式 |
| hydraulic_temp_warning | 5/8 | 中等（两相状态机） | 8个 | **13轮** | **无** | Manager&遗漏、const回归、include遗漏、初始化列表重复、API大小写 |
| engine_temp_monitor | 5/8 | 中等（多输出） | 10个 | ~3轮 | 有 | .h/.cc签名、Update()返回值误用 |
| fuel_pressure_warning | 5/8 | 中等（两相状态机） | 9个 | **2轮** | **无** | .h/.cc签名（第5次）、拼写GatValue、语法逗号分号 |
| brake_oil_warning | 5/11 | 中等（两相状态机） | 9个 | **2轮** | **无** | Manager&回归、include拼写、output赋值遗漏 |
| exhaust_temp_warning | 5/11 | 中等（两相状态机） | **1个** | **2轮** | **无** | class末尾漏分号（差一个字符就一次通过） |
| seat_heater_control | 5/12 | **高（三相状态机）** | **0个** | **1轮** | **无** | ★ 一次通过！零错误！ |
| cabin_climate_control | 5/12 | **最高（三相+多输出）** | **1个** | **2轮** | **无** | 成员变量漏下划线（cabin_temp→cabin_temp_） |
| brake_press_detector | 5/13 | **中等（边沿検出・新模式）** | **0个** | **1轮** | **无** | ★ 编译一次通过！（但有=vs==逻辑bug） |
| engine_stall_counter | 5/13 | **中等（计数器・新模式）** | **0个** | **1轮** | **无** | ★ 编译一次通过！连续第4个 |
| operation_mode_arbiter | 5/14 | **中等（多输入優先级・新模式）** | **0个** | **1轮** | **无** | ★ 連続第5个（拼写修正後）。逻辑bug：读了output而非input |
| drive_mode_controller | 5/14 | **中高（模式切換+Config+混合输出）** | **4个** | **5轮** | **无** | 类型搞混(Velocity/ElectricalOnOffState)、赋值语法、分支漏赋值 |
| hydraulic_pump_controller | 5/14 | **高（4状态+3个Timer协作）** | 数个 | 多轮 | **无** | Timer类型写成Config类型、Clear位置错误（逻辑bug） |
| engine_overheat_monitor | 5/18 | **中高（2迟滞器+timer+两相状態機）** | **5个** | **3轮** | **无** | `;`vs`,`、`:`vs空格、const&output、HysteresisFloat32漏传参数、Clear位置（第2次） |
| travel_lock_controller | 5/18 | **高（边沿検出+迟滞器+timer+状態機+diag）** | **4个** | **2轮** | **无** | Update声明漏、prev_fnr\_局部変数（逻辑bug）、下划線。**主動加diag检查** ✅ |
| idle_speed_controller | 5/19 | **中高（两相状態機+float演算+diag）** | **3个** | **1轮** | **无** | getValue大小写、Update传参、#include遗漏。review阶段修正，编译一次通过 ✅ |

**关键观察**：
- 无参考进步曲线：13轮 → 2轮 → 2轮 → 2轮 → **1轮** → 2轮 → **1轮** → **1轮** → **1轮** → 5轮 → 多轮 → **3轮** → **2轮** → **1轮**
- **趋势分析**：简单/中等模块稳定1轮通过，高难度模块（新类型+多timer）仍需多轮，但**全是拼写/类型手误，零概念错误**
- 新模式全部能独立完成：边沿检测、计数器、优先级仲裁、模式切换、定时器组合 ✅
- 概念型错误全部消灭：Manager&、const、Update()误用、签名不一致 → 连续正确
- **逻辑bug专题**（编译过但运行错）：`=`vs`==`、读output而非input、Clear位置、分支漏赋值
- **新感悟**：编译通过≠逻辑正确 → code review + 单元测试的意义

**已消灭的错误模式**：
- ~~enum class / Status:: 前缀~~ → 5/7 起零错误
- ~~函数嵌套（getter写在Update里面）~~ → 5/7 起零错误
- ~~拼写错误（Fucntion, signale_object）~~ → 基本消灭
- ~~状态机逻辑/switch结构~~ → 连续7个模块零逻辑错误 ✅
- ~~const 滥用~~ → fuel_pressure_warning 起已修正 ✅
- ~~Update() 当 bool 用~~ → fuel_pressure_warning 起已修正 ✅
- ~~.h/.cc 构造函数签名~~ → seat_heater + cabin_climate 连续正确 ✅
- ~~Manager& 值传递~~ → seat_heater + cabin_climate 连续正确 ✅
- ~~成员变量漏下划线~~ → 低频残留（cabin_climate 犯了1次），基本消灭

**仅存的低频问题**：
- 成员变量偶尔漏下划线（cabin_temp → cabin_temp_）— 纯手误
- include 文件名偶尔拼写错误 — 纯手速问题
- ⚠️ `=` vs `==` — 编译不报错的逻辑陷阱（brake_press_detector发现），加入检查清单
- ⚠️ Clear位置 — 应在进入新状态时Clear（hydraulic_pump_controller + engine_overheat_monitor 两次犯）
- ⚠️ prev变量必须是成员变量 — 局部变量每周期重置，边沿检测失效（travel_lock_controller发现）

**检查清单（最终版）**：
```
□ Manager& 是引用传递（不是值传递）？
□ .h/.cc 构造函数签名并排对比，逐字一致？
□ 所有成员变量带 _ 后缀？
□ Update() 大写开头？没当 bool 用？
□ GetValue() / GetState() 拼写正确？
□ 输出变量没加 const？
□ include 了所有用到的类型？（module_interface.h 也别忘）
□ 所有 include 文件名拼写正确？（逐个读一遍）
□ 参数分隔符是逗号，初始化列表分隔符也是逗号？
□ timer.Update() 在 switch 之前？
□ 每个 case 里都给 output 赋值了？
□ :: 双冒号没写成单冒号？
□ class 定义的最后 }; 有分号？
□ if 条件里的 == 没写成 =？（编译不报错的致命bug！）
□ 边沿检测的 prev 变量是成员变量？（不是 Update 里的局部变量！）
□ Clear() 在进入新状态时调用？（不是退出时！）
□ HysteresisFloat32::Update() 传了输入值？
□ .h 里声明了 void Update()？
□ 文件末尾有空行？
□ 局部变量每条路径都赋值了？（不会保留上周期值！）
```

### 新学到的嵌入式概念（5/11）

**电平检测 vs 边沿检测（Level vs Edge Detection）**：
- 电平检测：条件满足**期间**一直为true（`IsTimeUp()`）
- 边沿检测：条件从不满足→满足的**那一瞬间**为true（`IsTimeUpNow()`）
- 实现方法：先检查旧值 → 再更新 → 旧值false+新值true = 边沿
- 用途：电平检测→"到期后才允许灭灯"；边沿检测→"到期瞬间响一声蜂鸣"

**防御性编程（Defensive Programming）**：
- 溢出保护：`if (time_ > uint_max - sampling_time)` → 钳位到最大值
- 嵌入式必须考虑极端情况：uint32最大值 ÷ 10ms ≈ 497天连续运行
- 其他例子：除零保护、数组越界保护、信号异常保护（诊断不正常→不信任传感器）
- 面试用："I've studied defensive programming patterns in the codebase — overflow-safe counters, signal validity checks, and diagnostic-based failover."

**Config 外部注入**：
- 具体数值（如最短显示时间3秒=3000U）在 config_app.h 里配置
- 模块代码不硬编码任何阈值 → 修改参数不需要改模块代码
- 面试用："Separation of configuration from logic — similar to calibration parameters in automotive"

### 下一步建议
- [x] 方向A完成：4种常见模式全部掌握（组合逻辑、定时器防抖、多输出、状态机）
- [x] 方向B完成：数据流全链路追踪（传感器→设备驱动→诊断选择→故障切换→判断→下游）
- [x] 方向C完成：构建系统（Shell脚本→Makefile→cxrh850→4个.lib→链接→.mot）
- [x] 更多复杂模块：读懂三相状态机+多计时器模块（wiper_control_model_function）
- [x] 实战练习：**18个模块完成**（...→idle_speed_controller）
- [x] 多输出模块练习：engine_temp_monitor + cabin_climate_control + drive_mode_controller
- [x] 无参考一次编译通过：**seat_heater_control 三相状态机一次通过，零错误** ✅ 里程碑达成
- [x] 三相状态机无参考编写：seat_heater_control 1轮 + cabin_climate_control 2轮 ✅
- [x] IncrementTimer 源码深度阅读：边沿检测、溢出保护、Config注入
- [x] Git/GitHub 全流程学习：init → add → commit → push → 上传到GitHub ✅
- [x] 新模式掌握：边沿检测、计数器、多输入优先级、模式切换+混合输出、**定时器组合（3timer+4状态）** ✅
- [x] 理解接线文件/config_app.h/模块三者关系 ✅
- [ ] 面试准备：把练习成果整理成能讲的故事（STAR故事已写完英文稿，需练习口述）
- [ ] 定时器组合练习（多个定时器协作）
- [x] 读一个CAN解析模块（PGN 0xFEF500 Ambient Conditions）+ 补充CAN/J1939基础知识 ✅
- [ ] 追踪更多信号链路（如 CAN 总线信号的完整路径）

### 本周计划（5/11-5/16）

| 日 | 内容 | 状态 |
|---|------|------|
| Day 1 (5/11) | 无参考挑战 ×2 + IncrementTimer源码阅读 | ✅ 完成（brake_oil 2轮, exhaust 差1分号） |
| Day 2 (5/12) | 三相状态机无参考 + 综合挑战 + Git学习 | ✅ 完成（seat_heater **一次通过**！cabin_climate 1错/2轮。Git全流程+上传GitHub） |
| Day 3 (5/13) | STAR故事英文稿 + 技能分级 + 2个新模式练习 | ✅ 完成（4个STAR故事完整英文稿 + brake_press_detector + engine_stall_counter，均1轮通过） |
| Day 4 (5/14) | 新模式练习 ×3 + CAN模块阅读 + CAN/J1939基础 | ✅ 完成（operation_mode_arbiter 1轮 + drive_mode_controller 5轮 + hydraulic_pump_controller + CAN模块0xFEF500阅读 + CAN/J1939通信基础补充） |
| Day 5 (5/15) | CAN模块.cc精読 + CAN ID分解 + J1939深化 + 参考资料 | ✅ 完成 |
| Day 6 (5/18) | 復習練習×2 + CAN F00400完読 + 信号追跡 + so_switch + template学習 | ✅ 完成 |

**本周结束时的目标状态**：
- ✅ 无参考写两相状态机 → 稳定1-2轮通过 → **已达成**
- ✅ 无参考写三相状态机 → 3轮以内通过 → **已达成（1轮！超额完成）**
- ✅ Git 基本操作会用 → **已达成（含GitHub上传）**
- ✅ 新模式（边沿检测/计数器/优先级/模式切换）→ **已达成**
- ✅ STAR故事英文稿 → **已达成（4个完整版）**
- 🔲 能用2分钟介绍自己做了什么 → 需要口述练习（回家练）
→ **接近可投简历状态**

---

## 二十、Git/GitHub 完全指南（从零到上传）

> 2026-05-12 学习记录。从 `git init` 到成功 push 到 GitHub 的完整流程。

### 20.1 核心概念

```
工作区（Working Directory）    ← 你正在编辑的文件
  ↓  git add
暂存区（Staging Area）         ← "我确认这些修改要保存"
  ↓  git commit
本地仓库（Local Repository）   ← "拍一张快照，附上说明"
  ↓  git push
远程仓库（GitHub/GitLab）      ← "上传到云端，别人可以看到"
```

**类比**：
- `git add` = 把商品放进购物车
- `git commit` = 结账（保存到本地）
- `git push` = 寄出去（上传到 GitHub）

### 20.2 首次设置（只做一次）

```bash
# 1. 创建文件夹，放入代码
mkdir C:\MyPractice\embedded_modules

# 2. 进入文件夹
cd C:\MyPractice\embedded_modules

# 3. 初始化 Git 仓库
git init

# 4. 设置身份（用 GitHub 账号的用户名和邮箱）
git config user.name "你的GitHub用户名"
git config user.email "你的邮箱"

# 5. 创建 .gitignore（用记事本）
notepad .gitignore
# 内容：
# *.o
# *.obj
# *.lib
# build.bat

# 6. 暂存所有文件
git add .

# 7. 查看状态（确认没有奇怪的文件）
git status

# 8. 第一次提交
git commit -m "Add embedded practice modules"

# 9. 在 GitHub 网页创建新仓库（Private，不勾选任何初始化选项）

# 10. 关联远程仓库
git remote add origin https://github.com/你的用户名/仓库名.git

# 11. 推送
git branch -M main
git push -u origin main
# 第一次会弹出登录窗口，选 "Sign in with your browser" → 授权
```

### 20.3 日常使用（每次写完新模块）

```bash
# 把新文件复制到 Git 仓库文件夹里，然后：
cd C:\MyPractice\embedded_modules
git add .
git commit -m "Add xxx_module"
git push
```

**就这三行，每次都一样。**

### 20.4 其他有用的命令

```bash
# 查看当前状态（什么文件改了/新增了）
git status

# 查看提交历史
git log --oneline

# 查看改了什么
git diff

# 撤销某文件的修改（回到上次commit的状态）
git checkout -- file.cc

# 临时保存修改（不想commit但要切换工作）
git stash
git stash pop   # 恢复
```

### 20.5 分支操作（团队协作时用）

```bash
# 创建并切换到新分支
git checkout -b feature/new-module

# 切回主线
git checkout main

# 合并分支到当前分支
git merge feature/new-module

# 推送分支到远程
git push origin feature/new-module
```

**个人练习不需要分支**，但面试会问，要知道概念。

### 20.6 重要注意事项

**上传到 GitHub 之前必须确认**：
- ❌ 不包含公司名（Komatsu、小松）
- ❌ 不包含产品代号（K37TM）
- ❌ 不包含 Copyright 头
- ❌ 不包含公司编译器路径（build.bat 里的 GHS 路径）
- ✅ 你自己写的练习代码可以上传
- ✅ 仓库设为 Private（私有）

**检查命令**：
```bash
Select-String -Path "*.h","*.cc" -Pattern "Komatsu|Copyright|K37TM|SAIGYO"
# 如果有输出 → 有公司信息，删掉再上传
```

### 20.7 Git 面试用语

| 中文 | English |
|------|---------|
| 版本管理 | "I use Git for version control" |
| 分支开发 | "Feature branch workflow — create a branch, develop, then merge via pull request" |
| 代码审查 | "Pull request with code review before merging to main" |
| 提交规范 | "Descriptive commit messages explaining what and why" |
| 冲突解决 | "Manual conflict resolution when two branches modify the same code" |

**面试一句话**：
> "I use Git for version control — branching for feature development, committing with descriptive messages, and following a pull request workflow for code review before merging."

### 20.8 如果要创建新的仓库

和第一次完全一样的步骤：
1. 新建本地文件夹 → 放入代码
2. `git init` → `git config` → `.gitignore` → `git add .` → `git commit`
3. GitHub 网页创建新仓库
4. `git remote add origin` → `git branch -M main` → `git push -u origin main`

**一个本地文件夹 = 一个 .git = 一个 GitHub 仓库。各自独立。**

---

## 十九、北欧嵌入式工程师面试准备（拿着就能用版）

> 面向瑞典、芬兰、挪威、丹麦的嵌入式 C/C++ 岗位。
> 所有内容已按北欧面试风格组织，英文部分可直接在面试中使用。

### 21.1 北欧嵌入式市场：目标公司清单

| 国家 | 公司 | 领域 | 备注 |
|------|------|------|------|
| 🇸🇪 瑞典 | **Volvo Group** | 卡车/工程机械 | 你的经验最匹配！同行业 |
| 🇸🇪 瑞典 | **Scania** | 重型卡车 | 变速箱控制经验直接对口 |
| 🇸🇪 瑞典 | **Volvo Cars** | 乘用车 | AUTOSAR 重度用户 |
| 🇸🇪 瑞典 | **Einride** | 自动驾驶卡车 | 初创，技术氛围好 |
| 🇸🇪 瑞典 | **Husqvarna** | 园林机器人/电锯 | 嵌入式控制 |
| 🇸🇪 瑞典 | **Veoneer / Arriver** | ADAS 安全系统 | 功能安全 |
| 🇸🇪 瑞典 | **Aptiv** | 汽车电子 | AUTOSAR |
| 🇫🇮 芬兰 | **KONE** | 电梯控制系统 | 安全关键嵌入式 |
| 🇫🇮 芬兰 | **Wärtsilä** | 船用发动机控制 | 和你的项目极其相似 |
| 🇫🇮 芬兰 | **Nokia** | 通信设备 | 嵌入式 Linux |
| 🇩🇰 丹麦 | **Danfoss** | 液压/变频器控制 | 工程机械液压，对口！ |
| 🇩🇰 丹麦 | **Grundfos** | 水泵控制系统 | 嵌入式电机控制 |
| 🇳🇴 挪威 | **Kongsberg** | 国防/海事 | 安全关键系统 |

**最匹配的前3名**：Volvo Group、Scania、Danfoss（都做重型机械/变速箱/液压控制）

### 21.2 你的经验翻译表（中文→英文面试用语）

| 你学到的（中文） | 面试时怎么说（English） |
|----------------|----------------------|
| 大型建设机械制造商 变速箱控制系统 | "Safety-critical transmission control system for heavy construction equipment" |
| RH850 芯片 | "Renesas RH850 automotive-grade microcontroller" |
| Green Hills 编译器 | "Green Hills cross-compilation toolchain for RH850" |
| 四层架构 | "Layered software architecture with OS, middleware, application, and user layers" |
| Manager 树 + 32格限制 | "Composite pattern with fixed-size containers for deterministic memory usage" |
| INSTANTIATION 宏接线 | "Dependency injection via macro-based object wiring at compile time" |
| Config 分离机制 | "Separation of configuration data from business logic — similar to calibration parameters" |
| 信号对象（值+VALID/INVALID） | "Signal quality monitoring — each signal carries both value and validity status" |
| A/B 双传感器冗余 | "Redundant sensor architecture with diagnostic-based failover" |
| 跛行回家 | "Limp-home degradation mode for sensor failure scenarios" |
| 防抖动 debounce | "Debounce filtering to reject transient noise on sensor inputs" |
| 边沿检测 | "Edge detection for momentary switch processing" |
| 状态机 switch-case | "Finite state machine implementation for mode transitions" |
| 操作意图 vs 物理现实 | "Cross-validation between operator command and physical sensor feedback" |
| 单一职责流水线 | "Single-responsibility module pipeline with loose coupling" |
| Makefile + Shell 构建 | "Make-based build system with shell script automation for library compilation" |
| .cc → .o → .lib → .abs → .mot2 | "Compilation pipeline: source → object → static library → executable → flash image" |
| Git 版本管理 | "Git-based version control — feature branching, descriptive commits, pull request workflow for code review" |
| 防御性编程（溢出保护） | "Defensive programming — overflow-safe timer counters using uint32 max clamping to prevent wrap-around after extended runtime" |
| 电平检测 vs 边沿检测 | "Level detection (IsTimeUp) vs edge detection (IsTimeUpNow) — standard pattern for differentiating sustained state from state transitions" |
| 最短显示时间 timer | "Minimum display timer to prevent indicator lamp flickering during sensor value oscillation near threshold" |
| Config 外部注入 | "Separation of configuration from logic — similar to calibration parameters in automotive" |
| 10ms Update 循环轮询 | "Cooperative scheduling with 10ms cyclic task execution — polling-based, no blocking waits" |
| 星形拓扑状态机 | "Star topology state machine — all states route through a central idle state for safety and simplicity" |
| 三相状态机 transition/entry/do | "Three-phase state machine: transition (decide), entry (one-time init), do (continuous action)" |
| const & 只读引用输入 | "Const reference inputs for zero-copy read-only access — ownership stays with the producing module" |
| Manager 树形调用 | "Tree-structured task manager using Composite pattern — cascading Update() calls from OS through all modules" |

### 21.3 职业定位与岗位头衔 / Job Title & Professional Identity

#### 你到底算什么工程师？

| 头衔 | 符合？ | 理由 |
|------|-------|------|
| Embedded Software Engineer | ✅ 简历/面试用这个 | 行业标准头衔，你的技能在Junior级范围内 |
| Embedded Software Analyst | ✅ 最精准描述 | 2年+系统级代码分析、信号追踪、架构理解 |
| Embedded Systems Engineer (Junior) | ✅ 开始写代码后可用 | 已完成10个模块编写练习（含无参考三相状态机一次通过），正在过渡中 |
| HILS Test Engineer | ⚠️ 名义上是 | 派遣合同头衔，但实际HILS/台架工作较少 |
| Software Developer | ❌ 不要用 | 暗示你写量产代码，目前还不是 |
| Systems Architect | ❌ 不要用 | 太高级，你是在分析架构不是设计架构 |

#### 不同场景的自我介绍

**日常闲聊（朋友/家人问）：**
> "我做嵌入式软件的，在建设机械行业。简单说就是分析控制大型机械的芯片程序。"

**北欧嵌入式岗位面试（英文，最重要）：**
> "I'm an Embedded Software Engineer working on safety-critical transmission control systems for heavy construction equipment. My work involves analyzing a production codebase of 400+ modules on Renesas RH850, tracing signal flows across CAN/J1939 networks, and understanding the full system architecture including state machines, diagnostics, and FMEA."

**PhD申请/学术场合：**
> "I work in the embedded systems industry, analyzing real-time control software for construction machinery."

#### 面试时的诚实表述

**简历统一用 "Embedded Software Engineer"** — 这是北欧招聘市场标准头衔，HR搜索用这个关键词。Junior级别本来就不要求独立完成整个系统，分析+理解+能写模块 = 合格。

**但不要假装写量产代码，诚实说：**
> "My primary work has been analyzing and understanding the existing codebase rather than writing production code. However, I've been practicing writing embedded modules from scratch — 10 modules including three-phase state machines, multi-output controllers, hysteresis-based control, and timer logic. I can now write a medium-complexity three-phase state machine without reference code in a single pass. I use Git for version control and maintain my practice code on GitHub."

这在北欧文化里是加分的（诚实 + 展示学习能力 + 有代码可以展示）。

### 21.4 自我介绍模板（60秒版，直接背）

> "I'm an Embedded Software Engineer with hands-on experience in safety-critical systems. I've spent over two years working on a transmission control system for heavy construction equipment, running on a Renesas RH850 microcontroller with the Green Hills compiler.
>
> The system has a layered architecture with over 400 hand-written C++ modules. I've analyzed the full codebase — module architecture, signal flow from sensors to actuators, diagnostic and FMEA failure chains, the dependency injection wiring mechanism, and the Make-based build system.
>
> Beyond analysis, I've been writing embedded modules from scratch — 10 modules including three-phase state machines with hysteresis-based threshold detection, timer-based debounce logic, multi-output controllers, and diagnostic failover handling, all compiled for RH850. I can now write a three-phase state machine from scratch without reference code in a single compilation pass. I have experience with key embedded concepts like redundant sensor design, limp-home degradation, signal quality monitoring, defensive programming, and edge detection patterns. I use Git for version control.
>
> I'm looking for an embedded software development role in the Nordic industry."

### 21.5 STAR 故事（面试必备3个）

北欧面试喜欢 **competency-based questions**（能力导向提问）。用 STAR 格式回答：

#### 故事1：理解复杂系统（对应问题："Tell me about a complex system you worked with"）

**骨架（记住这4点就够）：**
- S: 400+模块，没人带，要自己看懂
- T: 理解整个系统的信号流
- A: 从简单→复杂逐层递进，追踪了一条4级信号链
- R: 几周内能独立读任何模块

**完整英文稿（约90秒）：**

> When I joined the project, I was handed a transmission control system for a large construction machine — over 400 hand-written C++ modules running on a Renesas RH850 microcontroller. There was no documentation walkthrough; I had to figure it out myself.
>
> My goal was to understand the full system — how sensor data flows from input to control output, and how all the modules connect together.
>
> So I took a systematic bottom-up approach. First, I started with the simplest modules — pure combinational logic, just if-else with no state. Once I was comfortable, I moved to modules with timers and hysteresis. Then state machines. Then multi-timer three-phase state machines. At each level, I traced signals through the central wiring file that connects all modules via dependency injection.
>
> One concrete example: I traced the complete vehicle speed signal — from dual redundant shaft sensors, through a diagnostic selector that picks the healthy one, through a failover module that switches to limp-home mode if both sensors fail, and finally to the stop-detection output that uses a 500-millisecond debounce timer. That's four processing stages for a single signal, each handled by a separate module following the single-responsibility principle.
>
> As a result, within a few weeks I could independently read and understand any module in the system. I mapped out the full signal flow architecture and understood the multi-layer safety design — redundant hardware, automatic failover, and graceful degradation to limp-home mode.

#### 故事2：安全设计意识（对应问题："How do you approach safety in embedded systems?"）

**骨架（记住这4点就够）：**
- S: 建设机械，软件出错=失控
- T: 理解系统怎么保证安全
- A: 分析了4层安全机制（VALID标志、冗余传感器、跛行模式、防抖定时器）
- R: 能解释多层安全设计的整体思路

**完整英文稿（约90秒）：**

> The system I worked on controls the transmission of a large construction machine — we're talking about a vehicle that weighs tens of tons. If the software makes a wrong decision, the machine could move unexpectedly. So safety is absolutely critical.
>
> I wanted to deeply understand how the system prevents unsafe states — not just "it's safe," but exactly how each layer works.
>
> What I found was a multi-layer safety architecture. First, every signal in the system carries a validity flag — VALID or INVALID — so if a sensor fails, downstream modules immediately know not to trust that data. Second, critical sensors like the output shaft speed sensor are physically duplicated — sensor A and sensor B on the same shaft — with a diagnostic module that automatically switches to the healthy one. Third, if both sensors fail, the system doesn't just stop — it enters a limp-home mode using estimated values, allowing the operator to safely move the machine to a service area. And fourth, time-critical decisions use debounce filtering — for example, the vehicle isn't considered "stopped" until speed stays below threshold for 500 milliseconds continuously, to reject transient noise.
>
> Now I can explain the complete safety philosophy: redundant hardware, signal quality monitoring, automatic diagnostic failover, graceful degradation, and temporal filtering — five layers working together. This understanding helps me write safer code myself, because I know what patterns to follow.

#### 故事3：构建系统（对应问题："Describe your experience with build systems"）

**骨架（记住这4点就够）：**
- S: 交叉编译，Windows→RH850芯片，多个库分开编译
- T: 理解从源码到可烧录文件的完整过程
- A: 追踪了整个构建流水线（Shell→C#→Make→4个库→链接→.mot2）
- R: 能解释每一步，知道改了模块后跑哪个脚本

**完整英文稿（约70秒）：**

> The project uses cross-compilation — we develop on a Windows PC but the target is a Renesas RH850 microcontroller. The build system is quite layered, and I wanted to understand the complete pipeline from source code to flashable image.
>
> So I traced through every step. It starts with Shell scripts that orchestrate the whole process. First, C# scripts scan the source directories and generate file lists. Then Make is invoked with parallel jobs to compile the sources. There are four separate static libraries built independently — hand-written application modules, auto-generated modules, CAN communication code, and memory data. Each library is compiled from its own set of source files using the Green Hills cxrh850 cross-compiler.
>
> After compilation, all four libraries are linked together with the middleware layer and the real-time OS to produce the final executable, which is then converted to a .mot2 flash image.
>
> As a result, I understand every step from .cc source file to .mot2 flash image. When I modify a module, I know exactly which script to run and which library gets rebuilt. This also helped me set up my own practice build environment for writing modules from scratch.

#### 故事4：深入理解复杂模块（对应问题："Tell me about a time you had to understand something deeply"）

**骨架（记住这4点就够）：**
- S: 雨刮控制模块——系统最复杂的模块（三相状态机+4个定时器+安全覆写）
- T: 不只看懂"做了什么"，还要理解"为什么这么设计"
- A: 逐行阅读+画状态图+追踪3种场景+发现星形拓扑设计原则
- R: 提炼出安全系统通用设计哲学："不需要+不安全+更复杂=不要做"

**完整英文稿（约90秒）：**

> The wiper and washer control module was the most complex module in the entire system — a three-phase state machine with four timers, three states, safety override logic, and hardware-specific relay control. I decided to understand it deeply, not just what it does, but why every design decision was made.
>
> I went through the code line by line. I discovered that the state machine uses what I call a "star topology" — all states route through a central idle state instead of allowing direct transitions between active states. At first this seemed unnecessary, but I figured out three reasons why: first, direct transitions aren't needed because there's no use case for them. Second, they're not safe — if you jump directly from one active state to another, timers from the previous state might carry stale values. Third, more direct transitions mean more paths to test and more potential bugs.
>
> I also manually traced three complete scenarios step by step — a quick button press, a long press, and a safety override — verifying the timing behavior across each 10-millisecond cycle. I drew the state transition diagram myself and validated it against the actual code.
>
> The key takeaway was a universal design principle for safety-critical systems: "not needed, not safe, more complex — don't do it." I now apply this thinking when I write my own modules — I've successfully written three-phase state machines from scratch using these same patterns.

### 21.5 常见技术面试题 + 准备答案

#### C++ 基础（必考）

**Q: What is the difference between a pointer and a reference?**
> A pointer can be null, can be reassigned, and uses `*` to dereference. A reference is an alias that must be initialized, cannot be null, and cannot be rebound. In our project, module inputs use `const &` (constant references) for zero-copy read-only access to other modules' outputs.

**Q: What is the difference between `const &` members and regular members?**
> `const &` members are references to external data — they don't own the data and can only read it. Regular (non-const, non-reference) members are objects owned by the class instance — they can be modified freely. In our project, input signals are `const &` (read-only view of another module's output), while timers and output signals are owned objects that the module updates each cycle.

**Q: What are the different ways to initialize members in a constructor initializer list?**
> Three patterns: (1) Reference binding — `config_(config)` saves a reference to external data. (2) Object construction — `timer_(config.timer_config)` creates a new object using parameters from the config. (3) Value initialization — `flag_(false)` sets a primitive to its initial value. The choice depends on ownership: references for shared/external data, construction for owned tools, direct values for simple state.

**Q: What is a virtual function? Why use it?**
> A virtual function enables runtime polymorphism. In our project, `ModuleInterface` declares `virtual void Update()`, and every module overrides it. The Manager calls `Update()` on each registered module without knowing the concrete type — this is the key to the composite pattern that manages 400+ modules.

**Q: What is a `protected virtual` method used for?**
> It's an extension point for subclasses. In our wiper control module, `IsWiperSwAvailable()` is `protected virtual` — the base class returns `true`, but subclasses (like engine-specific variants) can override it to add extra conditions. This follows the Open-Closed Principle: the base class is open for extension but closed for modification.

**Q: What is RAII?**
> Resource Acquisition Is Initialization — acquiring resources in constructors and releasing in destructors. This prevents resource leaks. In embedded systems like ours, we don't use dynamic memory (no `new`/`delete`), so RAII is mainly about ensuring proper initialization in constructor initializer lists.
>
> ⚠️ 你需要额外学习：智能指针（`unique_ptr`, `shared_ptr`），move 语义，这些在北欧面试经常考。

#### 嵌入式专题（必考）

**Q: What is cooperative scheduling? How does it differ from preemptive scheduling?**
> In cooperative scheduling, each task voluntarily yields control after completing its work — it runs its `Update()` method and returns. The scheduler calls tasks in sequence. In preemptive scheduling, the OS can interrupt a running task at any time to run a higher-priority task.
>
> Our project uses cooperative scheduling: the OS calls `user_main()` every 10ms, which cascades through the Manager tree calling each module's `Update()`. Every module must complete quickly and return — if one module blocks, the entire system freezes. This is deterministic and simple, but requires discipline: no infinite loops, no blocking waits.

**Q: What is polling? When would you use it vs interrupts?**
> Polling means periodically checking a condition each cycle rather than waiting for it. In our system, modules don't "wait" for events — they check conditions every 10ms and skip if not ready. A timer "waiting 500ms" actually means 50 cycles of checking `IsTimeUp()` and returning false.
>
> Polling is simpler and deterministic but wastes CPU cycles. Interrupts are more efficient for rare events but add complexity (ISR, context switching, race conditions). Our architecture uses polling at the application level, while the OS layer handles hardware interrupts.

**Q: Explain your experience with state machines in embedded systems.**
> I've worked with two levels of state machines. Simple state machines use switch-case with enum states — for example, our edge detection module converts ON/OFF switch input to OFF_ON/ON_OFF momentary signals using a 4-state machine.
>
> More complex modules use a three-phase state machine pattern: transition (decide if state should change), entry (one-time initialization when entering a new state), and do (continuous action while in the state). We detect state changes using a snapshot comparison: `if (current_state != previous_state)` — this is essentially edge detection at the state level.
>
> We also use a star topology where all states route through a central idle state, rather than allowing direct transitions between active states. This reduces complexity and improves safety in industrial control systems.

**Q: How do you handle sensor noise in embedded systems?**
> Debounce filtering. For example, in our speed sensor, we use an increment timer: the timer counts up each cycle when the condition is met, but resets to zero if the condition breaks. Only when the timer reaches the threshold (e.g., 500ms) do we consider the signal stable. This prevents transient noise from triggering false state changes.

**Q: What is a watchdog timer?**
> A hardware timer that resets the system if software stops responding. The software must periodically "kick" the watchdog to prove it's alive. If a module hangs or enters an infinite loop, the watchdog expires and forces a system reset.
>
> ⚠️ 你的笔记中没有涉及 watchdog，但这是嵌入式面试必考题，需要额外学习。

**Q: Explain interrupt handling.**
> An interrupt is a hardware signal that preempts the current execution to handle a time-critical event. When an interrupt fires, the CPU saves context, runs the ISR (Interrupt Service Routine), then restores context. ISRs should be short — handle the event and set a flag, then let the main loop process it.
>
> ⚠️ 你的项目是 Update 循环模式（cooperative scheduling），不直接写 ISR，但要理解概念。

**Q: What is the difference between big-endian and little-endian?**
> Byte order in multi-byte values. Big-endian stores MSB first (like reading left to right); little-endian stores LSB first. This matters in CAN bus communication — when two devices with different endianness exchange data, bytes must be swapped.

#### 安全相关（北欧特别重视）

**Q: What is functional safety? Do you know ISO 26262?**
> Functional safety ensures that safety-related systems operate correctly even in the presence of hardware or software faults. ISO 26262 is the automotive functional safety standard, defining ASIL levels (A to D, D being most critical).
>
> In our project, we implement several functional safety concepts: redundant sensors, signal validity monitoring, diagnostic failover, limp-home degradation, and debounce filtering. While I haven't formally worked with ISO 26262 processes, I understand the engineering principles behind them.
>
> ⚠️ 建议额外学习：ASIL 等级、V-Model 开发流程、FMEA 分析（你的 tool/ 目录下就有 FMEA 定义表）

**Q: How do you handle a sensor failure at runtime?**
> Our system uses a multi-level approach: (1) Dual redundant sensors measuring the same shaft. (2) Diagnostic modules continuously check sensor health. (3) If sensor A fails, automatic switchover to sensor B. (4) If both fail, enter limp-home mode using estimated values. (5) Each signal carries a VALID/INVALID flag so downstream modules can react to degraded data quality.

### 21.6 你需要补的知识清单（优先级排序）

| 优先级 | 主题 | 来源 | 预计时间 |
|-------|------|------|---------|
| 🔴 P0 | C++ 深度：智能指针、RAII、move 语义、虚函数表 | 《Effective Modern C++》 前10章 | 2周 |
| 🔴 P0 | 数据结构与算法：LeetCode Easy 30题 + Medium 20题 | LeetCode | 3周 |
| 🔴 P0 | 英文 STAR 故事练到流利 | 对镜子/录音练习 | 持续 |
| 🟡 P1 | 设计模式英文名记熟（你已经懂原理！） | 见 21.13 小节 | 2天 |
| 🟡 P1 | MCU 知识整理成可讲的故事 | 回忆整理 | 2天 |
| 🟡 P1 | RTOS 基础：任务调度、mutex、semaphore、优先级反转 | 《Real-Time Systems》或在线课程 | 1周 |
| 🟡 P1 | CAN 协议：帧结构、仲裁、错误处理 | CAN 规范 + 项目的 network/ 目录 | 1周 |
| 🟡 P1 | ISO 26262 概要：ASIL、V-Model、FMEA | 网上教程 + 项目 tool/ 里的 FMEA 表 | 3天 |
| 🟢 P2 | AUTOSAR 架构概念 | AUTOSAR 官网入门文档 | 3天 |
| 🟢 ✅ | Git 工作流（feature branch、PR、code review） | ✅ 5/12 完成基础操作 + GitHub上传 | ✅ 已完成 |
| 🟢 P2 | 单元测试（Google Test / CppUTest） | 教程 + 写几个测试 | 3天 |
| ❌ 不要碰 | MATLAB/Simulink | — | — |
| ❌ 不要碰 | AUTOSAR 深度细节 | — | — |
| ❌ 不要碰 | Linux kernel / device driver | — | — |

**为什么不学 MATLAB/Simulink？**
- 嵌入式软件工程师岗位（北欧招聘的 70%+）不要求 MATLAB
- MATLAB/Simulink 是另一个职业方向（控制算法工程师），需要 PID/传递函数/状态空间等控制理论基础
- 你的时间有限，C++ 深度 + LeetCode 的回报率远高于 MATLAB

### 21.7 北欧面试文化提醒

| 方面 | 北欧特点 | 你要注意的 |
|------|---------|-----------|
| **层级** | 极度扁平，叫名字不叫头衔 | 不要说 "Yes, sir" 或表现太谦卑 |
| **沟通风格** | 直接但友善，没有美式夸张 | 不需要过度自信，实事求是最好 |
| **"Lagom"** | 瑞典概念 = "刚刚好" | 不要过度吹嘘，也不要过度谦虚 |
| **团队合作** | 非常重视 "fika"（咖啡社交） | 面试常问 teamwork 问题，准备一个故事 |
| **Work-life balance** | 不加班是正常的 | 不要说 "I work 996"，说 "I'm efficient and deliver on time" |
| **英语** | 技术工作通常用英语 | 北欧人英语极好，你的口音不重要，清晰表达最重要 |
| **诚实** | 不知道就说不知道 | "I'm not familiar with that yet, but I understand the underlying concept of..." |
| **多轮面试** | 通常2-3轮：HR → 技术 → 团队/文化 | 技术面可能包括 live coding 和系统设计 |

### 21.8 薪资参考（2025-2026）

| 国家 | 初级嵌入式工程师 | 中级（3-5年） | 备注 |
|------|----------------|-------------|------|
| 🇸🇪 瑞典 | 35,000-42,000 SEK/月 | 42,000-55,000 SEK/月 | 税前，实际到手约70% |
| 🇫🇮 芬兰 | €3,200-3,800/月 | €3,800-5,000/月 | 税前 |
| 🇩🇰 丹麦 | 35,000-42,000 DKK/月 | 42,000-55,000 DKK/月 | 高税但高福利 |
| 🇳🇴 挪威 | 45,000-55,000 NOK/月 | 55,000-70,000 NOK/月 | 最高薪但生活成本也最高 |

### 21.9 简历要点（英文关键词 + 技能分级）

#### 技能分级的定义（面试时心里有数）

| 级别 | 含义 | 类比 | 面试被问时 |
|------|------|------|-----------|
| **Strong** | 天天用，闭着眼都能做 | 天天做饭 | 直接答，给例子 |
| **Working** | 看着参考能做出来 | 看菜谱做菜 | "I've used it in practice — for example..." |
| **Familiar** | 知道是什么、什么时候用，但没独立做过项目 | 知道菜名和食材，没自己做过 | "I haven't worked with it directly, but I understand the concept — ..." |

**Familiar不丢人**：Junior岗位JD写的"experience with X"很多时候Familiar就够了。面试官在意的是你知不知道这个东西存在、能不能学。

#### 技能分级写法（2026-05-13 更新，诚实原则！）

```
Title: Embedded Software Engineer
       ← 北欧招聘市场标准头衔，HR搜索用这个关键词

Technical Skills:
  Strong:      Embedded C++ (module development), State Machines, Signal Processing Logic,
               Code Analysis (400+ module codebase)
  Working:     Cross-compilation (GHS cxrh850/RH850), Build Systems (Make/Shell), Git,
               Hysteresis/Timer/Debounce patterns
  Familiar:    CAN/J1939, Functional Safety concepts, RTOS concepts, AUTOSAR concepts
```

#### 关键技能的诚实表述

| 技能 | 你的真实水平 | 简历写法 | 面试被追问时 |
|------|------------|---------|------------|
| RH850 | 应用层开发+交叉编译，没写过寄存器/BSP | `Renesas RH850 (application-level)` | "I worked at the application layer — writing and analyzing C++ modules compiled with the Green Hills toolchain. I haven't done bare-metal register programming." |
| CAN/J1939 | 知道系统里有CAN模块，理解帧仲裁和大小端概念，没写过CAN驱动 | 放Familiar级别 | "I understand that CAN uses message-based arbitration and our system transmits signals between ECUs via J1939. I've seen the CAN modules in the codebase, but I haven't written CAN communication code myself." |
| RTOS | 理解任务调度/mutex/优先级反转概念，项目用的是cooperative scheduling | 放Familiar级别 | "Our system uses cooperative scheduling rather than a preemptive RTOS, so I haven't used RTOS APIs directly, but I understand the concepts." |
| C++ | 能写模块（10个，含一次通过），能读懂量产代码，但没用过C++11+特性 | 放Strong级别 | "I write embedded C++ daily — state machines, timers, multi-output controllers. Our compiler is C++03, so I haven't used modern features like smart pointers in production." |

**核心原则**：
- ❌ 不撒谎 — 北欧文化最重技术诚实，夸大被发现=直接淘汰
- ❌ 不缩小 — "我用RH850编译器编译过模块"就是RH850经验
- ✅ 分级标注 — Strong / Working / Familiar 让面试官知道深度

#### 项目经验措辞（精确用词，不夸大）

| ❌ 不要写 | ✅ 要写 | 为什么 |
|----------|--------|--------|
| "Embedded Software Engineer" | "Embedded Systems Engineer (Analysis & Verification)" | 准确描述你的实际角色 |
| "Proficient in RH850" | "Worked on RH850-based system" | 你没写过寄存器代码 |
| "Programmed RH850" | "Target platform: Renesas RH850" | 客观描述 |
| "Developed transmission control system" | "Analyzed safety-critical transmission control codebase" | 诚实 |
| "HILS Test Engineer" | "Embedded Systems Analysis & Verification" | HILS限制定位，analysis更准确 |

#### ATS 关键词（确保简历里有）

```
- Languages: C++ (embedded), C, C#, Shell scripting
- Platforms: Renesas RH850 (application-level development, cross-compilation)
- Toolchain: Green Hills (cxrh850), Make, cross-compilation
- Version Control: Git, GitHub
- Protocols: CAN bus, J1939 (familiar)
- Design Patterns: Composite, Strategy, Dependency Injection, Observer, State Machine
- Concepts: Functional safety, redundant sensor design, signal quality monitoring,
  finite state machines, hysteresis-based threshold detection, debounce filtering,
  defensive programming, edge detection, cooperative scheduling
- Domain: Transmission control, construction equipment, safety-critical systems
- Practice: 10 embedded modules written from scratch (state machines, multi-output,
  timer logic), compiled for RH850 target
```
- Build: Makefile, shell scripts, static library linking
- OS: RTOS concepts, cooperative scheduling
```

> ⚠️ 简历中**不要写**客户公司名。写 "a major construction equipment OEM" 即可。

**Design Patterns 那一行会让北欧面试官眼前一亮** — 很多嵌入式工程师不会讲设计模式。

#### 简历格式规范（北欧版）

```
❌ 不要写：年龄、出生日期、照片、婚姻状况、性别
✅ 要写的：名字、联系方式、LinkedIn、技能、项目经验、教育
长度：1-2 页（不要超过2页）
语言：英文
```

### 21.10 面试前一天的检查清单

- [ ] 能用英语流利说出60秒自我介绍
- [ ] 能用英语讲4个 STAR 故事（原3个 + 深入理解复杂模块）
- [ ] 能画出四层架构图并解释
- [ ] 能画出 revolution 信号的全链路（传感器→is_stop）
- [ ] 能画出 wiper_control 的星形拓扑状态转移图
- [ ] 能解释指针 vs 引用、虚函数、RAII
- [ ] 能解释 const & vs 非const非& 成员的区别（ownership）
- [ ] 能解释 debounce、state machine、edge detection
- [ ] 能解释三相状态机（transition/entry/do）
- [ ] 能解释冗余传感器 + 跛行回家
- [ ] 能解释 cooperative scheduling 和 polling
- [ ] 能回答 "What don't you know yet?" → "I want to deepen my knowledge of AUTOSAR and ISO 26262 formal processes"
- [ ] 准备好问面试官的问题（至少2个）：
  - "What does the tech stack look like for this team?"
  - "How does your team handle code reviews and knowledge sharing?"

### 21.11 年龄问题：30+ 完全不是问题

| 方面 | 北欧现实 |
|------|---------|
| **法律** | 瑞典/芬兰/丹麦/挪威法律**严禁年龄歧视**，简历不写出生日期 |
| **行业平均** | 北欧嵌入式工程师转行/入行的平均年龄在 28-35 岁 |
| **教育节奏** | 北欧人 25-27 岁硕士毕业是常态，30 岁工作 3-5 年很正常 |
| **文化** | 北欧社会非常接受职业转换（career change），政府甚至提供培训补贴 |
| **简历规范** | **不写年龄、不贴照片、不写婚姻状况** — 这就是北欧简历的标准 |

### 21.12 技能不精通怎么办？（诚实表达策略）

#### 面试官对 Junior 的真实期望

| 他们期望的 | 你的现状 | 匹配？ |
|-----------|---------|--------|
| 能读 C/C++ 代码 | ✅ 你能读 400+ 个模块 | ✅ |
| 理解嵌入式基本概念 | ✅ 中断、定时器、GPIO、ADC | ✅ |
| 能解释一个项目的架构 | ✅ 四层架构、信号流、构建系统 | ✅ |
| 知道设计模式 | ✅ Composite、DI、Strategy、State Machine | ✅ |
| 能写复杂算法 | ❌ 需要练 LeetCode | → 补 |
| 精通 C++ 高级特性 | ❌ 需要学 | → 补 |
| 独立开发功能模块 | ❌ 还没做过 | → 入职后学，正常 |

**北欧 Junior 岗位不期望什么都精通。** 他们期望：有基础 + 能学 + 诚实 + 沟通清晰。

#### "你精通吗？"的标准回答模板

> ❌ "Yes, I'm an expert" → 被追问就穿帮
>
> ❌ "No, I'm not good at anything" → 太自贬
>
> ✅ 正确说法：
>
> "I have working knowledge of C++ and embedded systems. I've spent time analyzing a real safety-critical codebase — reading modules, tracing signal flow, and understanding the build system. I'm not at a senior level yet, but I learn quickly and I'm motivated to grow. For example, I went from zero knowledge of this transmission control system to understanding its full architecture in a few weeks."

**这句话同时表达了：诚实 + 能力 + 学习速度 + 具体证据 = 北欧面试官想听的。**

#### 被追问 RH850 经验时怎么回答

> "I worked on the application layer of an RH850-based transmission controller for a major construction equipment manufacturer. I didn't write low-level register code directly — the hardware abstraction was handled by the middleware layer — but I understand the full build pipeline from C++ source to flash image, and I understand the layered architecture that separates hardware access from application logic."

#### 不知道答案时的万能句式

> "I'm not familiar with that specific topic yet, but I understand the underlying concept of [相关概念]. It's something I'd be keen to learn."

这在北欧不是减分 — 诚实 + 学习意愿 = 加分。

#### 最实际的建议

```
不要想着"等我精通了再去面试"
→ 你永远不会觉得自己准备好了
→ 正确做法：边面边学
→ 面试本身是最好的学习 — 发现不足，回来补
→ 北欧公司对 Junior 有培养计划（onboarding 通常 3-6 个月）
```

### 21.13 你的项目中的设计模式（面试加分项）

北欧面试**特别爱问设计模式**。你的项目里已经有这些，能说出英文名 = 加分：

| 设计模式（English） | 在项目中的体现 | 面试怎么说 |
|-------------------|---------------|-----------|
| **Composite Pattern** | Manager 树（Manager 本身也是 ModuleInterface） | "The Manager class implements the Composite pattern — it's both a container and a module itself, enabling a tree structure where `Update()` propagates recursively." |
| **Dependency Injection** | INSTANTIATION 宏把依赖从外部传入 | "All dependencies are injected via constructor parameters. Modules don't create or locate their inputs — the wiring file passes them in during instantiation." |
| **Strategy Pattern** | A/B 传感器切换（根据诊断状态选择不同策略） | "The sensor selection module implements a Strategy pattern — switching between sensor A, sensor B, or estimated values based on diagnostic state." |
| **State Machine Pattern** | switch_momentry_convert 的 switch-case | "We use finite state machines with enum-based states and switch-case transitions for mode management." |
| **Observer Pattern**（广义） | 模块通过 const & 观察其他模块的输出 | "Modules observe other modules' outputs through const references — a lightweight observer mechanism without callback overhead." |
| **Template Method**（广义） | ModuleInterface 定义 Update() 接口 | "The base class defines the lifecycle methods (Init, Update), and each module overrides them — similar to the Template Method pattern." |
| **Star Topology FSM** | wiper_control 所有状态经过 NOT_ACTIVE | "We use star topology state machines where all states route through a central idle state. This reduces transition paths and prevents unsafe state combinations — the design principle is: if it's not needed, not safe, and more complex, don't do it." |

#### 你的额外技能怎么在面试中提及

| 技能 | 怎么说 |
|------|--------|
| **C#** | "I also have C# experience. In this project, the code generation tooling was written in C#. I'm comfortable reading and writing C# for tooling and test automation." |
| **MCU 编程** | "I have hands-on experience with MCU programming — GPIO, timers, ADC, interrupts, register-level operations. In this project, the hardware abstraction layer handles that, but I understand what's happening underneath." |
| **设计模式** | "I've studied design patterns and I can identify them in real codebases. For example, in this project I found Composite, Dependency Injection, Strategy, and State Machine patterns." |

---

## 二十、8周面试冲刺计划

> 项目内（工作时间）+ 项目外（每天1-2小时）双线并行。第8周开始投简历。

### 22.1 总体时间线

```
           项目内学习（工作时间）              项目外学习（每天1-2小时）
           ─────────────────              ────────────────────────
第1-2周     读复杂模块 + CAN 链路追踪        C++ 深度（智能指针、RAII、move语义）
第3-4周     读诊断模块 + FMEA 表             LeetCode Easy 30题
第5-6周     读网络层代码 + 整理设计模式       LeetCode Medium 20题 + RTOS 概念
第7-8周     总结、整理面试故事               英文 STAR 练习 + 模拟面试
            ↓                              ↓
           持续更新笔记                     第8周开始投简历
```

### 22.2 项目内学习计划

#### 第1-2周：读更复杂的模块

| 目标 | 具体做什么 | 面试价值 |
|------|-----------|---------|
| 读1个多定时器模块 | 找有2-3个 IncrementTimer 的模块 | 展示能处理复杂逻辑 |
| 读1个多状态机模块 | 和 switch_momentry_convert 对比 | 深化 state machine |
| 追踪 CAN 信号链路 | 从 `network::` 对象追踪到业务模块 | 新 STAR 故事素材 |

#### 第3-4周：读诊断和故障模块

| 目标 | 具体做什么 | 面试价值 |
|------|-----------|---------|
| 读1个 `diag_` 开头的模块 | 理解故障检测、DTC 码 | 功能安全必答题 |
| 打开 tool/ 下的 FMEA 表 | 看故障模式分析文档 | "I've reviewed FMEA documentation" |

#### 第5-6周：读网络层 + 整理

| 目标 | 具体做什么 | 面试价值 |
|------|-----------|---------|
| 看 `system/network/` 代码 | CAN 消息收发的代码结构 | "I have CAN bus experience" |
| 看 KDOG2 目录 | CAN 配置工具 | 工具链知识 |
| 整理项目中所有设计模式 | 复习 21.13，找更多例子 | 设计模式面试素材 |

#### 第7-8周：总结冲刺

| 目标 | 具体做什么 |
|------|-----------|
| 更新笔记到最终版 | 所有新模块加入第十五章 |
| 整理5个 STAR 故事 | 已有4个 + CAN链路追踪或诊断模块1个 |
| 画3张架构图（纸上能画） | 四层架构图 + revolution 全链路图 + wiper星形状态机图 |

### 22.3 项目外学习计划

#### 第1-2周：C++ 深度

```
资源：《Effective Modern C++》或 https://www.learncpp.com（第17-22章）

每天1-1.5小时：
├── 周一：智能指针 unique_ptr / shared_ptr
├── 周二：move 语义和右值引用
├── 周三：模板基础
├── 周四：练习 — 用 unique_ptr 写一个小程序
├── 周五：RAII 和资源管理
├── 周六：复习 + 用英文写出5个概念的面试回答
├── 周日：休息
└── 第2周：虚函数表、const正确性、override/final
```

**每个概念学完后，写一句英文面试回答。**

#### 第3-6周：LeetCode

```
第3-4周：Easy 30题（每天工作日2题 + 周末3题）
├── 重点：Array、String、HashMap、LinkedList、Stack
├── 用 C++ 写（练 STL：vector、map、unordered_map）
└── 每题不超过30分钟，看不出来就看答案理解思路

第5-6周：Medium 20题（每天1-2题）
├── 重点：Binary Tree、BFS/DFS、Two Pointers、Sliding Window
├── 北欧面试不出 Hard，Medium 足够
└── 每题用英文写一句思路总结
```

推荐题目：
```
Easy: Two Sum, Valid Parentheses, Merge Two Sorted Lists,
      Best Time to Buy and Sell Stock, Valid Palindrome,
      Linked List Cycle, Reverse Linked List, Maximum Depth of Binary Tree,
      Contains Duplicate, Invert Binary Tree, Binary Search...

Medium: Add Two Numbers, Longest Substring Without Repeating,
        3Sum, Container With Most Water, Group Anagrams,
        Binary Tree Level Order Traversal, Number of Islands,
        Course Schedule, Top K Frequent Elements, Validate BST...
```

#### 第5-6周穿插：RTOS 概念（每周2-3小时）

```
只需理解概念，不需要写代码：
├── Task/Thread 区别
├── Mutex vs Semaphore
├── Priority Inversion（优先级反转）
├── Watchdog Timer
├── Interrupt vs Polling
├── Cooperative vs Preemptive scheduling
└── 推荐：YouTube 搜 "RTOS basics embedded"（20分钟视频很多）
```

#### 第7-8周：英文面试练习

```
每天30分钟：
├── 周一：对镜子说60秒自我介绍（计时）
├── 周二：讲 STAR 故事1（录音→自己听→改进）
├── 周三：讲 STAR 故事2
├── 周四：讲 STAR 故事3
├── 周五：从 21.5 抽技术题，英文回答
├── 周六：找朋友或 AI 做模拟面试
├── 周日：修改简历最终版
└── 第8周同节奏 + 开始投简历
```

### 22.4 每周自检表（打印出来贴墙上）

```
第__周（日期：____）

项目内：
□ 读了哪个模块？ _______________
□ 笔记更新了吗？
□ 发现了新设计模式吗？ _______________

项目外：
□ C++/LeetCode 完成了什么？ _______________
□ 英文练习了几次？ ___次
□ 新写了哪个英文面试回答？ _______________

自我评估：
□ 这周最大的收获？
□ 下周最该补什么？
```

### 22.5 投简历时间线

```
第6周：简历初稿完成
第7周：找人 review（LinkedIn 上找北欧华人工程师）
第8周：开始投简历
       ├── LinkedIn: "Open to Work"，location 选北欧四国
       ├── 目标公司官网直投（Volvo, Scania, Danfoss 等 — 见 21.1）
       ├── 每周投 5-10 个岗位
       └── 搜索关键词："Embedded Software Engineer" + "C++" + 国家名
第9-12周：边面边学
       ├── 每次面试后记录问到的题目
       ├── 补短板 → 下次面试更好
       └── 面试本身是最好的学习
```

### 22.6 心态管理：关于"觉得自己做不好"

**这种感觉叫 Imposter Syndrome（冒充者综合征），90% 的工程师都有。**

#### 客观事实 vs 你的感觉

| 你觉得 | 客观事实 |
|--------|---------|
| "我什么都不会" | 3天前你不知道 INSTANTIATION 宏。现在你能解释四层架构、信号全链路、构建系统、6种设计模式 |
| "别人比我强" | 真实的竞争对手也在搜"什么是虚函数"。完美候选人不存在 |
| "我不够格" | 北欧 Junior 岗位不要求精通。他们要：能学 + 诚实 + 能沟通 |
| "面试会失败" | 第1次面试很可能表现不好，但第3次就会明显进步。这是正常的 |

#### "做不好"vs "不去做"

```
做了但做得不完美 → 拿到反馈 → 改进 → 下次更好 → 最终拿到 offer
没做            → 什么都不会发生 → 一年后还在原地
```

#### 北欧文化本身就接受"不完美"

> 瑞典有个词叫 **"Lagom"** = 不多不少，刚刚好。
> 他们不期望你完美，他们期望你**真实**。
> "I don't know that yet, but I'm willing to learn" 在北欧是加分的。

#### 给自己的提醒（面试前读一遍）

> 我不需要完美。我需要的是：
> 1. 能用英语清楚地讲出我理解的系统
> 2. 遇到不会的诚实说 "I'm not familiar with that yet"
> 3. 展示我的学习速度（从零到理解一个400+模块的系统）
> 4. 记住：面试官也是普通人，他们也经历过不自信的时候

### 22.7 不需要在日本"积累够了"再走

#### "等准备好再走"是一个陷阱

```
"再积累一年" → 一年后 → "再积累一年" → 又一年 → ...
              永远觉得不够              永远走不出去
```

#### 当前项目经验在北欧的价值

**大型建设机械制造商的品牌在北欧也有知名度。** 在北欧的矿山、森林、建筑工地看到该厂商的机械并不罕见。

| 你的日本经验 | 在北欧的价值 |
|-------------|-------------|
| 建设机械制造商的变速箱控制项目 | ⭐⭐⭐⭐⭐ 和 Volvo/Scania/Danfoss 同行业，直接对口 |
| 安全关键系统 | ⭐⭐⭐⭐⭐ 功能安全经验在哪都值钱 |
| 日本品质文化 | ⭐⭐⭐⭐ 北欧很欣赏日本的质量意识 |
| 日语能力 | ⭐⭐ 北欧不需要，但证明你能学外语 → 间接证明学习能力 |

#### 你已经超过很多北欧应届硕士

北欧本地应届硕士毕业生只有学校项目，没碰过真实工业代码。你有：
- 真实嵌入式项目经验（安全关键系统）
- C/C++/C# 基础
- 设计模式知识
- MCU 编程经历
- 理解过一个 400+ 模块的真实工业系统

#### 正确的做法

```
在日本的每一天：
├── 继续在当前项目上学习（免费的学习机会）
├── 执行8周冲刺计划（22.1-22.5）
└── 同时开始投简历

不需要"积累完了再走"
"边工作边准备，同时投简历"才是正确节奏
```


---

## 二十一、人生规划：瑞典PhD路线 / Life Planning: Sweden PhD Route

> 2026年4月14日制定。基于个人背景的完整分析和两条路线的对比评估。

### 23.1 个人背景客观评估 / Honest Self-Assessment

#### 教育背景 / Education

| 项目 | 详情 | 客观评价 |
|------|------|---------|
| 本科 | 自动化专业，国内非985/211 | 专业方向好（信号处理、控制理论基础），学校层次一般 |
| 硕士 | 日本国立电气通信大学（UEC），机械智能系统学专业 | ✅ 日本国立大学 = 国际认可的正规硕士学位 |
| 研究方向 | 非侵入型FES电极材料对比研究与电极开发 | ✅ FES是神经工程的核心技术之一（见23.3详细分析） |
| 研究成果 | ARSO 2020 论文投稿被拒，无正式发表 | ⚠️ 有劣势但不致命（有完整research experience） |
| 绩点 | 本科和硕士均一般 | ⚠️ 需要用动机信和研究匹配度弥补 |

#### 工作经历 / Work Experience

| 阶段 | 时长 | 内容 | 备注 |
|------|------|------|------|
| 工业炉公司 | 1年10个月 | 第一份工作 | 恶劣工作环境，主动辞职 |
| 在日华人派遣公司 | 6个月 | 第二份工作 | 被评能力不足，主动辞职 |
| 在日派遣公司→建设机械制造商 | 2年+（至今） | HILS测试工程师（名义）→ 实际是嵌入式系统分析 | 代码库分析、信号追踪、架构理解为主要业务 |

#### 技能盘点 / Skills Inventory

```
硬技能：
├── C/C++/C# 阅读/分析能力（能读懂400+模块的量产嵌入式代码库）
├── C++ 模块编写练习中（10个模块完成，含6次无参考挑战。无参考从13轮→1轮（一次编译通过！），错误数从21→0。三相状态机+多输出全部掌握。概念型错误全部消灭，仅剩极偶发打字精度问题）
├── 嵌入式系统架构理解（RH850、Manager架构、状态机、CAN/J1939/HTTP、信号流）
├── 底层工具库源码阅读（IncrementTimer源码：边沿检测、溢出保护、Config注入）
├── Git/GitHub 版本管理（init、add、commit、push、.gitignore、feature branch概念）
├── 信号处理基础（自动化本科、FES硕士）
├── FES系统研究经验（电极设计、阻抗测量、电刺激实验）
├── 日语（工作语言水平）
├── 英语（需要IELTS/TOEFL成绩）
└── MCU概念理解（实际的裸机开发经验还比较少）

软技能：
├── 自学能力极强（从零理解400+模块的工业代码库 = 客观证据，但属于分析而非开发）
├── 能在海外独立生活和工作（日本5年+）
└── 有完整的研究流程经验（硕士论文 + ARSO投稿经验）
```

#### 核心矛盾 / Core Tension

```
不适应日本社会和职场文化 → 确定想离开日本
有FES学术背景 → 在工作市场无用，但在学术申请中是核心资产
33岁 → 在日本/中国觉得"晚了"，在瑞典完全正常
想去北欧 → 两条路：①直接找工作（嵌入式） ②读PhD（神经科学/神经工程）
```

### 23.2 两条路线对比 / Route A vs Route B

#### 路线A：北欧嵌入式工程师 / Embedded Engineer Route

| 优势 | 劣势 |
|------|------|
| 已有扎实的代码阅读/架构分析能力（本笔记21-22章） | 实际的开发经验还比较少（以分析为中心→开发练习开始阶段） |
| 建设机械制造商经验在北欧有认知度 | 3份短期工作在简历上需要解释 |
| 嵌入式C++岗位在北欧持续缺人 | 竞争对手：北欧本地硕士 + 有实际项目经验的人 |
| 可以现在就开始投简历 | Junior岗位薪资一般，签证需雇主sponsor |

**评价**：2026-05-12更新：已完成10个从零编写练习（含6次无参考挑战），里程碑达成——三相状态机无参考一次编译通过零错误。错误数从21→0，编译轮次从13轮→1轮，所有概念型错误已消灭。难度也在同步升级（两相→三相→三相+多输出），均能2轮以内通过。Git/GitHub全流程学会，练习代码已上传GitHub私有仓库可供展示。下一步：面试准备→开始投简历。成功概率：**中高，接近就绪状态**。
**保留作为备选路线**，与PhD路线不冲突。

#### 路线B：瑞典PhD（神经工程方向） ⭐ 推荐路线 / Sweden PhD Route ⭐ Recommended

| 优势 | 劣势 |
|------|------|
| FES硕士背景直接对口神经工程 | 无论文发表，绩点一般 |
| 瑞典PhD = 带薪工作，不交学费 | 竞争存在，但非顶级热门方向压力较小 |
| PhD期间 = 工作居留许可，满4年可申永居 | 准备周期较长（6-12个月） |
| 学术自主权高（vs 被派遣做别人不想做的工作） | 需要补神经科学和Python知识 |
| 37-38岁毕业后还有30年职业生涯 | — |

### 23.3 FES背景的真实学术价值 / The Real Academic Value of Your FES Background

你觉得自己的硕士研究"很水"，但客观上看：

#### 你实际做过的事（换成学术英文）

```
你做过的                          学术语言翻译
────────                         ────────────
非侵入型FES电极材料对比           → Bioelectrode design and characterization
                                   for transcutaneous electrical stimulation
电极开发和试制                     → Development and prototyping of surface
                                   stimulation electrodes
皮肤-电极界面测量                  → Electrode-skin interface impedance
                                   measurement and analysis
电刺激参数测试                     → Transcutaneous electrical stimulation
                                   protocol design and parameter optimization
实验数据采集和分析                  → Experimental data acquisition and
                                   statistical analysis in biomedical research
ARSO论文撰写（虽然被拒）           → Research paper writing and peer review
                                   experience (submitted to IEEE ARSO 2020)
```

**ARSO论文没中 ≠ 没有research experience。**
你有完整的研究流程经验（提出问题→实验设计→数据采集→分析→撰写→投稿→收到审稿意见）。
很多PhD申请者连这个流程都没走过。

#### FES在神经工程中的位置

```
神经科学/神经工程的完整图景：

  大脑 ──→ 脊髓 ──→ 运动神经 ──→ 肌肉 ──→ 运动
   ↑                    ↑              ↑
   EEG/fMRI           电生理          EMG
   (脑电/功能磁共振)   (神经记录)      (肌电信号)
   
   ↓ BCI 解码          ↓ 神经接口      ↓ FES 刺激 ← ★你在这里★
   
你的硕士研究在"FES刺激"这个环节，这是整个链条的最后一环（输出端）。
读PhD时，你可以自然地往上游扩展（EMG信号处理 → 运动神经解码 → BCI）。
```

#### FES和瑞典热门研究方向的直接关联

| 瑞典热门方向 | 英文 | 和你FES背景的关系 |
|------------|------|-----------------|
| 神经工程 | Neural Engineering | FES是神经工程的核心应用之一 |
| 脑机接口 | Brain-Computer Interface (BCI) | BCI的下游输出常用FES驱动（脑→解码→FES→运动） |
| 神经假体 | Neuroprosthetics | 假肢/外骨骼控制 = FES + EMG + 运动控制 |
| 计算神经科学 | Computational Neuroscience | 用数学模型理解FES刺激的神经响应机制 |
| 康复工程 | Rehabilitation Engineering | 你的硕士正题 |
| 神经信号处理 | Neural Signal Processing | EMG/EEG处理 = FES闭环系统的必要输入 |

### 23.4 瑞典PhD的真实情况 / The Reality of a Swedish PhD

> **瑞典的PhD不是"读书"，是一份带薪工作 (employment)。**

| 项目 | 瑞典PhD的真实情况 | 备注 |
|------|-----------------|------|
| 法律身份 | 大学的正式雇员（employment contract） | 不是学生身份 |
| 薪资 | 约 30,000–34,000 SEK/月（税前） | ≈ 人民币 2万+/月，每年递增 |
| 学费 | **零** — PhD完全不交学费 | 学费由国家/大学承担 |
| 时长 | 4–5 年（含约20%教学任务） | 通常4年研究 + 1年教学 |
| 社保 | 和正式员工完全一样 | 医疗、养老、带薪年假25天 |
| 居留许可 | PhD期间拿工作居留许可 | 不是学生签证 |
| 永居 | 满4年可申请瑞典永久居留权 | PhD本身就够4年 |
| 毕业后 | 可留瑞典工作/创业，或去任何EU国家 | 极大的职业灵活性 |

**这意味着**：你不需要存一大笔学费。去瑞典读PhD本身就是"工作+移民+学术"三合一的路径。

### 23.4b 北欧PhD竞争力的真相 — 冷静分析 / The Real Competition Landscape (2026)

> 2026年4月的自我怀疑记录：担心免学费→竞争大→匹配度要求高→自己没信心。以下是基于事实的冷静分析。

#### 网上常见说法 vs 实际情况

| 网上常见说法 | 实际情况 | 来源/依据 |
|------------|---------|----------|
| "北欧PhD竞争超大" | **热门方向确实卷**（AI/ML/CS几百份申请/岗位），但**神经工程/BCI方向远没那么卷** | 多数神经工程PhD岗位收到10-30份合格申请 |
| "免学费所以全世界都来抢" | PhD是岗位制，不是"申请入学"。PI要的是能做他项目的人，不是GPA最高的人 | 岗位描述通常列出非常具体的技能要求 |
| "要顶刊论文才能申上" | 瑞典很多PhD岗位**只要求硕士学位**，不强制要求发表。有论文是加分，无论文不致命 | 各校PhD vacancy公告的"Requirements"部分 |
| "年纪大没戏" | 瑞典PhD平均入学年龄约28-30，33岁**完全在正常范围**。30-35岁入学非常常见 | 瑞典法律禁止年龄歧视，招聘不允许问年龄 |
| "竞争力 = 论文数 + GPA" | 瑞典PI最看重的是：**研究方向匹配度 + 动机信 + 能否独立工作** | — |

#### 你的实际匹配度（诚实评估）

```
强项 ✅（这些是你的辨识度）:
├── 硕士研究 = FES/电极/电刺激 → 直接对口神经工程
├── 有完整研究流程（实验设计→数据采集→论文撰写→投稿ARSO→收到审稿意见）
├── 嵌入式系统分析背景 → 对BCI硬件端、MCU信号采集系统有理解基础
├── 5年+海外独立工作经验 → 瑞典PI眼里 = 成熟、能独立完成任务
└── 日本大型建设机械制造商经验 → 北欧重工业领域有认知度

弱项 ⚠️（需要正视但非致命）:
├── 无论文发表 → 弱项，但ARSO投稿经历证明走过完整流程
├── 绩点一般 → 瑞典PhD遴选中不是首要标准，PI更看匹配度
└── 无Python/MATLAB神经信号处理经验 → 申请前需要补（见23.5加分操作）

致命弱点 ❌（如果不补会被筛掉）:
└── 如果申请的方向和FES完全无关 → 核心优势就没了
    → 所以精准选岗位是关键，不要广撒网
```

#### 竞争的本质：不是"你 vs 全世界"，而是"你 vs 这个岗位的其他申请者"

```
场景A（你会输的）：
  岗位：Machine Learning for Brain-Computer Interface
  竞争者：欧洲本地CS硕士，有Python/PyTorch经验，有EEG数据分析论文
  你：FES背景，无ML经验 → 完全不匹配 → 必输

场景B（你有机会的）：
  岗位：Design and evaluation of transcutaneous electrical stimulation 
        for motor rehabilitation
  竞争者：多数是biomedical工科硕士，可能没做过实际的电极实验
  你：硕士就是做FES电极的 → 直接相关 → 有辨识度 → 有真实机会

结论：不要看整体竞争，要看具体岗位的匹配度。
      你的策略 = 只申请和FES/电刺激/神经假体/康复工程直接相关的岗位。
```

#### 关于年龄的心理建设

```
你的内心：33岁了，还能读博吗？太晚了吧……

瑞典的现实：
├── PhD = 工作，不是"回去读书"
├── 你的同事会有25岁的也有40岁的
├── 瑞典社会没有"什么年龄该做什么"的概念
├── 北欧人25-27岁硕士毕业是常态，工作几年再读PhD是标准路径
├── 你37-38岁毕业，还有30年职业生涯
└── 你的5年海外工作经验在瑞典PI眼里是加分，不是减分

日本/中国的年龄焦虑 ≠ 北欧的现实。
你需要抛弃的不是信心，而是东亚的年龄时钟。
```

#### 信心不足时的自我检查清单

```
每次觉得"我不行"的时候，回来看这个：

□ 你做过FES电极实验吗？ → 做过。多数申请者没有。
□ 你走过完整的研究流程吗？ → 走过（提出问题→实验→数据→论文→投稿→审稿意见）。
□ 你能独立在海外生活和工作吗？ → 已经做了5年+。
□ 你是否能读懂嵌入式系统代码？ → 能。400+模块的真实工业代码库。
□ 你的年龄在北欧是问题吗？ → 不是。法律禁止年龄歧视。
□ 没论文就不能申PhD吗？ → 不是。瑞典多数PhD岗位不强制要求发表。

如果以上全部是"是"，那你的信心不足来自东亚社会压力的内化，
不是来自你客观能力的缺乏。
```

### 23.5 "33岁，没论文"真的是问题吗？ / Is Age 33 with No Publications Really a Problem?

#### 年龄 / Age

| 国家 | 33岁申PhD的现实 |
|------|---------------|
| 中国 | 社会压力 — 很多人觉得"太晚了" |
| 日本 | 社会压力大，年龄意识强 |
| **瑞典** | **完全正常。** PhD平均入学年龄约28-30岁，30-35岁入学非常常见 |

瑞典法律 **严禁年龄歧视**。PhD遴选只看：学术背景 + 研究匹配度 + 动机信。
瑞典社会文化中"什么年龄该做什么事"这个概念几乎不存在。
北欧人25-27岁硕士毕业是常态，工作几年再回来读PhD是标准操作。

#### 没有论文 / No Publications

| 情况 | 影响 | 弥补方法 |
|------|------|---------|
| 无论文申PhD | ⚠️ 有劣势但**不致命** | 写好动机信，详细说明FES研究内容和学到的东西 |
| ARSO被拒的论文 | 不是零 — 证明你走过完整的研究流程 | 整理成英文摘要附在申请材料里 |
| 绩点一般 | 对瑞典PhD申请影响较小（不是首要标准） | 用研究匹配度和动机信弥补 |

**加分操作（如果能在申请前完成）**：
- 在 arXiv/bioRxiv 上传一篇预印本（哪怕是对自己硕士研究的整理）
- 做一个EMG信号处理的Python项目，放GitHub上
- 上述两项任一完成，都会显著提升竞争力

### 23.6 瑞典目标院校与实验室 / Target Universities & Labs in Sweden

> 以下目标不局限于瑞典。当方向和兴趣明确后，应同时关注其他北欧国家（见23.8节四国对比）。

| 大学 | 方向 | 关键搜索词 | 为什么适合你 |
|------|------|-----------|------------|
| **KTH** Royal Institute of Technology, Stockholm | Neural Engineering, BCI, Biomedical Signal Processing | "KTH neuronic engineering PhD" | 工科背景最匹配，有专门的神经工程方向 |
| **Karolinska Institutet (KI)**, Stockholm | Neuroscience, Neurorehabilitation | "Karolinska neuroscience PhD" | 全球顶级神经科学，和KTH有联合项目 |
| **Chalmers**, Gothenburg | Biomedical Engineering, Neural Prosthetics | "Chalmers neural prosthetics" | ⭐有团队直接做假肢+EMG+FES，最对口 |
| **Lund University** | Neuroscience, Neural Interfaces | "Lund neuronano PhD" | 有实验室做侵入式/非侵入式神经接口 |
| **Linköping University** | Biomedical Engineering | "Linköping biomedical engineering PhD" | 有FES和康复机器人方向的老师 |
| **Uppsala University** | Signal Processing | "Uppsala biomedical signal processing" | 信号处理强校 |

#### 特别推荐：Chalmers — Max Ortiz Catalan 教授

```
研究方向：
├── 神经假肢控制 (Neural control of prosthetics)
├── EMG 运动解码 (EMG-based motor decoding)
├── 幻肢痛治疗 (Phantom limb pain treatment)
├── 骨整合神经接口 (Osseointegrated neural interfaces)
└── 和 FES 直接相关！

为什么推荐：
├── 他的研究和你的FES背景有直接关联
├── 他的团队在临床实际应用（不是纯理论）
├── Chalmers在工程应用方面非常强
└── 可以作为你的论文阅读起点

搜索方法：
Google Scholar → 搜 "Max Ortiz Catalan" → 看最近3年的论文
```

#### PhD岗位搜索平台

| 平台 | 网址 | 说明 |
|------|------|------|
| 各大学官方 Vacancies 页面 | 各校官网搜 "PhD positions" | 最权威，更新最快 |
| Academic Positions Sweden | academicpositions.se | 瑞典学术岗位聚合网站 |
| euraxess.ec.europa.eu | EURAXESS | 欧洲全境学术岗位 |
| FindAPhD.com | findaphd.com | 全球PhD搜索 |

**发布节奏**：瑞典PhD岗位全年滚动发布，集中在 **秋季（9-11月）** 和 **春季（2-4月）**。

### 23.7 过渡方案（如果PhD暂时没拿到） / Backup Plans

| 方案 | 说明 | 优缺点 |
|------|------|--------|
| **瑞典硕士（2年）** | 先申请KTH/Chalmers的Neural Engineering相关硕士 | 缺点：非EU学生有学费（~150,000 SEK/年≈10万RMB），但有Swedish Institute Scholarship等奖学金机会 |
| **Research Assistant** | 有些实验室招短期Research Assistant | 可以作为进入PhD的跳板，薪资略低于PhD |
| **继续走嵌入式工作路线** | 用本笔记21-22章准备的内容冲北欧嵌入式岗位 | 两条线不冲突，可以并行准备 |
| **日本国内PhD** | UEC或其他日本大学的博士课程 | 学费低，但不解决"想离开日本"的核心需求 |

### 23.8 北欧四国PhD详细对比 / Nordic Four Countries PhD Comparison

| | 🇸🇪 瑞典 Sweden | 🇫🇮 芬兰 Finland | 🇳🇴 挪威 Norway | 🇩🇰 丹麦 Denmark |
|---|---|---|---|---|
| **PhD身份** | **雇员** (employee) | 混合：雇员制 + 少量奖学金制 | **雇员** (employee) | **雇员** (employee) |
| **学费** | **零** | **零** | **零** | **零** |
| **月薪(税前)** | ~30,000-34,000 SEK ≈€2,700-3,100 | ~€2,500-3,200 | ~37,000-45,000 NOK ≈€3,200-3,900 | ~33,000-37,000 DKK ≈€4,400-5,000 |
| **薪资排名** | 第3 | 第4 | 第2（但生活成本最高） | 第1（但税也最高） |
| **生活成本** | 中高 | 中（四国最低） | **最高** | 高 |
| **PhD制度** | 几乎全部岗位制 | ⚠️ 部分需自找funding，注意招聘广告 | 几乎全部岗位制 | 几乎全部岗位制 |
| **永居路径** | 4年→可申永居 | 4年→可申永居(A-permit) | **3年→可申永居（最快）** | 较复杂，需额外条件 |
| **英语环境** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **神经工程/BioMed强校** | KTH, Karolinska, Chalmers, Lund | Aalto, Helsinki | NTNU, Oslo | DTU, Copenhagen, Aarhus |
| **对外国人友好度** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| **气候** | 冬长但不极端 | 冬长且冷（四国最冷） | 西海岸湿冷，内陆冷 | 四国中最温和 |

#### 关于学费和岗位制的关键事实

```
北欧四国PhD = 全部免学费。

"岗位制"的意思：
├── 大学发布一个PhD Position（职位空缺）
├── 你申请这个Position → 被录用 = 签劳动合同(employment contract)
├── 你的身份是大学的正式雇员（不是学生）
├── 薪资、社保、年假和正式员工一样
├── 学校的钱已经有了（来自研究项目经费）→ 他们在找人来做项目
├── 你不需要自己找钱/奖学金
└── 这是北欧PhD的标准模式

和中国/日本"考博"的区别：
├── 中国：考试/面试 → 录取 → 自费或拿奖学金(CSC等)
├── 日本：找导师 → 入学考试 → 交学费
├── 北欧：导师有项目经费 → 发布PhD Position → 你投简历申请 → 签合同上班
└── 本质上更接近"找工作"而非"入学读书"

芬兰例外：
├── 部分PhD是非岗位制（需要自找funding或拿奖学金）
├── 看清楚招聘广告：写了 "salary" / "employment contract" 的 = 岗位制（没问题）
├── 写了 "scholarship" / "grant-funded" 的 = 需要自行确认条件
└── 如果分不清 → 直接问招聘方
```

#### 推荐排序

```
第1名: 🇸🇪 瑞典 ⭐ 综合最优
├── 神经工程/生物医学方向最强（KTH + Karolinska联合 = 欧洲顶级）
├── PhD岗位数量最多（大学多、研究经费充足）
├── 永居路径清晰（4年PhD → 直接申请永居）
├── 英语环境最好（瑞典人英语水平全球前3）
├── 对外国人包容度最高
└── Chalmers有直接做FES/EMG/神经假体的实验室

第2名: 🇳🇴 挪威
├── 薪资最高（但生活成本也最高，实际购买力≈瑞典）
├── NTNU(挪威科技大学)工程+生物医学很强
├── 3年就可以申永居（比瑞典快1年）
├── 缺点：岗位总数比瑞典少（大学少）
└── 如果找到匹配的岗位，挪威是非常好的选择

第3名: 🇩🇰 丹麦
├── DTU(丹麦技术大学)工程方向强
├── 哥本哈根大学的神经科学不错
├── 气候四国中最温和
├── 缺点：永居政策比其他三国复杂

第4名: 🇫🇮 芬兰
├── Aalto和Helsinki在BCI方面有不错的研究
├── 生活成本四国中最低
├── 缺点：冬天最冷最暗，部分PhD是奖学金制
└── 如果特别喜欢某个芬兰教授的研究 → 那就去

实际策略：
├── 不要局限在一个国家
├── 同时关注4个国家的PhD岗位
├── 最终去哪里取决于哪个实验室/方向最吸引你
└── 也可以扩展到其他欧洲国家：德国、荷兰、瑞士的PhD也很好
```

### 23.9 竞争现实：你的竞争对手到底是谁？ / Competition Reality

#### "是不是都是清华北大的在和我竞争？"

```
你想象的竞争局面：
  清华本硕 + 3篇SCI → 申北欧PhD
  你（非985硕士 + 无论文）→ 申同一个北欧PhD
  → 你觉得毫无胜算

现实的竞争局面：
  1. 清华北大的人大多数不申北欧PhD
     ├── 他们的首选：美国top校 > 英国 Oxbridge > 瑞士ETH > 新加坡
     ├── 北欧PhD在中国的知名度不高，985学生很看QS排名
     ├── 北欧大学的QS排名（KTH约80-100名）对985学生"不够有面子"
     └── → 你的实际竞争对手没你想的那么强

  2. 北欧PhD的遴选标准和中国/日本完全不同：

     中国博士看重的（你确实不占优）：
     ├── 绩点
     ├── 论文数量
     └── 本科学校名

     北欧PhD看重的（你有机会的）：
     ├── ★研究方向匹配度★（你的背景和这个岗位有多相关？）← 最重要
     ├── ★Motivation letter★（你为什么想做这个？能说清楚吗？）
     ├── 导师的个人判断（聊过之后觉得你行不行）
     ├── 推荐信
     └── 绩点和论文（有参考价值，但不是决定因素）

  3. 你有些方面比985毕业生更有优势：
     ├── 真实的FES实验室经验（很多985直博生从未独立做过完整实验）
     ├── 工业界嵌入式系统经验（纯学术背景的PhD申请者没有）
     ├── 在日本独立生活工作5年（跨文化适应力的证明）
     ├── 33岁 = 比22岁的应届生更清楚自己想要什么
     │   （北欧教授更喜欢"成熟的、想清楚了的"PhD学生）
     └── FES背景直接对口神经工程（比一个做图像识别的985硕士更匹配）
```

#### 一个典型瑞典PhD岗位的申请池

```
一个PhD Position，收到 50-100 份申请。

分布情况（真实比例）：
├── 30-40份 = 完全不相关的海投（简历和岗位不沾边 → 直接淘汰）
├── 20-30份 = 方向大致相关但motivation letter写得很差/群发模板（淘汰）
├── 10-15份 = 认真的申请者（读了导师的论文、letter有针对性）
├── 3-5份  = 进入面试
└── 1份    = 录取

你的目标：成为那10-15个认真申请者之一。
不需要是"全场最强"，只需要是"足够匹配 + 足够认真"。
```

#### 教授选PhD学生的真实心理

```
瑞典教授在想：
"我有一个研究项目，需要招一个人来做。
 这个人需要：
 ├── 有相关背景（不需要从零教起太多东西）
 ├── 能独立工作（不需要我天天盯着）
 ├── 有动机（不会读到一半跑了）
 ├── 好相处（要一起工作4年呢）
 └── 能写出东西（最终要产出论文/毕业论文）

 这个人论文多一篇少一篇？→ 不是最重要的
 来自清华还是非985？→ 大多数瑞典教授根本不知道中国大学排名
 关键是：这个人能不能做好我的项目？和我合得来吗？"
```

#### 关于联系导师的成功率

```
现实预期：
├── 发10封邮件，可能有2-3个回复
├── 回复中可能1个说"目前没有岗位"
├── 1个说"你可以关注我们的岗位发布"  
├── 1个说"我对你的背景感兴趣，我们可以聊聊" ← 只需要1个
├── 0个回复也完全正常（教授很忙，不是针对你）
└── 这和你的能力无关，这是概率游戏

提高回复率的方法：
├── 读了导师的具体论文再写邮件（回复率翻倍）
├── 有可展示的东西（GitHub / 研究摘要）（回复率再翻倍）
├── 邮件简洁有力 <250词（教授3分钟能读完）
├── 发5-10个而不是只发1个（概率论）
└── 没回复的2周后follow-up一次（有些教授真的只是忘了回）

心理建设：
├── 你不是在"求教授收留"
├── 你是在说"我有FES经验+嵌入式技能+自学能力，我能为你的项目贡献价值"
├── PhD招聘是双向选择，不是单方面被面试
└── 联系10个教授，只需要1个match就够了
```

---

## 二十二、研究方向选择与知识地图 / Research Direction & Knowledge Map

### 24.1 四个候选方向对比 / Four Candidate Directions

| | 方向A: Closed-loop FES | 方向B: BCI for Rehab | 方向C: Computational Motor Neuroscience | 方向D: Neural Signal Processing |
|---|---|---|---|---|
| **研究什么** | 用传感器反馈实时调整FES参数 | 读脑电→解码运动意图→控制FES/外骨骼 | 用数学模型理解大脑如何控制运动 | 处理EMG/EEG信号，提取特征，分类/解码 |
| **核心技能** | 控制理论、FES参数优化、实时系统 | EEG采集、机器学习、实时BCI | 微分方程、动力系统、神经元模型 | 信号处理(滤波/FFT)、机器学习、Python |
| **和你FES的关系** | 你硕士的直接延伸 | FES的上游（脑→电信号→FES→肌肉） | FES刺激的是运动神经→理解运动控制原理 | FES需要EMG反馈→信号处理是基础 |
| **上手难度** | 中等（需补闭环控制理论） | 较高（需补脑电+ML） | **较高**（需补大量数学和神经理论） | **较低**（自动化背景有信号处理基础） |
| **就业出路** | 学术+医疗设备 | 学术+BCI公司 | 偏学术 | **学术+工业都行**（医疗设备、BCI创业公司） |
| **瑞典实验室数量** | 较少 | 中等 | 中等 | **较多** |

### 24.2 推荐方向：D（Neural Signal Processing）⭐ / Recommended: Direction D

**理由**：

```
1. 你自动化本科学过信号与系统、数字信号处理 → 有基础
2. 方向D更"做得出东西" → 容易产出结果、写进申请材料
3. 读PhD时可以自然往C方向扩展（先会处理信号，再理解信号背后的神经机制）
4. 方向D在瑞典的实验室更多、PhD岗位更多
5. 就业出路最广（学术/医疗设备公司/BCI创业公司都需要）

同时保持兴趣：
├── 方向C作为PhD期间的知识深化方向
└── 方向A/B作为具体应用场景（FES闭环 / BCI解码）
```

### 24.3 Neural Signal Processing 知识地图 / Knowledge Map

```
你需要掌握的知识体系：

Layer 0: 基础（你已经有的）
├── 信号与系统（自动化本科）
├── 数字信号处理基础（采样、滤波、FFT）
├── 线性代数、概率统计
└── 编程能力（C/C++, 需要补Python）

Layer 1: 神经科学基础（需要补的最小集）
├── 神经元的工作原理（action potential, synapse, neurotransmitter）
│   = 神经元如何产生电信号、如何传递信号
├── 运动控制系统（motor cortex → spinal cord → motor neuron → muscle）
│   = 大脑如何控制肌肉运动
├── EMG（肌电信号）= 运动神经激活肌肉时产生的电信号
│   ├── 表面EMG (sEMG) — 皮肤表面采集（非侵入，和你FES电极研究直接相关）
│   └── 针电极EMG (iEMG) — 插入肌肉内部采集
├── EEG（脑电信号）= 大脑皮层大量神经元同步活动的集体电信号
│   ├── 频段：delta(0.5-4Hz), theta(4-8Hz), alpha(8-13Hz), beta(13-30Hz), gamma(30-100Hz)
│   └── 运动相关：mu rhythm (8-12Hz), beta desynchronization
└── 其他神经信号：ECoG（皮层电图）、LFP（局部场电位）、spike sorting

Layer 2: 信号处理核心技术
├── 预处理：去噪、滤波（bandpass, notch filter for 50/60Hz）、伪迹去除
├── 特征提取：
│   ├── 时域：RMS, MAV, zero-crossing rate, waveform length
│   ├── 频域：PSD (Power Spectral Density), 频段功率比
│   ├── 时频域：STFT, Wavelet Transform, Hilbert-Huang Transform
│   └── 空间域：CSP (Common Spatial Patterns) — 主要用于EEG
├── 降维：PCA, ICA (Independent Component Analysis)
└── 分类/解码：
    ├── 传统ML：SVM, LDA, Random Forest, k-NN
    ├── 深度学习：CNN (用于时频图), LSTM/RNN (用于时间序列), Transformer
    └── 迁移学习：跨被试/跨session的模型适配

Layer 3: 应用场景
├── EMG-based gesture recognition（EMG手势识别）
├── EEG-based motor imagery classification（EEG运动想象分类）
├── FES closed-loop control using EMG feedback（EMG反馈的FES闭环控制）
├── Neuroprosthetic control（神经假体控制）
├── Brain-computer interface decoding（脑机接口解码）
└── Neural signal quality assessment（神经信号质量评估）
```

### 24.4 最小知识清单（2周内搞定） / Minimum Knowledge Checklist (2 Weeks)

```
□ 神经元的基本工作原理
  - Action potential（动作电位）= 神经元的"一次放电"
  - Synapse（突触）= 神经元之间的连接点
  - Excitatory vs Inhibitory（兴奋性 vs 抑制性）

□ EMG 是什么
  - 肌电信号 = 运动神经→肌肉收缩时产生的电信号
  - 表面EMG的频率范围：约 20-500 Hz
  - 振幅范围：约 0.01-5 mV
  - 和你FES电极研究的关联：同样是皮肤表面的电信号采集

□ EEG 是什么
  - 脑电信号 = 大脑皮层大量神经元同步活动的集体信号
  - 频率范围：0.5-100 Hz
  - 振幅范围：约 1-100 μV（比EMG小得多）
  - 关键频段和含义（delta/theta/alpha/beta/gamma）

□ 信号处理基础回忆
  - 采样定理（Nyquist）：采样率 ≥ 2 × 最高频率
  - 滤波器设计：低通、高通、带通、陷波（50/60Hz工频干扰）
  - 频谱分析：FFT, PSD

□ EMG/EEG的常见处理流程
  - 原始信号 → 预处理(滤波/去噪) → 特征提取 → 分类器 → 输出
  - 这和嵌入式系统的"传感器→处理→判断→输出"流水线思路完全一样

□ 分类/解码的基本思路
  - 特征提取（从原始信号中提取有意义的数字/向量）
  - 分类器训练（用已知标签的数据训练模型）
  - 在线解码（用训练好的模型实时分类新信号）
```

### 24.5 控制工程 × 神经工程 — 你的隐藏王牌 / Control Engineering × Neural Engineering

> 2026年4月的问题："我本科是自动化的，对控制工程和自动化理论也感兴趣，能用到PhD和北欧找工作上吗？"
> 答案：**不仅能用，而且是你最被低估的资产。**

#### 你本科学过的东西在神经工程中的对应关系

```
自动化本科课程               神经工程/康复工程中的直接对应
──────────────              ─────────────────────────
自动控制原理                 → 闭环FES控制（PID/自适应控制调节电刺激参数）
信号与系统                   → EMG/EEG信号处理（滤波、频谱分析、系统响应）
数字信号处理                 → 神经信号解码（FFT、小波变换、特征提取）
线性代数                     → 机器学习（SVM、PCA、CSP的数学基础）
概率统计                     → 统计推断、分类器评估、实验设计
传感器与检测技术              → 生物传感器（EMG电极、EEG帽、力传感器）
微机原理/嵌入式               → BCI硬件（MCU采集EMG → 实时处理 → 控制输出）
过程控制/系统建模              → 神经肌肉系统建模（输入=电刺激，输出=肌力/关节角度）

★ 你的自动化本科 = 神经工程PhD的完美预科。
  这不是巧合 — 神经工程本质上就是"控制 + 信号处理 + 生物"。
```

#### 控制工程在神经工程中的具体应用

```
应用1：Closed-loop FES（闭环功能性电刺激）⭐ 和你硕士研究直接相关

  传统FES（你硕士做的）= 开环：
    设定刺激参数 → 电刺激 → 希望肌肉收缩 → 结束
    问题：刺激参数固定，肌肉疲劳后效果下降

  闭环FES = 加入反馈控制：
    EMG/力传感器采集肌肉状态 → 控制器 → 实时调整刺激参数
    
    控制框图（你本科学过的）：
    r(t)        e(t)        u(t)         y(t)
    目标角度 ──→ ⊕ ──→ 控制器(PID) ──→ FES刺激器 ──→ 肌肉/关节
                 ↑ -                                    │
                 └────── EMG/力/角度传感器 ←──────────────┘
    
    这不就是你本科学的闭环控制系统吗？
    只是把"电机"换成了"肌肉"，把"编码器"换成了"EMG传感器"。
    
  研究热点：
  ├── 肌肉的非线性特性 → 需要非线性控制/自适应控制
  ├── 肌肉疲劳 → 参数时变 → 需要在线辨识 + 自适应算法
  ├── 延迟（电刺激→肌肉响应有约50-100ms延迟）→ 预测控制
  └── 多关节协调控制 → MIMO系统

应用2：BCI（脑机接口）中的控制

  脑信号 → 解码 → 控制信号 → 机器人/假肢/轮椅/FES
  
  控制理论在BCI中的角色：
  ├── 共享控制（shared control）：
  │   用户的脑信号 = 大致意图 → 控制器补全细节动作
  │   例：用户想"抓杯子"→ BCI检测意图 → 控制器规划手臂轨迹
  ├── 自适应解码器：
  │   脑信号随时间漂移 → 解码模型需要在线自适应
  │   = 你本科学的自适应控制的思路
  └── 稳定性分析：
      BCI系统需要保证闭环稳定（不能让假肢失控）
      = Lyapunov稳定性 / 鲁棒控制

应用3：康复机器人控制

  外骨骼/康复机器人 + FES + 人的运动意图 = 复杂的人机耦合系统
  
  控制挑战：
  ├── 阻抗控制（impedance control）：
  │   机器人需要适应人的运动，不是强制人跟着机器
  ├── 力/位混合控制：
  │   同时控制关节角度和接触力
  ├── 意图识别 + 轨迹规划：
  │   EMG → 识别运动意图 → 规划辅助轨迹 → 电机+FES协同
  └── 安全约束：
      力矩/速度限幅（和你看的刹车灯控制逻辑一样，都是安全约束）

应用4：计算神经科学中的控制模型

  大脑如何控制运动？→ 这本身就是一个控制问题
  ├── 最优控制理论（LQR, LQG）解释人类运动规划
  ├── 卡尔曼滤波器模型解释人脑如何融合感觉信号
  ├── 内部模型（internal model）= 大脑里有一个"被控对象的模型"
  │   前馈控制 = 预测式控制（不等反馈就先动作）
  │   这和你本科学的前馈+反馈复合控制完全对应
  └── 强化学习 = 运动学习的数学模型
```

#### 你现在看的代码就是控制逻辑的实例

```
你选中的 brake_lamp_model_function.cc 中的代码：

  if (!is_diag_normal_brake_press_sensor_.GetValue()) {
    if (!is_diag_normal_brake_stroke_sensor_.GetValue()) {
      service_brake_state = OFF;
    } else {
      service_brake_state = brake_pedal_state;
    }
  } else {
    service_brake_state = brake_press_state;
  }

这是一个典型的传感器冗余切换逻辑（sensor redundancy logic）：
├── 正常情况：用压力传感器（brake_press）
├── 压力传感器故障：切到行程传感器（brake_pedal）
├── 两个都故障：强制OFF（安全状态）
└── = 故障安全（fail-safe）策略

用控制工程的语言说：
├── 这是一个离散事件系统的状态切换
├── 涉及传感器冗余（redundancy）和故障检测（fault detection）
├── 和FES闭环控制的需求完全对应：
│   EMG传感器故障了怎么办？→ 切换到备用传感器还是安全停止？
│   → 这就是你本科学的容错控制（fault-tolerant control）

你在工作中分析的每一个模块的控制逻辑，都是你控制工程基础的实践验证。
```

#### 控制工程在北欧PhD中的具体方向

| PhD方向 | 控制理论的角色 | 北欧相关实验室 | 和你的匹配度 |
|---------|-------------|-------------|------------|
| **Closed-loop FES** | PID/自适应/模型预测控制 | Chalmers, Aalborg (DK) | ⭐⭐⭐⭐⭐ 最高 — FES硕士 + 控制本科 |
| **Rehabilitation Robotics** | 阻抗控制/力控制/轨迹规划 | KTH, Chalmers, NTNU | ⭐⭐⭐⭐ |
| **BCI + Shared Control** | 共享控制/自适应控制 | KTH, Lund, DTU | ⭐⭐⭐ |
| **Neural Signal Processing** | 自适应滤波/Kalman滤波 | 多个 | ⭐⭐⭐ |
| **Computational Motor Control** | 最优控制/内部模型 | KTH, Lund | ⭐⭐⭐ |

```
★ 重新评估你的最优PhD方向 ★

之前推荐的方向D（Neural Signal Processing）仍然是好选择，
但如果你对控制理论有真实的兴趣，方向A（Closed-loop FES）
的匹配度其实更高：

  方向A的优势：
  ├── FES硕士 + 控制本科 = 完美的双重匹配
  ├── 申请时的叙事更自然："我做过FES电极，学过控制理论，
  │   我想把两者结合做闭环FES控制"
  ├── 竞争对手更少（做信号处理的人多，做FES控制的人少）
  └── 北欧有几个专门做这个的实验室（Aalborg大学是全球FES研究中心）

  方向A的注意：
  ├── PhD岗位数量比方向D少（更小众）
  ├── 需要补现代控制理论（自适应控制、模型预测控制）
  └── 但你有控制本科基础 → 补起来比别人快很多

  建议：方向A和方向D都关注，看到哪个岗位先出来就申哪个。
  两个方向的技能有很大重叠（信号处理 + Python都需要）。
```

#### 特别推荐：丹麦Aalborg大学 — FES控制研究全球领先

```
Aalborg University (AAU), Denmark
├── Department of Health Science and Technology
├── 全球FES研究的核心基地之一
├── 有团队专门做：
│   ├── Closed-loop FES for motor rehabilitation
│   ├── EMG-based control of FES
│   ├── Neural prosthetics
│   └── Neuromuscular modeling and control
├── 你的FES硕士 + 控制本科 → 和Aalborg的方向高度匹配
├── 丹麦PhD也是带薪岗位制（和瑞典一样）
└── 搜索: "Aalborg University FES PhD" 或看 hst.aau.dk

注意：之前的院校列表（23.6）主要是瑞典的，
      如果你关注FES控制方向，丹麦Aalborg应该加入你的目标名单。
```

#### 控制工程在北欧嵌入式工作市场的价值

```
如果走工作路线（而非PhD），控制工程背景在北欧也非常有用：

北欧嵌入式岗位中经常出现的关键词：
├── "Control Systems Engineer" — 直接对口
├── "ADAS / Autonomous Systems" — 自动驾驶需要控制理论
├── "Motion Control" — 电机控制/机器人控制
├── "Model-Based Design" — Simulink/MATLAB建模
├── "Functional Safety (ISO 26262)" — 你的刹车灯代码就是安全逻辑
└── "Systems Engineer" — 系统级设计，控制+嵌入式的交叉

北欧重工业/技术公司：
├── ABB（瑞典/瑞士）— 机器人控制、工业自动化 ← 直接对口
├── Volvo（瑞典）— 自动驾驶、动力总成控制
├── Scania（瑞典）— 重型车辆控制系统
├── Ericsson（瑞典）— 通信系统
├── Wärtsilä（芬兰）— 船舶/能源控制系统
├── Kongsberg（挪威）— 防务/海事控制系统
├── Danfoss（丹麦）— 液压/电驱动控制 ← 和建机行业相关
├── Vestas（丹麦）— 风力发电控制
├── Nokia（芬兰）— 通信
└── Epiroc（瑞典）— 矿山设备自动化 ← 和建设机械制造商经验直接相关

你的组合：自动化本科 + 建设机械制造商嵌入式经验 + 控制理论兴趣
→ 在北欧重工业/自动化公司非常有市场
→ 特别是ABB、Volvo、Epiroc这些公司
```

#### 如何深入学习控制工程

```
你本科学过自动控制原理 → 现在需要补的是"现代控制"和"应用方向"。

第一步：复习经典控制（你本科学过，只需要唤醒）
├── PID控制（工业界90%的控制器都是PID）
├── 传递函数、Bode图、根轨迹
├── 稳定性分析（Routh/Nyquist/Bode判据）
├── 推荐：翻翻本科教材，2-3天就能回忆起来
└── 或看 Brian Douglas 的 YouTube 频道（控制理论可视化，极好）

第二步：补现代控制（PhD/高级岗位需要）
├── 状态空间表示法（你分析的嵌入式状态机 → 就是状态空间的工程实现）
├── 可控性/可观性（线性代数）
├── 最优控制（LQR — Linear Quadratic Regulator）
├── 卡尔曼滤波（Kalman Filter）— 信号处理+控制的交叉点
├── 推荐课程：Steve Brunton (Univ. of Washington) YouTube系列
│   "Control Bootcamp" — 免费、工程导向、有Python代码
└── 总计：约4-6周，每天30-60分钟

第三步：应用方向（选一个深入）
├── 选项A：闭环FES控制（如果走PhD方向A）
│   → 读 "Functional Electrical Stimulation: Standing and Walking 
│      After Spinal Cord Injury" (Springer) 的控制章节
│   → 搜索论文："PID control FES" / "adaptive control FES" / 
│      "model predictive control FES"
├── 选项B：机器人控制（如果走工作路线）
│   → 学 Simulink/MATLAB 建模
│   → 搜索课程："Robot Control" on edX/Coursera
└── 选项C：用Python实现控制算法（两个方向都有用）
    → pip install python-control
    → 用Python实现PID、LQR、Kalman Filter
    → 放GitHub上 = 面试谈资

推荐的学习资源：
├── YouTube: Brian Douglas "Control Systems" — 经典控制可视化
├── YouTube: Steve Brunton "Control Bootcamp" — 现代控制+Python
├── Coursera: "Modern Robotics" by Kevin Lynch (Northwestern)
├── 书: "Feedback Control of Dynamic Systems" (Franklin et al.)
└── 书: "Modern Control Engineering" (Ogata) — 你本科可能用过的教材
```

#### 控制工程在面试中的叙事

```
PhD面试时怎么说：
"My undergraduate degree is in Automation Engineering, where I 
 studied control theory, signal processing, and embedded systems. 
 My Master's research on FES gave me experience with the biological 
 side — electrode design and stimulation protocols. Now I want to 
 combine these two: using control theory to create closed-loop FES 
 systems that can adaptively adjust stimulation based on neural 
 signal feedback."

嵌入式工作面试时怎么说：
"I have a background in control engineering from my undergraduate 
 studies, and I've spent two years analyzing embedded control systems 
 at a major construction equipment manufacturer. I've traced signal 
 flows through 400+ software modules implementing sensor processing, 
 state machines, and safety logic — like sensor redundancy switching 
 and fail-safe control strategies."

关键：控制工程不是一个单独的技能，而是一个思维框架。
├── 读代码时：你看到的是输入→处理→输出→反馈循环
├── 分析系统时：你想的是稳定性、鲁棒性、故障模式
├── 设计实验时：你想的是控制变量、观测量、噪声
└── 这些思维模式在PhD和工业界都极其有价值
```

### 25.1 从零开始的入门资源 / Starting Resources

#### 在线课程（选1个，每天30-40分钟）

| 课程 | 平台 | 费用 | 为什么推荐 |
|------|------|------|-----------|
| **"Computational Neuroscience"** by Rajesh Rao (Univ. of Washington) | Coursera | 免费旁听 | 最经典的入门课，覆盖神经元模型、信号编码、运动控制 |
| **Neuromatch Academy — Computational Neuroscience** | neuromatch.io | 完全免费 | 用Python实操，边学边写代码，最适合工科背景 |
| **Andrew Ng — Machine Learning** | Coursera | 免费旁听 | 补机器学习基础（分类器那部分需要） |

**只需完成前4-5周内容**，不需要学完整门课。目标是建立基本概念框架。

#### 参考书（当字典用，不需要从头读）

| 书名 | 作者 | 用途 |
|------|------|------|
| **"Introduction to Neural Engineering for Motor Rehabilitation"** | Dario Farina et al. | ⭐最推荐 — 直接覆盖FES+EMG+神经信号处理+运动控制 |
| "Bioelectricity: A Quantitative Approach" | Plonsey & Barr | 生物电学基础（理解你的FES电极研究的理论背景） |
| "Pattern Recognition and Machine Learning" | Bishop | 机器学习经典教材（分类器部分） |

### 25.2 论文搜索工具 / Paper Search Tools

| 工具 | 网址 | 用途 |
|------|------|------|
| **Google Scholar** | scholar.google.com | 搜论文、看引用次数、看被引关系 |
| **Semantic Scholar** | semanticscholar.org | AI推荐相关论文，比Google Scholar更智能 |
| **Connected Papers** | connectedpapers.com | 输入一篇论文→可视化显示相关论文网络（强烈推荐） |
| **PubMed** | pubmed.ncbi.nlm.nih.gov | 生物医学论文数据库 |
| **arXiv / bioRxiv** | arxiv.org / biorxiv.org | 预印本平台（最新研究，尚未同行评审） |

### 25.3 种子论文扩展法 / Seed Paper Expansion Method

**不要漫无目的地搜。** 用"种子扩展法"系统地建立阅读网络：

```
第1步：找3篇"种子论文"（Review Paper = 综述论文）
  综述论文 = 有人帮你把某个领域最近几年的重要研究全部整理好了
  一篇好的综述 = 这个领域的地图
       ↓
第2步：从种子论文的参考文献里，挑出反复被引用的关键原始论文
  如果三篇综述都引用了同一篇论文 → 那篇一定是领域内的里程碑
       ↓  
第3步：用 Connected Papers 或 Google Scholar 的 "Cited by" 功能
  找到引用这些关键论文的最新研究（= 这个方向目前在做什么）
       ↓
第4步：逐渐聚焦到你最感兴趣的具体话题和实验室
  注意观察：哪些作者反复出现？他们在哪个大学？
```

### 25.4 你的3篇种子论文 / Your 3 Seed Papers (Start Here)

#### 种子1 — EMG信号处理综述（方向D核心） / EMG Signal Processing Review

```
搜索词: "surface EMG signal processing review"
目标论文:
  "Surface Electromyography Signal Processing and Classification Techniques"
  或任何 Dario Farina（神经工程大牛，现在Imperial College London）的综述

Dario Farina 的研究覆盖：
├── sEMG信号分解 (EMG decomposition)
├── 运动意图解码 (Motor intention decoding)
├── 神经假肢控制 (Neural prosthetics control)
└── 人机接口 (Human-machine interfaces)

搜索方法：Google Scholar → 搜 "Dario Farina EMG review"
```

#### 种子2 — BCI + 运动解码综述（方向D偏应用） / BCI Motor Decoding Review

```
搜索词: "EEG motor imagery classification review"
或: "brain-computer interface motor rehabilitation review"

推荐论文:
  "A review of classification algorithms for EEG-based brain-computer interfaces: 
   a 10 year update" (2018, Lotte et al.)
  — 这篇涵盖了EEG分类的所有主流方法

为什么要读：
├── 了解EEG信号处理的标准流程
├── 了解常用分类算法（SVM, LDA, CNN, CSP...）
└── BCI是神经信号处理最热门的应用
```

#### 种子3 — FES + 闭环控制综述（连接你的硕士研究） / FES Closed-loop Review

```
搜索词: "closed-loop FES control review"
或: "FES rehabilitation neural control review"

推荐论文:
  "Functional Electrical Stimulation and its use during cycling 
   exercise: a review" 或类似综述

为什么要读：
├── 从你熟悉的FES出发，看它如何和EMG反馈结合
├── 了解闭环FES的当前研究前沿
├── 这是你写动机信时"从硕士到PhD的桥梁"
└── 读这篇时你会有最强的"啊这个我懂"的感觉（建立信心）
```

### 25.5 论文阅读三遍法 / Three-Pass Paper Reading Method

#### 第一遍（5-10分钟）：判断"值不值得读" / First Pass: Triage

```
只看这几个部分：
□ Title → 大概什么话题？
□ Abstract → 做了什么、结论是什么？（最重要的200个词）
□ Figures → 翻一遍所有图表，能看懂几个？
□ Conclusion → 最终发现了什么？

判断标准：
  → 和我的兴趣方向相关吗？
  → 方法/技术我能大概理解吗？
  → 相关 → 进入第二遍
  → 不相关 → 放弃，下一篇（不要纠结）
```

#### 第二遍（30-60分钟）：理解整体框架 / Second Pass: Comprehension

```
□ Introduction
  → 这个领域的背景是什么？
  → 他们要解决什么问题？（= "gap"，文献综述中发现的空白）
  → 他们的方法新在哪里？

□ Methods
  → 用了什么数据？（多少被试、什么采集设备）
  → 什么信号处理流程？（滤波→特征→分类器）
  → 什么评估指标？（accuracy, F1-score, AUC...）

□ Results
  → 结果好坏？和之前的方法比怎么样？
  → 看图表就够了，不需要记住每个数字

□ Discussion
  → 他们承认了哪些局限？（= 你未来可以改进的方向！）
  → 未来工作建议是什么？

可以跳过：
  - 看不懂的数学推导（记下来，回头补）
  - 非常细节的实验参数设置
```

#### 第三遍（只对最重要的2-3篇做） / Third Pass: Deep Dive

```
□ 逐段精读
□ 复现思路：如果我来做这个实验，我会怎么设计？
□ 找不懂的术语 → Google / YouTube 搜教程
□ 在 Semantic Scholar 上看 "Cited by" → 后续有人做了什么改进？
□ 思考：这个方法的局限是什么？我能想到改进吗？
```

### 25.6 论文笔记模板 / Paper Note Template

每篇论文用以下格式记录：

```markdown
## [论文标题]

- **作者/年份/期刊**: 
- **一句话总结**: （用自己的话，不超过2句）
- **问题**: 他们要解决什么问题？
- **方法**: 用了什么核心方法/技术？
- **数据**: 用了什么数据集？多少被试？
- **主要结果**: 关键数字（如"分类准确率 95.2%"）
- **局限性**: 作者自己承认的 + 我发现的
- **和我的兴趣的关联**: 
- **我不懂的概念（待补）**: 
- **可以改进的地方**: ← ★最重要！这就是你未来的研究idea来源★

标签: #EMG #classification #deep_learning  （方便后续检索）
```

### 25.7 Python + 神经信号实操资源 / Hands-on Python Resources

读论文的同时，动手跑代码会加速理解10倍。

#### 免费数据集

| 数据集 | 来源 | 内容 |
|--------|------|------|
| **PhysioNet** | physionet.org | 大量免费的EMG/EEG/ECG数据集 |
| **BCI Competition datasets** | bbci.de/competition | 标准的BCI评测数据集（EEG motor imagery） |
| **Ninapro** | ninapro.hevs.ch | EMG手势识别数据集（多个被试，多种手势） |
| **MOABB** | github.com/NeuroTechX/moabb | Python库 + 多个BCI数据集的统一接口 |

#### Python 库

```
必装：
├── numpy, scipy          — 数值计算、信号处理
├── scikit-learn          — 机器学习（SVM, LDA, RandomForest...）
├── matplotlib            — 画图
├── mne-python (mne.tools) — 专门处理 EEG/MEG 数据的库（行业标准）
└── pandas                — 数据整理

高级（后续用到时再装）：
├── pytorch / tensorflow  — 深度学习
├── nilearn               — fMRI 数据处理
└── pyriemann             — 黎曼几何分类器（BCI前沿方法）
```

#### 建议的第一个实操项目

```
项目：EMG 手势分类 (EMG Gesture Classification)

步骤：
1. 从 Ninapro 或 PhysioNet 下载一个 EMG 手势数据集
2. 用 Python 读取数据，画出原始 EMG 波形
3. 预处理：带通滤波 (20-500Hz) + 整流 + 平滑
4. 特征提取：计算 RMS、MAV、过零率等时域特征
5. 分类：用 scikit-learn 的 SVM 或 Random Forest 分类不同手势
6. 评估：计算准确率、混淆矩阵

这个项目：
├── 和你的FES电极研究有直接关联（同样是皮肤表面的肌电信号）
├── 覆盖了完整的信号处理流程
├── 可以写进CV / 放 GitHub
└── 面试/申请时可以展示（"我做了一个EMG分类项目"）
```

### 25.8 Python/MATLAB 从零到可展示的具体路线图 / Concrete Python/MATLAB Roadmap

> 你的起点：C/C++能读懂，Python/MATLAB接近零基础。
> 你的目标：做出一个能放GitHub、能写进CV、面试时能讲清楚的神经信号处理项目。
> 关键：不用精通，只要"做出了一个真实的东西"就够了。

#### 为什么选Python而不是MATLAB？

```
Python：
├── 免费（MATLAB要付费或学校授权）
├── 神经信号处理社区的主流（mne-python, scikit-learn, PyTorch）
├── PhD面试时说"I use Python"比"I use MATLAB"更受欢迎
├── 就业也用得上（嵌入式测试脚本、数据分析都用Python）
└── 北欧实验室几乎都用Python

MATLAB：
├── 也很常见（特别是老一辈教授的实验室）
├── Signal Processing Toolbox 非常强大
├── 如果你申请的实验室明确用MATLAB → 到时候再学也来得及
└── 你有信号与系统的本科基础 → MATLAB上手不难

结论：先学Python，以后需要MATLAB再补。
```

#### 第0周：环境搭建（半天搞定）

```
1. 安装 Anaconda（包含Python + Jupyter Notebook + 常用库）
   → 下载: anaconda.com/download
   → 安装时勾选 "Add to PATH"

2. 打开 Jupyter Notebook，运行:
   import numpy as np
   import matplotlib.pyplot as plt
   x = np.linspace(0, 2*np.pi, 100)
   plt.plot(x, np.sin(x))
   plt.title("My first plot")
   plt.show()
   
   → 看到正弦波 = 环境OK

3. 安装神经信号处理库:
   pip install mne scikit-learn pandas

完成标志：能在Jupyter里画出一个正弦波。
```

#### 第1-2周：Python基础（每天30-60分钟）

```
你会C/C++ → Python上手很快。重点学这些：

□ 变量、列表、字典（C的struct → Python的dict）
□ for循环、if/else（和C几乎一样，只是缩进代替大括号）
□ 函数定义（def代替void/int）
□ numpy数组操作（= 信号处理的核心）
  → np.array, np.zeros, np.linspace
  → 数组加减乘除（向量化运算，不用写for循环）
  → 切片：arr[10:50]（取第10-49个元素）
□ matplotlib画图（plt.plot, plt.subplot, plt.xlabel）
□ 读写文件（pandas.read_csv → 读数据集）

推荐资源（选1个，不要贪多）：
├── Kaggle "Python" 课程（免费，2-3小时搞定）
├── 或直接跟着下面的项目边做边学（遇到不懂的Google一下）
└── ❌ 不推荐从头看500页Python教材

完成标志：能用numpy生成一个信号，用matplotlib画出来。
```

#### 第3-4周：信号处理复习 + Python实操（每天40-60分钟）

```
你本科学过信号与系统 → 现在用Python复现你学过的东西。

□ 生成模拟信号：
  import numpy as np
  fs = 1000  # 采样率 1000Hz
  t = np.arange(0, 1, 1/fs)  # 1秒
  signal = np.sin(2*np.pi*10*t) + 0.5*np.sin(2*np.pi*50*t)  # 10Hz + 50Hz
  # → 画出来，看到两个频率叠加的波形

□ FFT频谱分析：
  from scipy.fft import fft, fftfreq
  Y = fft(signal)
  freq = fftfreq(len(t), 1/fs)
  plt.plot(freq[:len(freq)//2], np.abs(Y[:len(Y)//2]))
  # → 看到10Hz和50Hz两个峰

□ 滤波器设计：
  from scipy.signal import butter, filtfilt
  b, a = butter(4, 30/(fs/2), btype='low')  # 30Hz低通
  filtered = filtfilt(b, a, signal)
  # → 画出来，50Hz成分被滤掉了

□ 这些你本科都学过！区别只是工具从MATLAB/手算变成了Python。

完成标志：能用Python做FFT + 滤波，并画出结果。
```

#### 第5-6周：EMG信号处理入门项目 ⭐（核心产出）

```
★ 这就是你要放GitHub的项目 ★

项目名：EMG Signal Processing and Gesture Classification
仓库结构：
  emg-gesture-classification/
  ├── README.md          ← 英文，说明项目目的和方法
  ├── data/              ← 数据集（或下载链接）
  ├── notebooks/
  │   ├── 01_data_loading.ipynb
  │   ├── 02_preprocessing.ipynb
  │   ├── 03_feature_extraction.ipynb
  │   └── 04_classification.ipynb
  └── requirements.txt

步骤：
1. 下载 Ninapro DB1（ninapro.hevs.ch）
   → 10个被试，52种手势，表面EMG信号
   → 或用 PhysioNet 的更小的EMG数据集

2. 01_data_loading.ipynb:
   → 用scipy或pandas读取数据文件
   → 画出原始EMG波形（多通道）
   → 标注不同手势对应的时间段

3. 02_preprocessing.ipynb:
   → 带通滤波 20-500Hz（EMG有效频段）
   → 50Hz陷波滤波（去工频干扰）
   → 全波整流（np.abs）
   → 包络提取（低通滤波后的整流信号）

4. 03_feature_extraction.ipynb:
   → 滑动窗口（200ms窗口，50ms步长）
   → 每个窗口计算特征：
     ├── RMS（均方根）
     ├── MAV（平均绝对值）
     ├── ZC（过零率）
     ├── WL（波形长度）
     └── 频域PSD特征
   → 得到特征矩阵 [样本数 × 特征数]

5. 04_classification.ipynb:
   → from sklearn.svm import SVC
   → from sklearn.model_selection import train_test_split
   → 训练SVM / Random Forest / LDA
   → 计算准确率，画混淆矩阵
   → 比较不同分类器的结果

这个项目的价值：
├── 证明你会Python + 信号处理 + 机器学习
├── 和你的FES电极研究直接相关（同样是surface EMG/肌电）
├── 覆盖了PhD岗位要求的核心技能
├── README写好了 = 面试时的谈资
└── 总工作量：约30-50小时（分散在2周内）
```

#### 第7-8周（可选加分）：进阶项目选择

```
如果基础项目做完了还有精力，选一个做：

选项A：EEG Motor Imagery Classification
  → 用BCI Competition数据集
  → 特征：CSP (Common Spatial Patterns)
  → 分类：LDA
  → 更对口BCI方向的PhD岗位

选项B：实时EMG可视化
  → 用Arduino + EMG传感器（如MyoWare，约100-200元）
  → Python实时读取串口数据 + 实时画图
  → 硬件+软件结合 = 更能展示嵌入式背景

选项C：将基础项目用深度学习重做
  → CNN处理时频图（STFT → 2D image → CNN分类）
  → 比较传统ML vs 深度学习的结果
  → 需要PyTorch基础

推荐：先把基础项目做好放GitHub，加分项以后有时间再做。
```

#### MATLAB（仅在需要时）

```
如果你申请的实验室明确使用MATLAB：
├── MATLAB Online（mathworks.com）有30天免费试用
├── 信号处理的代码逻辑和Python几乎一样
│   （Python的 scipy.signal ≈ MATLAB的 Signal Processing Toolbox）
├── 你不需要专门学MATLAB → 申到PhD后再学也来得及
└── 面试时如果被问"会MATLAB吗？"
    → "I have experience with Python for signal processing. 
       I haven't used MATLAB extensively, but given the 
       similarity in signal processing workflows, I'm confident 
       I can pick it up quickly."
```

#### 时间总结

```
第0周：环境搭建              — 半天
第1-2周：Python基础           — 每天30-60分钟
第3-4周：信号处理复习+Python  — 每天40-60分钟
第5-6周：EMG项目 ★           — 每天60-90分钟（核心产出）
第7-8周：进阶（可选）         — 按兴趣

总计：约8周，每天平均1小时
产出：一个可展示的GitHub项目 + 信号处理Python技能
```

---

## 二十四、联系导师与申请材料准备 / Contacting Professors & Application Materials

### 26.1 联系潜在导师的邮件模板 / Email Template for Contacting Professors

**在瑞典，PhD申请最有效的方式是直接联系导师。**
不要只投在线系统，先发邮件建立联系。教授如果对你感兴趣，会告诉你什么时候有岗位。

```
Subject: Prospective PhD Applicant — FES / Neural Signal Processing Background

Dear Prof. [Name],

I am writing to express my interest in pursuing a PhD in your group 
at [University].

I hold a Master's degree in Mechanical and Intelligent Systems Engineering 
from the University of Electro-Communications (UEC Tokyo), where my 
research focused on non-invasive Functional Electrical Stimulation (FES). 
Specifically, I investigated surface electrode materials and their 
characterization for transcutaneous electrical stimulation, including 
electrode-skin impedance analysis and stimulation protocol design.

Your recent work on [提到导师的一篇具体论文的标题或主题] resonated 
strongly with my background, particularly [说明具体哪一点相关]. 
I am keen to extend my experience in bioelectrode interfaces toward 
[说明你想做的研究方向, e.g., "EMG signal processing and motor 
intention decoding for rehabilitation applications"].

I am currently working in the embedded systems industry in Japan, 
where I have gained experience in real-time signal processing and 
safety-critical system design. I am motivated to return to academic 
research in neural engineering.

Would you have any upcoming PhD openings, or could you advise on 
how best to apply to your group? I have attached my CV for your 
reference.

Thank you for your time, and I look forward to hearing from you.

Best regards,
[Your Full Name]
[Your email]
```

#### 邮件要点 / Key Points

```
必须做到的：
├── 提到导师的具体论文（证明你真的读了，不是群发）
├── 说明你的背景和他研究的具体关联
├── 简洁 — 不超过200-250词
├── 附上CV（PDF格式）
└── 用学术语气，不要过度谦虚也不要吹嘘

不要做的：
├── ❌ 不要群发模板邮件（教授一眼看出）
├── ❌ 不要写"I am very interested in your research"但不说清楚为什么
├── ❌ 不要写太长的邮件（教授没时间读1000词）
├── ❌ 不要在邮件里说自己"不够好"或"成绩一般"
└── ❌ 不要只发1个教授 — 发5-10个，提高成功率
```

### 26.2 申请材料清单 / Application Materials Checklist

| 材料 | 说明 | 准备优先级 |
|------|------|-----------|
| **CV (学术简历)** | 教育、研究经历（FES）、技能、语言能力。不写年龄/照片（北欧规范） | 🔴 P0 — 联系导师时就要附上 |
| **Motivation Letter** | ⭐最重要⭐ 为什么你想做这个方向、你的背景如何支撑、你想在PhD期间达成什么。800-1000词 | 🔴 P0 |
| **硕士成绩单（英文版）** | 找UEC教务处开具英文成绩单 | 🔴 P0 — 需要时间办理 |
| **学位证明（英文版）** | 硕士学位证的英文翻译/认证 | 🔴 P0 |
| **推荐信 × 2** | ①硕士导师（必须） ②一位了解你的教授或上司 | 🟡 P1 — 早点联系推荐人 |
| **英语证明** | IELTS 6.5+ 或 TOEFL iBT 90+ | 🟡 P1 — 大多数瑞典PhD要求 |
| **硕士论文/研究摘要** | 英文，1-2页。含FES研究内容、方法、结果 | 🟡 P1 |
| **ARSO论文草稿** | 虽然被拒，但整理成可展示的形式附在申请材料中 | 🟢 P2 |
| **GitHub项目** | EMG信号处理小项目（见25.7节） | 🟢 P2 — 加分项 |

### 26.3 Motivation Letter 写法框架 / How to Write the Motivation Letter

```
第1段：Opening（100词）
  "我是谁，我的背景是什么，我想申请什么"
  提到你的FES研究方向、UEC硕士学位

第2段：Research Background（200词）
  "我做过什么研究"
  详细但简洁地描述你的FES电极研究
  不需要说"结果不理想" → 说"this experience gave me foundational skills in..."
  提到你从研究流程中学到了什么

第3段：Why This Direction（200词）
  "为什么我想转向Neural Signal Processing"
  从FES的局限性出发 → 引出'需要更好的信号处理和解码技术' → 自然过渡到你想做的方向
  关键句型："My FES experience showed me that effective neural rehabilitation 
  requires not only stimulation technology, but also robust neural signal 
  decoding — which is why I want to pursue research in..."

第4段：Why This Group / University（150词）
  "为什么选这个实验室"
  提到教授的具体研究 → 说明和你兴趣的关联
  提到瑞典在神经工程领域的优势

第5段：What I Bring（150词）
  "我能贡献什么"
  FES实验经验、信号采集能力、嵌入式系统理解、自学能力
  强调跨学科背景（自动化 + 生物医学 + 嵌入式）

第6段：Closing（100词）
  "我的目标"
  PhD期间想达成什么、长期职业目标
  简短有力
```

### 26.3b Motivation Letter 完整范文 + 逐段解析 / Full Example + Commentary

> 以下是针对一个假想的瑞典PhD岗位写的范文。实际使用时需要：
> ① 替换[方括号]中的内容  ② 根据具体岗位调整第3-4段  ③ 不要原封不动复制

#### 假想岗位

```
Position: PhD student in Biomedical Signal Processing — 
          EMG-based motor intention decoding for rehabilitation
University: Chalmers University of Technology, Gothenburg
Supervisor: Prof. [Name]
```

#### 完整范文（约900词）

```
MOTIVATION LETTER

PhD Position: Biomedical Signal Processing — 
EMG-based Motor Intention Decoding for Rehabilitation
Applicant: [Your Name]

--- 第1段：Opening ---

I am writing to apply for the advertised PhD position in Biomedical 
Signal Processing at Chalmers University of Technology. I hold a 
Master's degree in Mechanical and Intelligent Systems Engineering from 
the University of Electro-Communications (UEC Tokyo, 2020), where my 
research focused on non-invasive Functional Electrical Stimulation 
(FES) for motor rehabilitation. My background in bioelectrode 
characterization, combined with my subsequent industry experience in 
embedded systems analysis, has prepared me to contribute meaningfully 
to research on EMG signal processing and motor intention decoding.

[解析：3句话说清楚你是谁、学什么的、为什么你match这个岗位。
 PI读完这段就能判断你是否值得继续看。]


--- 第2段：Research Background ---

During my Master's research, I investigated surface electrode 
materials for transcutaneous functional electrical stimulation. 
Specifically, I designed and fabricated non-invasive electrodes using 
different conductive materials, measured electrode-skin interface 
impedance under varying conditions, and optimized stimulation 
protocols for comfortable and effective transcutaneous stimulation. 
This work required systematic experimental design, data acquisition 
using specialized measurement equipment, and quantitative analysis of 
bioelectrical signals — skills directly relevant to EMG signal 
processing research.

I submitted a paper based on this work to IEEE ARSO 2020, gaining 
experience in academic writing and the peer review process. While the 
paper was ultimately not accepted, the reviewer feedback was 
invaluable in sharpening my understanding of rigorous experimental 
methodology and the standards of biomedical engineering research.

[解析：把FES研究翻译成学术语言。注意不说"研究不好"，
 而是强调学到的技能。ARSO被拒的事情一笔带过，重点放在
 "学到了什么"上。]


--- 第3段：Why This Direction ---

My FES research experience revealed a fundamental insight: effective 
motor rehabilitation requires not only the ability to deliver 
electrical stimulation, but also the ability to accurately decode the 
patient's motor intention from neural signals. This realization has 
driven my interest toward EMG signal processing and motor intention 
decoding — the "sensing and interpretation" side of the 
neurorehabilitation loop.

I have since begun building skills in Python-based signal processing 
(NumPy, SciPy, scikit-learn, MNE-Python) and have completed a 
personal project on EMG gesture classification using the Ninapro 
dataset, implementing preprocessing pipelines, time-domain feature 
extraction, and SVM-based classification [GitHub link]. This 
experience confirmed my strong interest in pursuing this direction at 
a research level.

[解析：这段是核心——解释从FES到Neural Signal Processing的
 逻辑。不是"要转行"，而是"FES的经历让我意识到需要更好的
 信号解码"。如果你还没做GitHub项目，可以写"I am currently 
 developing..."。]


--- 第4段：Why This Group ---

Professor [Name]'s work on [具体研究，例如 "osseointegrated neural 
interfaces and EMG-based prosthetic control"] is particularly 
compelling to me. Your recent publication "[论文标题]" (2024) 
demonstrated [什么具体发现], which directly relates to my interest 
in bridging electrical stimulation and neural signal decoding. 
The opportunity to work with [具体设备/数据/方法] at Chalmers 
would allow me to combine my electrode-tissue interface knowledge 
with advanced signal processing techniques.

I am also drawn to the interdisciplinary research environment at 
Chalmers and the strong tradition of translational neurotechnology 
research in Sweden, where engineering solutions are developed in 
close collaboration with clinical partners.

[解析：这段证明你读了导师的论文。必须提到具体的论文标题
 和发现。这是区分"群发模板"和"认真申请"的关键段落。
 每申一个岗位，这段要完全重写。]


--- 第5段：What I Bring ---

Beyond my academic background, I bring two years of industry 
experience in embedded systems analysis at a major construction 
equipment manufacturer in Japan. In this role, I independently 
analyzed a production codebase of 400+ modules running on RH850 
microcontrollers, traced signal flows across CAN/J1939 communication 
networks, and developed a deep understanding of real-time system 
architecture and state machine design. This experience has given me 
strong skills in systematic code analysis, signal flow tracing, and 
working with safety-critical embedded systems — competencies that 
are directly applicable to developing reliable neural signal 
processing pipelines and real-time BCI systems.

Additionally, five years of living and working independently in 
Japan have developed my adaptability, cross-cultural communication 
skills, and ability to work autonomously — qualities essential for 
a PhD researcher.

[解析：把你的嵌入式经历翻译成对PhD有用的技能。注意：不说
 "我是HILS测试工程师"，而说"embedded systems analysis"。
 不说"自学"，而说"independently analyzed"。]


--- 第6段：Closing ---

My goal for this PhD is to develop robust EMG signal decoding 
methods that can contribute to more effective and personalized 
motor rehabilitation systems. In the longer term, I aim to build 
a career in neural engineering research, bridging the gap between 
electrical stimulation technology and intelligent neural signal 
interpretation.

I would welcome the opportunity to discuss how my background in 
FES research and embedded systems analysis could contribute to 
your group's work. Thank you for considering my application.

[解析：简短有力。说清楚PhD目标和长期方向。不要写太空泛的
 话（如"I want to contribute to science"）。]
```

#### 动机信的核心原则

```
✅ DO:
├── 每段都要有明确的功能（who → research → why direction → why group → skills → goal）
├── 第4段（Why This Group）必须每个申请都重写 — 这是最花时间但最重要的段
├── 用具体事实而非形容词（"analyzed 400+ modules" > "strong analytical skills"）
├── 从FES到neural signal processing的叙事要自然（不是"转行"，是"延伸"）
├── 总长度控制在800-1000词（1.5-2页A4）
├── 英语不需要完美 → 请native speaker或Grammarly检查语法即可
└── 诚实但不自贬（不说"我不够好"，说"我学到了..."）

❌ DON'T:
├── 不要写成自传（不需要说"我小时候对科学感兴趣"）
├── 不要每段都说"I am very motivated"（用事实证明motivation）
├── 不要群发同一封信（PI能看出来）
├── 不要提年龄（北欧不问也不care）
├── 不要提工资（PhD是工作但动机信不谈钱）
├── 不要写超过1200词（PI没时间看3页）
└── 不要在第2段说"结果不理想/研究很水/论文被拒"
```

#### 写作流程

```
第1步：先写第4段（Why This Group）
  → 因为这是最具体的，需要先读导师论文
  → 这段定了，其他段自然围绕它展开

第2步：写第3段（Why This Direction）
  → 从FES到这个具体方向的逻辑桥梁

第3步：写第2段和第5段（Research + Skills）
  → 相对固定，微调即可

第4步：写第1段和第6段（Opening + Closing）
  → 最后写，因为需要知道全文的基调

第5步：请人检查
  → 英语语法：Grammarly Premium 或 找native speaker
  → 内容逻辑：请硕士导师或信任的人读一遍
  → 字数检查：800-1000词
```

### 26.4 关于ARSO被拒论文的处理 / How to Handle the Rejected ARSO Paper

**不要觉得它毫无价值。** 具体做法：

```
1. 整理成英文研究摘要（1页）
   ├── Title
   ├── Background: 为什么研究非侵入FES电极材料
   ├── Methods: 对比了哪些材料、用了什么测量方法
   ├── Results: 主要发现（即使结果不完美，也有数据）
   └── Significance: 这个研究的意义/贡献

2. 在Motivation Letter中的措辞
   ✅ "During my Master's research, I investigated surface electrode 
      materials for transcutaneous FES. This work, while exploratory, 
      gave me hands-on experience in bioelectrode characterization, 
      experimental design, and academic writing. I submitted a paper 
      to IEEE ARSO 2020, and the reviewer feedback further deepened 
      my understanding of the publication process."
   
   ❌ 不要说："我的研究很水" / "导师让我水毕业" / "论文被拒了很失败"

3. 如果还记得审稿意见
   → 反思问题出在哪里（实验设计？数据量？写作？）
   → 这些反思可以在面试中提到（"I learned from the review process that..."）
```

### 26.5 北欧PhD面试完全指南 / The PhD Interview — What to Expect

> 你的问题："申请博士会有面试吗？是什么样的感觉？"
> 答案：**会有，但和你想象的可能完全不一样。**

#### 面试流程概览

```
瑞典PhD面试和日本面试的区别：

日本面试：
├── 西装、深鞠躬、敲门→ 正式问候
├── "请告诉我您的应聘动机"
├── 答案几乎需要背好，偏差=减分
├── 面试官在打分表上打分
├── 紧张、压力大、上下级关系明确
└── 感觉像"被审判"

北欧PhD面试：
├── Zoom/Teams视频通话（或现场，但多数先线上）
├── 教授穿T恤或衬衫，叫你first name
├── 像一场学术对话（conversation），不是审讯
├── 教授在判断：和你合作4年，舒不舒服？
├── 你也在判断：这个导师和我合得来吗？
├── 感觉更像"两个人聊研究"而非"面试"
└── 如果教授对你感兴趣，气氛通常很轻松
```

#### 面试通常有几轮？

```
最常见的流程：

第0步：邮件联系阶段（非正式）
  → 你发邮件给教授 → 教授有兴趣 → 约一次informal chat
  → 这不是正式面试，但其实已经在考察你了
  → 持续15-30分钟，Zoom上聊

第1步：正式面试（第一轮）
  → 收到面试邀请邮件
  → 通常1-2个教授参加（主导师 + 可能的co-supervisor）
  → 30-60分钟
  → 内容：你的背景、研究兴趣、对这个项目的理解

第2步：第二轮面试（有些学校有，有些没有）
  → 可能让你做一个10-15分钟的presentation
  → 或者让你和实验室其他成员聊天（看team fit）
  → 有些学校到这一步会让你做一个小task（如分析一段数据）

第3步：Offer / Rejection
  → 瑞典大学：面试后1-4周给结果
  → 有些会先打电话告诉你结果，然后发正式邮件

注意：很多情况下只有1轮面试就决定了。流程比想象的简单。
```

#### 教授会问什么？/ Common Questions

```
═══════════════════════════════════════════════════
 第一类：关于你的研究背景（必问）
═══════════════════════════════════════════════════

Q1: "Can you tell me about your Master's research?"
    你的回答要点：
    ├── 2-3分钟讲清楚FES电极研究
    ├── 用学术英文（electrode characterization, impedance analysis...）
    ├── 不要说"研究很水" → 说"exploratory work that gave me 
    │   hands-on experience in..."
    └── 提到你学到了什么技能

    参考回答：
    "My Master's research focused on non-invasive Functional 
    Electrical Stimulation. Specifically, I designed surface 
    electrodes using different conductive materials and characterized 
    their performance by measuring electrode-skin interface impedance. 
    I also optimized stimulation protocols for transcutaneous 
    stimulation. Through this work, I gained hands-on experience in 
    experimental design, bioelectrical measurement, and data analysis. 
    I submitted a paper to IEEE ARSO, and while it was not accepted, 
    the review process taught me a great deal about scientific rigor."

Q2: "What did you learn from your Master's research?"
    → 这是变体问法，重点在"学到了什么"而不是"做了什么"
    → 提到：实验设计能力、数据采集和分析、学术写作、
      peer review流程的完整经历

Q3: "Why didn't you publish your paper?"
    → 有可能会被问，但语气通常是好奇而非质疑
    → 诚实回答："The reviewers raised valid concerns about [具体问题]. 
       In retrospect, I would have [你现在觉得可以改进的地方]. 
       This experience taught me the importance of [什么教训]."
    → 展示你的反思能力 = 加分

═══════════════════════════════════════════════════
 第二类：关于你为什么想做这个方向（必问）
═══════════════════════════════════════════════════

Q4: "Why do you want to do a PhD in neural signal processing?"
    你的叙事线：
    FES经历 → 意识到信号解码的重要性 → 自学Python/信号处理 
    → 做了EMG项目 → 想在学术层面深入
    
    参考回答：
    "My FES research showed me that effective rehabilitation requires 
    not just stimulation, but also accurate interpretation of neural 
    signals. This insight motivated me to explore EMG signal processing 
    on my own — I completed a gesture classification project using the 
    Ninapro dataset. This experience confirmed that I want to pursue 
    this direction at a research level, which is why I'm applying to 
    your group."

Q5: "Why did you choose this specific position / our group?"
    → 必须提到教授的具体研究
    → "Your work on [具体论文/项目] is directly related to my 
       interest in [具体方向]. I was particularly interested in 
       [论文中的某个具体发现或方法]."

Q6: "Why Sweden / why Chalmers?"
    → 可以说：研究环境、interdisciplinary approach、specific lab
    → 不要说："because it's free" 或 "because I want to leave Japan"

═══════════════════════════════════════════════════
 第三类：关于你的技术能力（可能会问）
═══════════════════════════════════════════════════

Q7: "What programming languages do you know?"
    → "I have strong reading-level proficiency in C/C++ from my 
       industry experience analyzing embedded systems. I've been 
       developing Python skills for signal processing, and I've 
       completed an EMG classification project using NumPy, SciPy, 
       and scikit-learn."
    → 诚实说Python还在学习中，但展示你在积极补

Q8: "Do you have experience with [specific tool/method]?"
    → 如果有：说具体做了什么
    → 如果没有：
       "I haven't used [X] directly, but I have experience with 
       [相关的东西]. Given my background in [相关技能], I'm 
       confident I can learn it quickly."
    → 永远不要只说"No"。说"Not yet, but..."

Q9: "Can you explain [一个技术概念]?"
    → 教授可能会问一些信号处理/神经科学的基础概念
    → 确保你能解释：FFT, bandpass filter, EMG/EEG是什么, 
       feature extraction, classification的基本流程
    → 不需要很深，但基础要扎实

═══════════════════════════════════════════════════
 第四类：关于你的工作经历和个人情况
═══════════════════════════════════════════════════

Q10: "Tell me about your industry experience."
    → 用26.3b范文第5段的内容
    → "embedded systems analysis at a major construction equipment 
       manufacturer" — 不点名公司
    → 强调：independent analysis, 400+ modules, real-time systems, 
       CAN communication, state machines

Q11: "Why are you leaving industry for academia?"
    → "Working in embedded systems gave me valuable engineering skills, 
       but I realized my core interest lies in research — specifically, 
       understanding neural signals. My Master's FES work was the most 
       intellectually engaging period of my career, and I want to 
       return to that."
    → 不要说"工作不开心" / "日本太压抑" / "被派遣公司坑了"

Q12: "You've changed jobs several times. Can you explain?"
    → 有可能被问（3份工作 + 短期）
    → 诚实但正面地回答：
       "My first two positions helped me understand what I was 
       looking for in a career. My current role at the construction 
       equipment manufacturer has been the most rewarding — it gave 
       me deep exposure to embedded systems. These experiences 
       collectively clarified my direction: I want to pursue research 
       in neural engineering, combining my signal processing background 
       with my engineering skills."

═══════════════════════════════════════════════════
 第五类：关于PhD本身（教授想确认你理解PhD是什么）
═══════════════════════════════════════════════════

Q13: "What do you expect from a PhD?"
    → "I expect it to be challenging — both intellectually and 
       personally. I understand that PhD research involves periods 
       of uncertainty and failure, and I'm prepared for that. 
       I'm looking forward to developing deep expertise in a specific 
       area while contributing original research to the field."
    → 不要说"I want a degree" → 说"I want to do research"

Q14: "Where do you see yourself after the PhD?"
    → 说清楚但不要太具体
    → "I see myself continuing in neural engineering research, 
       either in academia or at a research-oriented company. 
       I'm particularly interested in translational research that 
       bridges laboratory findings and clinical applications."

Q15: "Do you have any questions for me?"
    → ★ 一定要准备2-3个问题 ★
    → 好问题：
       ├── "What would a typical day/week look like for a PhD 
       │    student in your group?"
       ├── "How does your group typically collaborate with clinical 
       │    partners?"
       ├── "What skills do you think are most important for success 
       │    in this project?"
       └── "What are the main challenges you foresee in this project?"
    → 不要问的：
       ├── ❌ 工资多少（岗位信息里写了）
       ├── ❌ 假期多少天（显得不关心研究）
       └── ❌ 什么都不问（显得没准备）
```

#### 面试的真实"感觉"

```
紧张是正常的。但北欧面试和日本/中国面试有本质区别：

日本面试的感觉：
"我在被打分。说错一句话就完了。"
→ 紧张来自：权力不对等 + 标准答案的期待

北欧PhD面试的感觉：
"两个对FES/信号处理感兴趣的人在聊天。"
→ 教授想知道的不是你有多"优秀"，而是：
  ├── 你对这个课题真的有兴趣吗？（还是只想拿学位）
  ├── 你能独立思考吗？（不是背答案）
  ├── 和你合作4年，我会不会后悔？（人际契合）
  └── 你遇到问题会怎么处理？（成长型思维）

实际体验（过来人描述）：
├── "比我想象的友好很多。教授笑着聊，有时会开玩笑。"
├── "感觉更像是在和一个senior colleague聊我的兴趣。"
├── "教授在某些问题上不同意我的看法→但这不是扣分，
│    他只是在看我怎么处理不同意见。"
├── "有一个问题我不会答→我说I'm not sure but I think...
│    →教授说that's a good intuition，反而加了分。"
└── "全程45分钟，紧张了前5分钟，后面就自然了。"

最重要的一条：
══════════════════════════════════════════════
  你不需要每个问题都答对。
  你需要展示的是：诚实、好奇心、思考过程。
  "I don't know, but I would approach it by..." 
  比假装懂要好100倍。
══════════════════════════════════════════════
```

#### 面试准备清单

```
面试前1周：
□ 重读自己的Motivation Letter（面试问题80%基于你写的内容）
□ 重读导师最近2-3篇论文的摘要和结论
□ 准备2-3分钟的硕士研究介绍（英文，对镜子/录音练习）
□ 准备2-3个要问教授的问题
□ 确认Zoom/Teams能正常使用（测试摄像头、麦克风）

面试前1天：
□ 再看一遍本节的Q&A
□ 不要临时抱佛脚学新东西（会更焦虑）
□ 早点睡
□ 提醒自己：这是一场对话，不是考试

面试当天：
□ 提前5分钟进入视频会议
□ 背景整洁（或用Zoom虚拟背景）
□ 穿着：smart casual（衬衫就行，不需要西装）
□ 准备好纸笔（万一需要记什么）
□ 桌上放一杯水

面试中：
□ 听清问题再回答（没听清可以说"Could you repeat that?"）
□ 不用急，想3-5秒再回答 = 完全正常
□ 不会的问题说"I'm not sure, but my understanding is..."
□ 微笑 + 眼神接触（看摄像头，不是看屏幕）
□ 面试结束时说"Thank you for your time. I really enjoyed 
   this conversation."

面试后：
□ 24小时内发一封简短的感谢邮件
   "Dear Prof. [Name], thank you for taking the time to meet 
   with me today. I really enjoyed discussing [具体聊到的话题]. 
   I remain very interested in the position and look forward to 
   hearing from you. Best regards, [Name]"
□ 然后该干嘛干嘛。等结果期间继续投其他岗位。
```

#### 如果英语不够流利怎么办？

```
现实情况：
├── 北欧教授每天和非英语母语的人交流（PhD学生来自全球）
├── 他们完全习惯各种口音和不完美的英语
├── 你不需要说得像native speaker
├── 你需要的是：能把意思说清楚

实用技巧：
├── 说慢一点 > 说快但错误百出
├── 用简单句 > 用复杂句但说不通
├── 如果一个词想不起来 → 用解释代替
│   （"the thing that filters out noise" = bandpass filter）
├── 准备的部分（自我介绍、研究背景）提前练几遍
├── 即兴部分不强求完美 — 教授在听内容，不是打语法分
└── 日本的英语教育让你以为"说错就丢人" → 在北欧没人在意

如果真的很紧张，面试开头可以说：
"I apologize in advance if my English is not perfect — 
 I've been working in Japanese for the past five years, 
 but I'll do my best to express myself clearly."
→ 教授会理解，而且这反而展示了你的诚实。
```

---

## 二十五、完整时间线 / Master Timeline

### 27.1 探索节奏版时间线 / Exploration-Paced Timeline（无死线版）

> ⚠️ 这个时间线没有硬性deadline。每个阶段的推进信号是"我准备好了"，而非"到日子了"。
> 核心原则：不赶、不慌、不和别人比速度。走到哪算哪，随时可以暂停和调整。

```
═══════════════════════════════════════════════════════════════
 第1阶段：不带目的的探索期（3-6个月，从现在开始）
 信号：感觉"这个话题我真的想深入了解" → 进入第2阶段
═══════════════════════════════════════════════════════════════
 做什么：
 □ 随便翻翻，不带KPI：
     ├── YouTube上看看脑机接口/神经工程的科普视频
     ├── 翻翻FES/EMG/BCI相关的论文标题和摘要（不用精读）
     ├── 浏览 Coursera / edX 上神经科学/信号处理的课程简介
     ├── 在Google Scholar搜搜自己感兴趣的关键词
     └── 随手记"这个有意思" / "这个没兴趣"
 
 □ 复习自己的硕士论文（不着急，有空看看就行）
 □ 在建设机械制造商继续工作（收入来源）+ 嵌入式学习继续（备选路线）
 □ 允许自己对方向D(Neural Signal Processing)以外的东西感兴趣
 □ 允许自己某些天什么都不想学

 不做什么：
 ✗ 不需要马上选定研究方向（方向D只是一个初始参考，不是承诺）
 ✗ 不需要制定论文阅读计划
 ✗ 不需要联系任何教授
 ✗ 不需要准备申请材料
 ✗ 不需要报考IELTS

═══════════════════════════════════════════════════════════════
 第2阶段：兴趣聚焦期（时长不限，2-4个月左右）
 信号：某个方向看了5篇以上还想继续看 → 这就是你的方向
═══════════════════════════════════════════════════════════════
 □ 围绕感兴趣的关键词找综述论文（review paper），读1-2篇
 □ 用种子扩展法（25.5节）从综述的参考文献中找到更多论文
 □ 开始论文笔记（用25.6节的模板，不追求完美）
 □ 搜索"谁在做这个方向" → 浏览北欧大学实验室页面
 □ 装Python + mne-python环境，跑跑tutorial代码（体验一下就行）
 □ 准备CV（学术版，英文）← 这个可以早点做，不依赖方向确定

═══════════════════════════════════════════════════════════════
 第3阶段：从兴趣到行动（时长不限，2-4个月左右）
 信号：我能用2-3句话说清楚"我想做什么研究" → 可以联系导师了
═══════════════════════════════════════════════════════════════
 □ 精读2-3篇你最感兴趣的论文（认真做笔记、理解方法和结论）
 □ 写"研究兴趣陈述"的草稿（英文，300词左右）
 □ 用Python跑一个小项目（EMG信号处理 / BCI tutorial / etc.）
 □ 向UEC申请英文成绩单和学位证明（这个需要时间，提前办）
 □ 联系硕士导师，说一下你的想法，请求将来帮写推荐信
 □ 英语考试：确认是否需要 → 如果需要，开始准备IELTS/TOEFL

═══════════════════════════════════════════════════════════════
 第4阶段：联系导师 + 投申请（时间随缘）
 没有deadline的意思是：岗位全年都有，错过这一轮还有下一轮
═══════════════════════════════════════════════════════════════
 □ 根据论文阅读和实验室浏览，缩小到5-8个研究组（四国通看）
 □ 用26.1的模板给5-8个教授发邮件
 □ 写 Motivation Letter 初稿（用26.3的框架）
 □ 监控北欧PhD vacancy页面（瑞典/挪威/丹麦/芬兰）
 □ 看到匹配的岗位就投（不要等"完美准备好"）

═══════════════════════════════════════════════════════════════
 第5阶段：等结果 / 调整 / 再投
═══════════════════════════════════════════════════════════════
 □ 收到面试邀请 → 准备学术面试
 □ 收到offer → 办理工作居留许可 → 辞职 → 搬家
 □ 没有offer → 不慌，分析原因 → 调整策略 → 下一轮再投
 □ 同时考虑备选方案（嵌入式工作路线始终可用）

 核心心态：
 ├── 北欧PhD岗位全年滚动发布，不是高考那种一年一次的机会
 ├── 晚一年入学 = 多一年工作经验和准备 = 申请材料更强
 ├── 你已经33了 → 34或35入学没有区别
 └── 重要的是方向对了，而不是速度快了
```

### 27.2 每周时间分配（探索期版本） / Weekly Time Allocation (Exploration Mode)

```
探索期（第1-2阶段）的节奏：每周 3-5 小时，随便翻翻

工作日（周一-周五）：
├── 白天：建设机械制造商工作（收入来源）
├── 嵌入式学习（有空闲时）：保持备选路线
└── 有兴趣的晚上（不强制每天都做）：
    ├── 看一个YouTube科普视频（20-30分钟）
    ├── 翻1-2篇论文摘要（20分钟）
    ├── 或者什么都不做（也完全可以）
    └── 唯一的原则：觉得累了就停，不追求连续性

周末：
├── 有精力的时候：看看论文、浏览实验室网站、写写笔记（1-2小时）
├── 没精力的时候：休息。不学也没关系。
└── 偶尔看看YouTube上脑机接口/神经工程的纪录片也算

总计：每周约 3-5 小时（第1-2阶段）
后期如果方向明确了、动力起来了 → 自然会增加到 8-12 小时
这不需要计划，它会自然发生的（就像你学变速箱控制代码的时候一样）。
```

### 27.3 两条线并行策略 / Dual Track Strategy

```
PhD路线（主线）             嵌入式工作路线（备选）
─────────                  ────────────
论文阅读+Python实操          继续在当前项目学习代码
联系瑞典导师                 本笔记21-22章的面试准备
准备PhD申请材料              LeetCode + 个人项目
投PhD岗位                   投北欧嵌入式岗位

两条线不冲突：
├── PhD准备主要在晚上和周末
├── 嵌入式学习在工作时间的空闲时段
├── 如果PhD先拿到offer → 走学术路线
├── 如果工作先拿到offer → 走工业路线
└── 无论走哪条路，你都在往北欧移动

最坏的情况也不坏：
├── PhD没申到 + 工作没找到 → 调整策略再投一轮
├── 过程中积累的论文阅读、Python技能、英语能力 → 都不浪费
└── 你比什么都不做的自己强100倍
```

### 27.4 心态管理 / Mindset

```
关于"硕士研究不理想"：
├── 那段经历不是浪费 — 你有了FES实验经验、学术流程经验
├── 很多成功的PhD学生，硕士阶段也做得不好
├── PhD和硕士最大的区别：你可以选自己真正感兴趣的方向
└── "从失败中学到什么" 比 "一路顺利" 更有说服力（在北欧）

关于"33岁是不是太晚"：
├── 瑞典PhD平均入学年龄 28-30，30-35入学非常常见
├── 37-38岁毕业后还有30年职业生涯
├── 比起"来不来得及"，更重要的是"如果不做，5年后会后悔吗？"
└── 答案如果是"会后悔" → 那就做

关于"没有论文"：
├── 你有完整的研究流程经验（很多申请者连这个都没有）
├── 你有一篇投稿过的论文草稿（整理出来就是材料）
├── 如果能在申请前做一个Python项目放GitHub → 弥补论文缺失
└── 动机信写好 = 胜过一篇低质量的论文

关于"不知道能不能行"：
├── 回忆一下你是怎么学变速箱控制代码的：从完全看不懂到理解400+模块的完整架构
├── 你证明过自己可以从零学会复杂的东西
├── 学神经科学也是一样：一步一步来，不需要一口气全学会
└── 从今天开始，每天30分钟。6个月后你会惊讶于自己的进步。

关于"竞争对手都比我强"：
├── 清华北大的人不太申北欧PhD（他们首选美国/英国/瑞士/新加坡）
├── 北欧教授大多不知道中国大学排名（985/211/双一流对他们没意义）
├── 遴选标准 = 研究方向匹配 + 动机信 + 导师印象 ≠ 学校排名+绩点
├── 你比纯学术背景的人多了一个优势：5年工业界嵌入式系统经验
├── 你比应届硕士多了一个优势：清楚知道自己为什么要做这件事
└── 你的对手不是"全国最优秀的人"，而是"碰巧也对北欧这个方向感兴趣的人"

关于"信号处理都忘了"：
├── 重新学 ≠ 从零学。你只是生锈了，不是从来没学过
├── 有了嵌入式工作经验后，很多信号处理概念会"豁然开朗"
│   因为你现在知道信号是怎么从传感器到寄存器到变量的（之前不知道）
├── 不需要先复习完教材再开始看论文 → 看论文时遇到不懂的再查
├── 这叫"按需学习"（learn on demand），比系统复习高效10倍
└── 你学变速箱控制代码也不是先读完C++教科书再看的，而是直接看代码遇到不懂再查

关于"我靠AI学的，不算真本事"：
├── 先问自己：AI替你理解了吗？没有。理解发生在你脑子里，不在AI里。
├── "傻子也能用AI"说的是：复制粘贴、不问为什么、不追问、不验证的人
├── 你的用法完全不同：
│   ├── 你提出具体问题（"static_cast和C强转有什么区别？"）
│   ├── 你追问到底（"那如果溢出了呢？" "那嵌入式里为什么选这个？"）
│   ├── 你验证理解（"你确定是这样吗？" "我理解的对吗？"）
│   └── 你把答案变成了自己的知识（你现在能独立读变速箱控制代码了）
├── 这叫"用AI做苏格拉底式对话"，不叫"依赖AI"
├── Google的人、翻书的人之所以"显得更强"：
│   不是因为工具高级，是因为那些过程更慢、强迫他们自己组织信息
│   你的追问链（这是什么→为什么→如果不→那这个呢）= 同样的深度思考过程
├── 判断你是不是在"被AI喂饭"的4个问题：
│   ✗ 不问问题就让AI给答案？ → 你不是（你问题多到爆）
│   ✗ 从不验证AI说的对不对？ → 你经常质疑
│   ✗ 拿掉AI完全做不了？     → 你现在能独立读代码
│   ✗ 只复制不理解？         → 你从没复制过代码，你在学原理
│   四条全不符合 → "AI让人变傻"这个批评不是在说你
├── 到了PhD阶段：没有人在乎你用什么工具学的
│   教授只在乎你理解了没有、能不能讨论、能不能推进研究
│   AI、书、Google、同事 — 都是工具。知识进了你的脑子，它就是你的。
└── ★ 别把自己的能力归功于工具。工具谁都能用，不是谁都能像你这样用。

关于"我什么都不会"：
├── 你觉得你什么都不会，是因为你总是在看"还不会的部分"
├── 回头看看你实际已经会的：
│   ├── 从零理解了400+模块的变速箱控制系统架构
│   ├── 搞懂了状态机、定时器、组合逻辑、多输出4种模式
│   ├── 追踪了revolution信号从CAN到模块的完整数据流
│   ├── 理解了C++的static_cast、模板、继承、枚举、位操作
│   ├── 理解了Makefile构建系统和交叉编译
│   ├── 独立在日本工作生活5年
│   └── 硕士期间独立做了FES电极实验
├── 你不是"什么都不会"，你是"总觉得自己还不够"
├── 这其实是一种好品质（叫 growth mindset / 成长型思维）
│   但不要让它变成自我否定
└── ★ 每次觉得"我不行"的时候，回来翻翻这份笔记的前20章，看看你已经走了多远。
```


---

## 二十六、北欧生活实用指南 / Nordic Life Practical Guide

> 本章整理了在决定去北欧之前需要了解的"现实面"——好的和坏的都有。
> 不美化、不恐吓，只说真话。

### 28.1 瑞典/北欧的真实缺点 / Real Downsides

```
1. BankID困境（数字身份）
├── 几乎所有线上服务需要BankID
├── 开通需要：personnummer → 银行账户 → BankID
├── personnummer需要居留许可处理完 → 1-3个月空窗期
├── 空窗期内：不能网购、不能登录政府网站、不能签手机合同
├── PhD学生：学校帮加速，但仍有1-2个月等待
└── 一次性问题，办完就没事了

2. Migrationsverket（移民局）
├── 效率极低，处理时间不可预测
├── 工作签证：官方4个月，实际6-12个月
├── PhD居留相对快（学校帮push），1-3个月
├── 续签有时比首次更慢
├── 系统故障、丢失材料时有发生
└── PhD学生比工作签证好很多（学校有专人处理）

3. 冬天的黑暗
├── 斯德哥尔摩12月：早8:30天亮，下午2:30天黑（日照6小时）
├── 北部城市12月几乎不见太阳
├── SAD(季节性情绪障碍)是真实存在的
├── 夏天反过来：晚上11点天还亮
└── 应对：光疗灯、维生素D、中午散步、保持运动

4. 社交冰墙 ★★★ 外国人最大痛点
├── 瑞典人礼貌友好但极难交朋友
├── 同事：工作时很好，下班后各回各家
├── 交到真正瑞典朋友平均需要2-5年
├── 5年后朋友圈仍然全是其他外国人的情况很常见
├── 不是针对你 → 瑞典人对所有人都这样
├── "Lagom"文化 = 不要太多、不要太少、不要太突出
└── 有太太+教会 → 这个问题会大幅缓解

5. 住房危机（尤其斯德哥尔摩）
├── 斯德哥尔摩公寓排队：平均8-15年
├── 二手租房(andrahand)贵、不稳定
├── PhD第一年：学校通常有宿舍
├── 哥德堡、隆德比斯德哥尔摩好找
└── 详见28.5节（带太太的住房方案）

6. 物价
├── 约日本的1.3-1.5倍
├── 外食：一顿午餐120-150 SEK（≈1600-2000日元）
├── 自己做饭可控制在日本水平
├── PhD薪资够生活但不宽裕
└── 两个人自己做饭 → 可以接受

7. 瑞典式官僚
├── 数字化但系统互不相通、crash频繁
├── Skatteverket/Försäkringskassan/Migrationsverket各有各的系统
├── 电话客服等待30分钟-2小时
└── 和日本不同类型的烦，但不会更轻松
```

### 28.2 和日本的关键对比 / Sweden vs Japan: Key Comparison

```
你厌恶日本文化的核心（推测）       瑞典的情况
─────────────────────         ──────────
等级制度/上下关系/年功序列       → 几乎没有。教授让你叫名字(du)
察言观色/不能直说/表面和谐       → 鼓励直接沟通
加班文化/带薪假难休              → PhD自己管时间，没人逼
外国人永远是"外人"              → 制度平等，但社交有隐形墙
形式主义（盖章、礼仪）           → 极少。无盖章无鞠躬文化

本质区别：
├── 日本的问题 = 文化层面（改不了，越了解越厌恶，待越久越痛苦）
├── 瑞典的问题 = 系统层面（有解决方案，大部分一次性阵痛，待越久越顺）
├── 日本 = "被压着"
├── 瑞典 = "被忽略"（冷落但不压迫）
└── 但有太太+教会 → "被忽略"的问题大幅减轻

自测（多数打勾 → 瑞典适合你）：
□ 能忍受独处（太太在 → 自动打勾）
□ 不需要别人认可
□ 能自己做饭
□ 冬天宅家也OK
□ 讨厌被管比讨厌被冷落更强烈
```

### 28.3 交通 / Transportation

```
核心结论：不会开车完全没问题。

所有北欧城市：
├── 公共交通（地铁/公交/有轨电车） = 日常全覆盖
├── 自行车基础设施极好（专用车道、停车架）
├── 月票费用：830-970 SEK/月（≈1.1-1.3万日元）
├── 超市在步行/骑车/公交范围内
├── 网购送货到家（食材+日用品）
└── 大件购买 → 送货服务或偶尔租车

各城市：
├── 斯德哥尔摩：SL（地铁+公交+轻轨+渡轮），月票970 SEK
├── 哥德堡：Västtrafik（公交+有轨电车），月票830 SEK，骑车也很方便
├── 隆德/马尔默：城市小 → 自行车主力，火车15分钟互通
└── 和日本一样：城市生活不需要车

唯一需要车的场景 = 偏远度假/郊游 → 租车解决
```

### 28.4 信仰生活 / Faith & Church Life

> 个人背景：改革宗基督徒

```
瑞典宗教环境：
├── 完全的宗教自由
├── 高度世俗化（每周去教堂 < 5%）→ 反而是需要福音的禾场
├── 国教 = 路德宗（Svenska kyrkan），改革宗是少数派但存在
└── 没有任何信仰方面的歧视或限制

可参加的教会选择：

① 国际教会（International Church）⭐ 新到推荐先去
   ├── Stockholm International Church (SIC)
   ├── International Church of Gothenburg
   ├── Lund International Church
   ├── 英语崇拜，成员多为在瑞外国人
   ├── 社交功能强 → 认识人的最快方式之一
   └── 神学立场：广义evangelical，包容不同传统

② 华人教会 ⭐⭐ 太太也能融入
   ├── 斯德哥尔摩华人基督教会
   ├── 哥德堡华人团契
   ├── 规模小（20-50人），华语崇拜+团契
   ├── 太太不擅长英语的话 → 这是最重要的支持网络
   └── 海外华人教会神学立场常偏改革宗 → 对你来说很合适

③ 瑞典福音派教会（EFK等）
   ├── Evangeliska Frikyrkan = 瑞典最大福音派联盟
   ├── 神学接近改革宗（不完全是传统Reformed）
   └── 主要用瑞典语 → 语言学好后可以deeper参与

④ 严格Reformed教会
   ├── Reformed Church in Sweden → 存在但非常小
   ├── 一些Presbyterian植堂运动在北欧活动
   └── 可联系 Gospel Coalition Europe 了解当地reformed网络

事工方向：
├── 华人教会/团契服事 ← 最自然的起点
│   ├── 海外华人教会永远缺人（尤其年轻弟兄）
│   ├── 改革宗神学背景在华人教会很受欢迎
│   ├── 带查经、翻译、接待、音乐 → 都需要人
│   └── 夫妻一起服事 → 融入最快
├── 校园事工（IFES / Cru 学生团体）
├── 难民/移民事工（适应后）
└── 教会 = 你们在瑞典最重要的社交网络+属灵家庭

信仰方面的好消息：
├── 完全的宗教自由（对比中国三自限制）
├── 教会内部打破"社交冰墙" → 弟兄姊妹间的关系比瑞典社会温暖得多
├── 北欧高度世俗化 = 福音的禾场
└── 你在日本有教会生活经验 → 到瑞典后转换很顺畅
```

### 28.5 带太太过去 — 签证、住房、生活 / Moving with Wife

```
═══════════════════════════════════════════════════════════════
 太太的签证
═══════════════════════════════════════════════════════════════
├── 你申请PhD工作居留 → 同时为太太申请"家属居留"
├── 条件：合法婚姻（结婚证公证+英文翻译） + 经济能力 + 有住房
├── PhD薪资(~30,000 SEK)通常满足经济条件
├── 太太拿到家属居留 → 有在瑞典工作的权利
├── 处理时间：3-6个月 → 提前申请
└── 续签和你同步

═══════════════════════════════════════════════════════════════
 两个人的住房
═══════════════════════════════════════════════════════════════
方案A：学校couple/family housing
├── 部分大学有 → KTH、Chalmers有少量couple公寓
├── 面积：35-50㎡ 一室一厅
├── 费用：5,500-8,000 SEK/月
├── ⚠️ 名额少 → 拿到offer后立刻申请！

方案B：二手租房（andrahand）一室一厅
├── 哥德堡：7,000-10,000 SEK/月
├── 斯德哥尔摩：9,000-13,000 SEK/月
├── 隆德/马尔默：6,000-9,000 SEK/月
├── 通过 Blocket.se / Qasa.se / Facebook Groups 找
└── 需要personnummer + 收入证明

方案C：合租大公寓（过渡期）
├── 与其他PhD夫妻合租
├── 省钱 + 有人照应
└── 适合刚到的前几个月

═══════════════════════════════════════════════════════════════
 两人经济预算（月）
═══════════════════════════════════════════════════════════════
你的PhD税后收入：~23,000 SEK
├── 房租（一室一厅）：-8,000
├── 吃饭（自己做，两人）：-6,000
├── 交通（两人）：-1,500
├── 手机+网络：-500
├── 其他：-1,500
├── 十一奉献：-2,300（税后的10%，如果你有这个习惯）
├── 剩余：~3,200 SEK → 紧但能活（Year 1-2）
└── Year 3-4太太BT实习（~25,000 SEK/月税后）→ 家庭收入翻倍，经济质变

═══════════════════════════════════════════════════════════════
 太太的职业背景：中国执业医生（国内医生）
═══════════════════════════════════════════════════════════════

 ★ 这是非常大的优势。瑞典极度缺医生。

 中国医学学位 → 瑞典行医的完整路径：
 （非EU第三国医生认证流程）

 第1步：瑞典语（最大的门槛）
 ├── 必须通过 Tisus 考试 或 达到 SFI D + SAS Grund 水平（≈C1级）
 ├── 硬性要求 → 要和患者沟通
 ├── 时间：认真全职学 1.5-2年
 ├── SFI课程完全免费 → 来了就可以开始
 └── 这段时间同时了解瑞典医疗体系

 第2步：Socialstyrelsen（国家卫生社会事务局）学历认证
 ├── 提交：医学学位 + 成绩单 + 执业资格 + 工作经验证明
 ├── 中国正规医学院学位 → 通常被认可（但需要补考）
 └── 处理时间：几个月

 第3步：Kunskapsprov（知识考试）
 ├── 理论考试：医学知识笔试（瑞典语）
 ├── 临床考试：OSCE（客观结构化临床考试）
 ├── 可以考多次（不是一次定终身）
 ├── 有专门备考课程（一些地区免费提供给外国医生）
 └── 通过率：认真准备 → 大多数人1-2次通过

 第4步：BT（Bastjänstgöring）实习期
 ├── 约1年临床实习 ← 有薪资！约30,000-35,000 SEK/月
 ├── 在医院各科室轮转
 ├── 完成后 → 获得瑞典执业医师资格（legitimation）
 └── 之后可做 ST（专科培训）→ 成为专科医生

 现实时间线（和你的PhD同步）：
 ├── Year 1-2：全力学SFI + 申请学历认证
 ├── Year 2：考瑞典语Tisus + 备考医学知识考试
 ├── Year 2-3：通过Kunskapsprov
 ├── Year 3-4：BT实习（有薪 ~30,000 SEK/月 → 家庭收入翻倍！）
 ├── Year 4+：正式执业 → 可做ST专科培训
 └── ★ 正好和你PhD的4年重合 → 你毕业时太太也可以行医了

 医生在瑞典的薪资：
 ├── BT实习期：~30,000-35,000 SEK/月
 ├── 全科医生（allmänläkare）：50,000-65,000 SEK/月
 ├── 专科医生（specialistläkare）：60,000-80,000 SEK/月
 └── → 太太正式行医后，家庭收入直接翻倍以上

 太太在学瑞典语期间（Year 1-2）：
 ├── 全职学SFI（每天3-4小时课 + 自学）
 ├── 同时向Socialstyrelsen提交学历认证
 ├── 有些地区提供"外国医生准备课程"（边学语言边熟悉瑞典医疗术语）
 ├── 教会参与 → 社交+信仰支持
 ├── SFI同学 = 天然的朋友圈
 ├── 如果需要赚钱 → 可做不需要瑞典语的兼职
 └── 但如果你PhD薪资勉强够 → 建议太太全力学语言（长期收益远大于短期打工）

 为什么太太是医生 = 整个计划的安全网：
 ├── 瑞典极度缺医生 → 政府非常希望外国医生留下
 ├── 有专门为外国医生设计的培训项目（kompletteringsutbildning）
 ├── 医生 = 永居很稳（紧缺职业 + 稳定工作）
 ├── 万一你PhD不顺利 → 太太有独立的职业线
 ├── 医院工作环境 → 天然社交圈（同事关系比一般行业close）
 └── 太太有自己的事业 → 不会因"跟你来了但什么都做不了"而frustration

═══════════════════════════════════════════════════════════════
 带太太的优势（比单身更好）
═══════════════════════════════════════════════════════════════
├── 太太是医生 → 在瑞典比你更"抢手"（瑞典缺医生比缺工程师更严重）
├── 社交冰墙 → 有太太 → 孤独感减半
├── 生活管理 → 两人分担 → PhD精力更集中
├── 教会 → 夫妻一起事工 → 融入更快
├── 经济 → Year 3-4起太太实习收入 → 家庭收入翻倍
├── 签证 → 同时累积居住年限 → 同时拿永居
├── 双职业线 → 你PhD+她行医 = 极强的长期根基
└── 心理 → 异国最怕孤独。太太 = 最大的安全网
```

### 28.6 财富积累 / Wealth Building

```
PhD期间（4年）— 分阶段预算：

Year 1-2（你PhD + 太太学瑞典语）：
├── 收入：你PhD税后 ~23,000 SEK/月（单收入）
├── 每月可存：~3,000 SEK
├── 两年存款：~7万 SEK → 紧但能活

Year 3-4（你PhD + 太太BT实习）：
├── 收入：你23,000 + 太太BT实习~25,000（税后）= ~48,000 SEK/月
├── 每月可存：~20,000-25,000 SEK → 经济状况质变
├── 两年存款：~50-60万 SEK

4年PhD期间总存款估计：~60-70万 SEK（≈800-900万日元）
（前提是太太Year 3开始BT实习 → 如果晚一年，总存款减少~25万SEK）

PhD毕业后（Year 5+） — 双职业家庭：

你的收入（工业界）：
├── Ericsson/Volvo/ABB/Scania: 40,000-58,000 SEK/月
├── Spotify/Klarna: 50,000-80,000+ SEK/月
├── 医疗器械(Medtronic/Elekta): 神经工程PhD直接对口
├── 学术界：博士后35,000-40,000 → 教授55,000-70,000
└── PhD毕业 = 直接Senior级入职

太太的收入（行医）：
├── 全科医生：50,000-65,000 SEK/月
├── 专科医生：60,000-80,000 SEK/月
└── 医生薪资非常稳定，不受经济周期影响

双收入家庭（5年后典型）：
├── 你（工业界）税后：~35,000 SEK/月
├── 太太（全科医生）税后：~38,000 SEK/月
├── 家庭税后月收入：~73,000 SEK
├── 支出（房租+生活+奉献）：~30,000 SEK
├── 每月可存+投资：~40,000 SEK
└── → 这是非常舒适的中上产生活

投资：ISK账户（极友好的税制）
├── ISK里股票基金只按总值收极低税（~1%），不按收益收税
├── 全球指数基金定投（Avanza / Nordnet 平台）
└── 每月定投 → 长期复利

15年后预期（~48岁，双收入家庭）：
├── 工作存款（双收入10年）：~400-500万 SEK
├── 投资收益（假设年化7%）：~200-300万 SEK
├── 总资产：~600-800万 SEK ≈ 8000万-1亿日元
├── 如果买了房 → 房产增值额外
├── 两人都有瑞典养老金（强制缴纳）
├── 属于瑞典社会的上中产阶级
└── ★ 远超你们在日本能达到的财务水平

路径C：瑞典PhD → 跳去德国/瑞士/荷兰工业界
├── 瑞士薪资 = 瑞典1.5-2倍
├── 太太如果在瑞典拿到执业资格 → 其他EU国家也有互认机制
└── 但医生转国家需要重新认证语言 → 如果定居瑞典则太太不需要再折腾
```

### 28.7 毕业后的流动性 / Post-PhD Mobility

```
瑞典PhD = 欧洲的"入场券"

├── 4年PhD → 申请瑞典永居
├── 5年居住 → 申请EU长期居留 → 任何EU国家工作居留（简化手续）
├── 5年居住 → 可入籍（瑞典护照 = EU护照 = 欧盟自由流动）
│   ⚠️ 中国不允许双重国籍 → 入籍需放弃中国籍（重大决定，不急）
│
├── 直接跳槽其他国家也可以：
│   德国：有PhD → 工作签证快
│   瑞士：非EU但对高学历友好，薪资欧洲最高
│   荷兰：Highly Skilled Migrant签证，PhD秒批
│   挪威/丹麦：北欧内部流动方便
│
└── 总结：瑞典是跳板，不是终点（如果你不想它是终点的话）
```

### 28.8 推荐城市 / Recommended Cities

```
🇸🇪 哥德堡 (Göteborg) ⭐⭐ 综合推荐第1
├── Chalmers大学 + Volvo总部
├── 城市大小舒适（50万人）
├── 住房比斯德哥尔摩好找
├── 有华人教会/团契
├── 人比斯德哥尔摩友善
└── 适合夫妻一起生活

🇸🇪 斯德哥尔摩 (Stockholm) ⭐
├── KTH + Karolinska → 学术最强
├── 工作机会最多、最国际化
├── 华人教会最大
├── 缺点：住房最难、物价最高
└── 适合想毕业后留科技界

🇸🇪 隆德/马尔默 (Lund/Malmö)
├── Lund大学很强 + 离哥本哈根20分钟
├── 气候四国最温和
├── 缺点：城市小

🇳🇴 特隆赫姆 (Trondheim)
├── NTNU工程很强 + 自然环境好
├── 缺点：冬天更冷更暗

🇩🇰 哥本哈根 (Copenhagen)
├── DTU + 哥大 + 非常国际化
├── 缺点：住房贵
```

### 28.9 适应策略 / Adaptation Strategy

```
到校第一周：
├── 学校International Office → 带你办personnummer
├── 等personnummer（2-6周）→ 用现金/Revolut卡过渡
├── 拿到后 → 银行账户 → BankID → 数字世界解锁

善用学校支持：
├── International Office → 行政问题
├── PhD Coordinator → 学术行政
├── 同实验室senior PhD → 问"你当初怎么搞定的"
├── 新生群（Facebook/WhatsApp）→ 互相帮忙

社交：
├── 不要试图"融入瑞典人" → 先融入国际社区+教会
├── 教会（华人+国际）= 最核心的社交圈
├── PhD同事 = 天然社交圈（大家都是外国人）
├── 太太的SFI同学 = 另一个社交圈
└── 心理预期：第一年朋友少是正常的

冬天：
├── 光疗灯（SAD lamp）→ 早上用30分钟
├── 维生素D（药店有卖）
├── 中午出去走15分钟
├── 保持运动（学校健身房便宜）
├── 1月最难 → 2月好转 → 3月明显 → 夏天补偿一切

总体节奏：
├── 前3个月 = 最难（行政、孤独、文化冲击）← 所有人都这样
├── 3-6个月 = 有节奏了
├── 6-12个月 = "其实还行"
├── 1年后 = 要么"这就是我的地方"要么"换个地方试试"
├── 你已经移居过一次（中国→日本）→ 你知道culture shock
├── 这次的动力完全不同：日本=被动选择 → 瑞典=主动选择
│   主动选择的适应力远强于被动选择
└── 有太太+有教会 → 你比大多数独自来的PhD学生条件好得多
```

### 28.10 抉择框架 / Decision Framework

```
你不需要现在抉择。但可以用这个框架在探索中找到答案：

问自己：
├── 5年了，对日本文化的厌恶感是减轻了还是加重了？
├── 如果再过5年还在同样的环境，什么感觉？
├── "偶尔不爽"还是"持续窒息"？
└── 如果是持续加重且窒息 → 不是"适应一下就好"的问题

核心问题：
┌─────────────────────────────────────────┐
│ 如果5年后你还在日本，                      │
│ 你会不会后悔没有试过？                      │
└─────────────────────────────────────────┘

如果答案是"会" → 你已经有答案了。

试了之后：
├── 去了，很好 → 留下
├── 去了，不行 → 回来或去别处（硕士+经验 → 不会没路）
├── 申请中发现不想去 → 至少做过功课，不后悔
└── 无论哪种结果 > "什么都没做但一直后悔"

而且你的时间线是探索模式 — 不是明天就辞职买机票。
答案会在探索过程中自然出现。
```

---

## 二十九、北欧求职跟踪 / Job Search Tracker

> 2026-05-14 开始。记录招聘网站、投递进度、JD分析。

### 29.1 招聘网站清单

**综合平台（最重要）**：

| 网站 | 覆盖 | 特点 | 优先级 |
|------|------|------|--------|
| **LinkedIn Jobs** | 全北欧 | 90%的岗位在这里，HR和猎头活跃 | ⭐⭐⭐ 必用 |
| **Indeed** | 全北欧 | 聚合多个来源，选国家搜索 | ⭐⭐ |

**各国本地网站**：

| 国家 | 网站 | 说明 |
|------|------|------|
| 🇸🇪 瑞典 | Arbetsförmedlingen (arbetsformedlingen.se) | 瑞典官方就业网 |
| 🇸🇪 瑞典 | Jobbland.se | 聚合多个瑞典招聘平台 |
| 🇫🇮 芬兰 | Duunitori.fi | 芬兰最大招聘网 |
| 🇫🇮 芬兰 | TE-palvelut (tyomarkkinatori.fi) | 芬兰官方就业网 |
| 🇩🇰 丹麦 | Jobindex.dk | 丹麦最大招聘网 |
| 🇳🇴 挪威 | Finn.no/jobb | 挪威最大招聘网 |

**搜索关键词**（直接复制粘贴）：
```
Embedded Software Engineer
Embedded C++ Developer
Embedded Systems Engineer
Firmware Engineer
```

### 29.2 LinkedIn Profile 要点

- 头衔用 **Embedded Software Engineer**
- Summary 用笔记21.4的自我介绍模板
- Skills 里加关键词（C++, Embedded, RH850, State Machines, CAN, Git...）
- 设置 **Open to Work**（可以只对HR可见）
- 地点偏好设成 Sweden / Finland / Denmark / Norway

### 29.3 JD分析记录

> 看到感兴趣的岗位，把JD发给Copilot，帮你分析匹配度和差距。

| # | 日期 | 公司 | 职位 | 国家 | 匹配度 | 状态 | 备注 |
|---|------|------|------|------|--------|------|------|
| — | — | — | — | — | — | — | 还未开始投递 |

**状态选项**：📋 关注中 → 📝 准备材料 → 📮 已投递 → 📞 面试中 → ✅ Offer / ❌ 拒绝

### 29.4 投递材料清单

| 材料 | 状态 | 备注 |
|------|------|------|
| 英文简历（CV） | 🔲 未开始 | 用笔记21.9的关键词+技能分级 |
| LinkedIn Profile | 🔲 未开始 | 英文，Open to Work |
| Cover Letter 模板 | 🔲 未开始 | 每个公司微调 |
| GitHub Profile | ✅ 已有 | 私有仓库，面试时可展示 |
| STAR故事（口述练习） | 🔲 未练 | 4个故事英文稿已写好 |



---

## 三十、面试英文Q&A完整版（回家练习用）/ Interview English Q&A Complete Edition

> 2026-05-19 整理。覆盖自我介绍、技术问题、项目问题、行为问题。
> 使用方法：每天练2-3个，对镜子或录音，每个答案控制在60-90秒。

---

### 30.1 自我介绍（60秒版 + 120秒版）

#### 60秒版（电话筛选/HR面用）

> "Hi, my name is [Name]. I'm currently working as an embedded systems engineer in Japan, analyzing a safety-critical transmission control system for a major construction equipment manufacturer. The system runs on a Renesas RH850 microcontroller, and I work with a codebase of over 400 C++ modules.
>
> Over the past few months, I've gone from reading and analyzing production code to writing my own embedded modules from scratch — 18 modules total, including state machines, timer logic, and diagnostic handling. I've also traced complete signal chains through the system, from CAN bus input to diagnostic failover to downstream control logic.
>
> I'm looking for an embedded software engineering position in the Nordics, where I can contribute my C++ and embedded systems skills while continuing to grow as an engineer."

#### 120秒版（技术面用，加技术细节）

> 在60秒版基础上，加上：
>
> "To give you a concrete example of the system's complexity — I traced how a single engine speed signal flows through the system. It starts as raw CAN data coming in on J1939, gets decoded by an auto-generated codec module, then goes through a diagnostic check module that monitors 75 failure conditions across the entire transmission. If any fault is detected, a signal switch module automatically routes downstream consumers to a safe fallback value. Seven different modules depend on this one signal — and they all share the output through const references, which is essentially a lightweight observer pattern with zero runtime overhead.
>
> I've also studied the template mechanism that makes these switch modules work — a single template class generates type-specific code at compile time for different signal types like BoolSignal, RotationalSpeed, or Temperature. This is necessary because the Green Hills C++03 compiler needs to know the concrete type to generate machine code, which is why all template code lives in header files."

---

### 30.2 C++ 技术问题

**Q1: What is a C++ template and why is it useful?**
> "A template is a compile-time code generation mechanism. You write the logic once with a placeholder type, and the compiler generates a separate version for each concrete type used. In our project, we have a signal switch module — `so_switch_model` — that works with any signal type: BoolSignal, RotationalSpeed, Temperature, and so on. Without templates, we'd need to write and maintain a separate class for each type. With the template, one header file handles all of them. The key point is that templates must be in header files because the compiler needs to see the full implementation when it encounters a specific type — it can't generate machine code for an unknown type in a separate .cc file."

**Q2: Why must template implementations be in header files?**
> "Because templates are instantiated at compile time. When the compiler processes a .cc file that uses `so_switch<RotationalSpeed>`, it needs to see the full template definition to generate the concrete code. If the template body were in a separate .cc file, the compiler wouldn't have access to it — .cc files are compiled independently. So the implementation must be in the .h file that gets #included wherever the template is used."

**Q3: What is the difference between a pointer and a reference?**
> "A pointer can be null, can be reassigned, and requires explicit dereferencing with `*`. A reference is an alias — it must be initialized, cannot be null, and cannot be rebound. In our embedded project, module inputs use `const &` references for zero-copy read-only access to other modules' outputs. This avoids the overhead of copying signal objects every cycle."

**Q4: What does `const &` mean in a class member?**
> "It means a read-only reference to data owned by someone else. In our system, input signals are `const &` members — the module can read the value but cannot modify it. This enforces a clear ownership model: only the producing module can write to a signal, and all consumers see the same data through shared const references."

**Q5: What is a virtual function?**
> "A virtual function enables runtime polymorphism. In our framework, the base class `ModuleInterface` declares `virtual void Update()`. Every module overrides this. The Manager calls `Update()` on each registered module without knowing the concrete type — this is the Composite pattern that manages over 400 modules."

**Q6: What is RAII?**
> "Resource Acquisition Is Initialization. You acquire resources in the constructor and release them in the destructor. In our embedded system, we don't use dynamic allocation — no `new` or `delete` — so RAII mainly manifests as proper initialization in constructor initializer lists. Every member is initialized before the constructor body runs."

**Q7: What is the difference between stack and heap allocation?**
> "Stack allocation is automatic — variables are created when entering a scope and destroyed when leaving. It's fast but limited in size. Heap allocation uses `new`/`malloc` — it's flexible but requires manual management and is slower. In safety-critical embedded systems, we avoid heap allocation entirely because it can cause fragmentation and non-deterministic timing. All our objects are statically allocated."

**Q8: What is an initializer list and why use it?**
> "An initializer list initializes members before the constructor body runs. It's mandatory for references and const members since they can't be assigned after construction. In our modules, we use three patterns: reference binding like `input_(external_signal)`, object construction like `timer_(config.timer_config)`, and value initialization like `counter_(0)`. Using the initializer list is also more efficient than assignment in the constructor body."

---

### 30.3 嵌入式系统问题

**Q9: Explain cooperative scheduling vs preemptive scheduling.**
> "In cooperative scheduling, each task runs its update method and voluntarily returns control. The scheduler calls tasks in a fixed order. In preemptive scheduling, the OS can interrupt a running task at any time to run a higher-priority one.
>
> Our system uses cooperative scheduling: every 10 milliseconds, the OS calls `user_main()`, which cascades through a Manager tree calling each module's `Update()`. Every module must complete quickly and return. If one module blocks, the entire system freezes. This gives us deterministic timing but requires discipline — no infinite loops, no blocking waits."

**Q10: How do you handle sensor noise?**
> "We use debounce filtering with increment timers. The timer counts up each cycle when the condition is true, but resets to zero if the condition breaks even once. Only when the timer reaches the threshold — say 500 milliseconds — do we consider the signal stable. This prevents transient spikes from triggering false state changes. I've implemented this pattern in my own practice modules."

**Q11: What is a watchdog timer?**
> "A hardware timer that resets the entire system if software stops responding. The application must periodically 'kick' or 'feed' the watchdog to prove it's still running correctly. If a module hangs or enters an infinite loop, the watchdog expires and forces a hardware reset. It's the last line of defense against software failures in safety-critical systems."

**Q12: Explain big-endian vs little-endian.**
> "It's about byte order in multi-byte values. Big-endian stores the most significant byte first; little-endian stores the least significant byte first. This matters in CAN communication — when two ECUs with different byte orders exchange data, bytes must be swapped. In our J1939 modules, I've seen how 4-byte CAN data is decomposed into individual bytes and reassembled with proper byte order handling."

**Q13: What is a state machine and how have you used one?**
> "A state machine has a set of states, transitions between them, and actions in each state. I've worked with two levels. Simple ones use switch-case with enum states — for example, an edge detection module with four states.
>
> More complex modules use a three-phase pattern: transition phase decides if the state should change, entry phase runs one-time initialization, and do phase handles continuous actions. We detect state changes by comparing `current_state != previous_state` — essentially edge detection at the state level. I've written three-phase state machines from scratch, including one that compiled on the first attempt."

---

### 30.4 CAN / J1939 通信问题

**Q14: What is CAN bus?**
> "CAN — Controller Area Network — is a message-based communication protocol used in vehicles and industrial equipment. It uses differential signaling on two wires for noise immunity. Messages have priority-based arbitration — lower ID wins — so higher-priority messages get through first. There's no addressing — any node can listen to any message. In our system, CAN connects multiple ECUs that share sensor data and control commands."

**Q15: What is J1939?**
> "J1939 is an application-layer protocol built on top of CAN, specifically for heavy-duty vehicles and construction equipment. It uses 29-bit extended CAN IDs. The ID is decomposed into four bytes: priority, a reserved byte, the PGN — Parameter Group Number — which identifies what data the message carries, and the source address. The PGN is the key identifier — for example, PGN 0xFEF5 is Ambient Conditions carrying air temperature and pressure."

**Q16: How does CAN data decoding work in your system?**
> "We have auto-generated codec modules created by a tool called KDOG2. The receiving module takes raw CAN data — 8 bytes — and extracts individual signals using bit manipulation: shift and mask operations. For example, extracting a 16-bit engine speed value from bytes 4 and 5 of the CAN frame. The codec also handles byte order conversion and scaling — converting raw integer values to engineering units like RPM or degrees Celsius. Each decoded signal becomes a signal_object with a value and a validity flag."

---

### 30.5 诊断与安全系统问题

**Q17: How does your system detect and handle faults?**
> "We have a multi-layer approach. First, a `FailureCheckFunction` module monitors a condition with debounce timing — for example, checking if a sensor value is out of range for 500 milliseconds continuously. When the debounce timer expires, it produces a FailureSignal with state OCCUR.
>
> Next, a `DiagFailureJudge` module aggregates an array of FailureSignals — in our transmission system, one array has 75 elements covering brakes, clutches, speed sensors, and oil temperature. If any signal shows OCCUR or RECOVER, the judge outputs true, meaning the system is in a fault condition.
>
> Finally, an `so_switch` module uses that diagnosis result to route downstream consumers to either the real sensor value or a safe fallback value. Seven modules might depend on one sensor — and they all automatically get the safe value when a fault is detected."

**Q18: What is signal validity and why does it matter?**
> "Every signal in our system is wrapped in a signal_object that carries both a value and a validity flag — VALID or INVALID. This means downstream modules always know whether they can trust the data. If a CAN message times out or a sensor fails, the signal becomes INVALID, and downstream logic can react accordingly — typically by using a safe default value or entering a degraded mode."

**Q19: What is limp-home mode?**
> "When critical sensors fail and no backup is available, the system enters a degraded operating mode called limp-home. Instead of shutting down completely — which could leave a 40-ton machine stranded in a dangerous location — the system uses estimated or default values to allow the operator to safely drive the machine to a service area at reduced performance. It's a graceful degradation strategy."

**Q20: Explain redundant sensor design.**
> "For safety-critical measurements like output shaft speed, we use dual sensors — sensor A and sensor B measuring the same physical quantity. A diagnostic module continuously monitors both. If sensor A fails, the system automatically switches to sensor B. If both fail, it falls back to estimated values or enters limp-home mode. This is implemented using the so_switch template module I mentioned — the same code handles any signal type."

---

### 30.6 架构与设计模式问题

**Q21: Describe the software architecture of your system.**
> "It's a four-layer architecture. At the bottom, the RTOS handles hardware and timing. Above that, a middleware layer provides hardware abstraction. The third layer is the framework — the Manager pattern that coordinates all modules. At the top are the application modules — over 400 of them — handling business logic like engine control, diagnostics, and CAN communication.
>
> The Manager implements the Composite pattern: it's both a container and a module itself. The root Manager calls Update() on its children, which may be other Managers or leaf modules. This creates a tree structure where one call propagates through the entire system."

**Q22: What design patterns have you seen in the codebase?**
> "Several. Composite — the Manager tree structure. Dependency Injection — all module inputs are passed via constructor parameters through the INSTANTIATION macro, not created internally. Strategy — the sensor selection module switches between sensor A, B, or estimated values based on diagnostic state. State Machine — used extensively for mode management. Observer — modules observe other modules' outputs through const references. And Template Method — the base class defines the Update() lifecycle that all modules override."

**Q23: What is Dependency Injection and how is it used?**
> "Dependency Injection means a module doesn't create or locate its own dependencies — they're passed in from outside. In our system, a centralized wiring file uses INSTANTIATION macros to construct each module and pass in all its inputs, configs, and dependencies. This means modules are loosely coupled — you can test them with different inputs, and changing a signal source only requires editing the wiring file, not the module code."

---

### 30.7 项目经验 / 行为问题

**Q24: Tell me about a challenging technical problem you solved.**
> "When I first started reading the codebase, I couldn't understand how one signal switch module could serve seven different downstream consumers. In C#, which was my previous language, I'd expect each consumer to have its own instance. But in the embedded system, all seven modules share a single so_switch output through const references — essentially a zero-overhead observer pattern. Understanding this required me to deeply learn C++ reference semantics and the framework's ownership model. Now I can trace any signal through the entire system from source to consumer."

**Q25: How did you learn embedded C++ with no prior experience?**
> "I followed a progressive approach. First, I spent weeks reading production code — understanding the framework, the Manager pattern, signal objects, and the build system. Then I started writing modules from scratch, beginning with simple combinational logic, then adding timers, state machines, and multi-output controllers. I went from 13 compilation attempts on my first module to one-attempt success on my 18th module. I also read the source code of library modules like IncrementTimer and so_switch to understand the underlying implementations, not just the APIs."

**Q26: How do you approach reading unfamiliar code?**
> "I start by identifying the inputs and outputs — in our framework, that's the constructor parameters and the output signal objects. Then I read the Update() method to understand the core logic. For complex modules, I trace specific scenarios step by step — for example, 'what happens when the engine speed drops below the threshold for 500ms?' I also draw state transition diagrams for state machine modules. The key insight I learned is to follow the data flow, not the control flow."

**Q27: Tell me about a time you found something unexpected in the code.**
> "While tracing the diagnostic chain for the engine speed signal, I discovered that the DiagFailureJudge module monitors an array of 75 failure signals — covering the entire transmission system, not just one sensor. At first I thought each sensor would have its own small diagnostic check. But the architecture aggregates all failure conditions into one array, and one judge module evaluates them all with a simple for-loop. This taught me an important design principle: aggregation at the data level keeps the logic simple even when the system is complex."

**Q28: What is your biggest weakness? / What do you want to improve?**
> "I haven't worked with C++11 or later features in production — our compiler is C++03. I understand the concepts of smart pointers, move semantics, and auto, but I haven't used them in a real project. I'm also still building my experience with unit testing frameworks like Google Test. These are areas I'm actively studying and eager to apply in a modern codebase."

---

### 30.8 反问面试官的问题（必须准备2-3个）

> 面试结束时一定会问 "Do you have any questions for us?"
> 说"No"= 没兴趣 / 没准备。以下选2-3个：

1. "What does the tech stack look like for your team? Which compiler and target platform do you use?"
2. "How does your team handle code reviews and knowledge sharing?"
3. "What would a typical first project look like for someone joining your team?"
4. "How do you balance between developing new features and maintaining existing code?"
5. "What does the development cycle look like — how often do you release to production?"

---

### 30.9 每日练习计划（回家用）

```
周一：60秒自我介绍（录音→听→改→再录）
周二：Q1-Q4（C++基础）
周三：Q9-Q13（嵌入式系统）
周四：Q14-Q16（CAN/J1939）
周五：Q17-Q20（诊断与安全）
周六：Q21-Q23（架构与设计模式）
周日：Q24-Q28（行为问题）+ 反问

第二周重复，但不看答案试着自己说。
录音对比第一周，你会发现明显进步。
```

**练习要点**：
- 不要背稿，理解逻辑后用自己的话说
- 每个答案60-90秒，不要太长
- 说慢一点 > 说快但卡壳
- 不会的词用解释代替："the module that switches between signals" = so_switch
- 练到能不看笔记流利说出来 = 准备好了



---

## 三十一、求职实战指南（5/19更新）/ Job Search Practical Guide

> 2026-05-19 整理。包含北欧市场实况、时间安排、求职策略。

### 31.1 北欧嵌入式岗位市场（2026年5月实查数据）

**瑞典官方就业网（Arbetsförmedlingen）实时查询结果**：
- 搜索关键词 "Embedded" → **268个招聘广告，388个岗位**
- 这还不包括 LinkedIn、Indeed、公司官网直招

#### 在招公司示例（2026年5月）

| 公司 | 城市 | 岗位 | 类型 |
|------|------|------|------|
| Knightec | 哥德堡/Västerås | Embedded Engineer | 咨询 |
| Piab | Danderyd | Embedded Developer | 产品 |
| Knowit | Linköping/斯德哥尔摩 | Embedded Developer Automotive | 咨询 |
| Nexer Mobility | 哥德堡 | ADAS Embedded Developer | 咨询 |
| Techrytera | 哥德堡/Lund | Embedded Software Engineer | 咨询 |
| Avaron | Södertälje | Embedded Software Developer | 咨询 |
| Friday | Linköping/斯德哥尔摩 | Embedded C/C++ Developer | 咨询 |
| Multiply | 不限地点 | Embedded C/C++, Linux | 咨询 |
| Synective Labs | 哥德堡 | Embedded + FPGA | 产品 |

**观察**：
- 哥德堡（Göteborg）是嵌入式重镇（Volvo总部所在地）
- 大量咨询公司（Knightec, Knowit, Friday）= 一个广告对应多个项目
- 有 Automotive 方向 = 你的建机经验直接对口

#### 北欧整体估计

```
瑞典官网：      388个嵌入式岗位（实查）
芬兰：          ~100-200个
丹麦：          ~100-200个
挪威：          ~50-100个
LinkedIn额外：  ~200-400个
────────────────────────────
北欧总计：      约 800-1300个活跃嵌入式岗位
```

#### 招聘网站

| 网站 | 覆盖 | 备注 |
|------|------|------|
| LinkedIn Jobs | 全北欧 | 90%的岗位，⭐必用 |
| arbetsformedlingen.se | 🇸🇪 瑞典 | 官方网站，388个岗位 |
| Duunitori.fi | 🇫🇮 芬兰 | 芬兰最大 |
| Jobindex.dk | 🇩🇰 丹麦 | 丹麦最大 |
| Finn.no/jobb | 🇳🇴 挪威 | 挪威最大 |

### 31.2 在日本远程面试流程

**不需要先搬到北欧。标准流程：**

```
在日本投简历 → 远程面试 → 拿offer → 公司sponsor签证 → 辞职 → 搬家
```

| 阶段 | 方式 | 说明 |
|------|------|------|
| 投简历 | LinkedIn/公司官网 | 在日本直接投 |
| HR筛选 | 视频30分钟 | 时差调整即可 |
| 技术面试 | 视频1-2轮 | 可能含live coding |
| 终面 | 视频 | 团队匹配 |
| 入职 | 拿offer后搬家 | 公司办工作签证 |

**时差**：日本下午5点 = 北欧上午10点。面试约在下班前后完全可行。

#### 工作签证（公司帮你办）

| 国家 | 签证类型 | 处理时间 |
|------|---------|---------|
| 🇸🇪 瑞典 | Work Permit | 2-4个月 |
| 🇫🇮 芬兰 | Residence Permit for Work | 1-3个月 |
| 🇩🇰 丹麦 | Fast-track / Pay Limit Scheme | 1-2个月 |
| 🇳🇴 挪威 | Skilled Worker Permit | 1-3个月 |

### 31.3 PhD与工作同时推进（不冲突）

```
路线A（工作）：投简历 → 面试 → offer → 搬到北欧工作（3-6个月）
路线B（PhD）：探索 → 联系导师 → 申请 → 入学（按你的节奏，可能2027秋）

两条线完全独立，哪个先有结果就先走哪个。
```

**4种结果都不亏**：
1. 先拿到工作offer → 去工作，以后想读PhD随时申请
2. 先拿到PhD offer → 去读博，4年后就业面更广
3. 两个都拿到 → 选你更想要的（有选择 = 赢）
4. 暂时都没有 → 继续在日本工作+继续提升，零损失

### 31.4 每周时间安排（现实版）

**白天（8:00-18:00）在公司**：
- ✅ 嵌入式学习（和工作相关，已经在做）
- ❌ 投简历/改LinkedIn（明显非工作内容）
- ❌ 读论文/PhD探索（和工作无关）

**晚上原则：每晚只做 ONE THING，30-45分钟，做完就休息**

| 星期 | 晚上做的一件事 | 时间 |
|:---:|------------|:---:|
| 一 | 练英文面试Q&A（第30章） | 30min |
| 二 | 练英文面试Q&A | 30min |
| 三 | 投简历（LinkedIn投2-3个岗位） | 30min |
| 四 | 练英文面试Q&A / STAR故事 | 30min |
| 五 | **休息** | 0 |
| 六 | 投简历 + 改简历 | 1h |
| 日 | PhD探索（论文标题/导师主页） | 1h（不强制） |

### 31.5 求职心理建设

```
找工作是概率游戏，不是考试：
├── 投10个，0回复 → 正常，继续投
├── 投30个，拿到2-3个面试 → 标准转化率
├── 第一次面试表现不好 → 所有人第一次都不好
├── 典型周期：3-6个月，投50-100个岗位
└── 找工作失败的代价 ≈ 0。不找的代价 = 永远不知道自己行不行

不需要准备到100%才开始。70%就投。
面试本身会把你从70%推到90%。
```

### 31.6 启动清单

```
第1周（本周）：
□ 每晚30分钟练第30章Q&A（对着手机录音）

第2周：
□ 继续练Q&A + 把LinkedIn改成英文
□ LinkedIn设 Open to Work（可只对HR可见）

第3周：
□ 用笔记21.9写英文简历
□ 投出第一个岗位 ← 里程碑！

之后持续：
□ 每周投3-5个 + 每天练30分钟英文
□ 面试后记录问到的题，回来补
```


---

## 三十二、FailureCheckFunction 源码阅读（5/19）

> 故障检出的源头模块。诊断链第2层——把 BoolSignal 条件转换为 FailureSignal（NORMAL/OCCUR/RECOVER）。

### 32.1 模块结构

```
diagnosis_check_model::FailureCheckFunction
├── .h：module/wlh/include/diagnosis_check_model_failure_check_function.h
├── .cc：module/wlh/src/diagnosis_check_model_failure_check_function.cc
├── 继承：ModuleInterface（有 Update()）
├── 核心成员：
│   ├── failure_（FailureDetector）← 中间件层的故障判定引擎
│   └── diagnosis_check_flag_（const BoolSignal&）← 输入条件
└── 代码量：.h 约50行，.cc 约30行
```

### 32.2 构造函数

```cpp
FailureCheckFunction(config, diagnosis_check_flag, mask, manager)
    : diagnosis_check_flag_(diagnosis_check_flag),    // 绑定输入
      failure_(config.failure_config, mask) {          // 创建 FailureDetector
  manager.RegistModule(*this);                         // 注册到 Manager
}
```

**3个输入**：
- `diagnosis_check_flag`（BoolSignal）：检查条件（例如"发动机转速超限了吗？"）
- `mask`（FaultTransitionMaskOrder）：故障转移掩码（"现在允许判定吗？"）
- `config`：定时器配置（防抖时间）

### 32.3 Update()——核心逻辑（就一行）

```cpp
void FailureCheckFunction::Update() {
    failure_.Update(diagnosis_check_flag_.GetValue());
}
```

把 BoolSignal 的值传给 FailureDetector，让它更新内部状态。

### 32.4 FailureDetector 内部机制（从Config推导）

FailureDetector 在中间件层（.lib），C++ 源码不可见。但从 C# Config 推导出完整行为：

```
FailureDetector::Config {
    FaultTimer::Config timer {
        bool restable;                     // 条件中断时重置？
        IncrementTimer::Config set_time;   // 故障确认定时器
        IncrementTimer::Config clear_time; // 故障清除定时器
    }
}
```

**状态转移**：
```
              set_time 到期
  NORMAL ──────────────────→ OCCUR（故障发生）
    ↑                           │
    │      clear_time 到期      │
    └───────────────────────────┘
            RECOVER（故障恢复）
```

- set_time：条件持续成立 N ms → 确认故障（防抖）
- clear_time：条件持续消失 N ms → 确认恢复
- restable=true：条件中断一次就重置 set_time（和 IncrementTimer 一致）

### 32.5 mask 的作用

```
mask = MaskOr/MaskAnd 的输出（FaultTransitionMaskOrder）

├── mask 允许 → FailureDetector 正常判定
└── mask 禁止 → FailureDetector 暂停（不让故障状态变化）

禁止判定的场景：
├── 系统启动中（信号不稳定）
├── 车速太低（某些故障低速时不应判定）
└── 钥匙关闭后（避免关机瞬间误判）
```

### 32.6 真实配置示例

```cpp
// dg_failure_hmt_engine_over_run（发动机超速检测）
config = {
    restable = true,
    set_time  = { sampling=10ms, initial=0, time_up=0 },  // 0ms = 立即判定
    clear_time = { sampling=10ms, initial=0, time_up=0 }   // 0ms = 立即恢复
};
// → 超速 = 危险状态，不需要防抖，检测到就立即报OCCUR
```

其他故障可能配置 set_time=500ms（持续500ms才确认），和之前学的 debounce 逻辑一致。

### 32.7 真实接线

```cpp
// dg_failure_hmt_engine_over_run 的实际接线
INSTANTIATION(
    (FailureCheckFunction),
    dg_failure_hmt_engine_over_run,
    /* config : */ dg_failure_hmt_engine_over_run_config,
    /* diagnosis_check_flag : */ hmt_clutch_and_synchro_protection.IsEngineOverRunRef(),
    /* mask : */ mask_or_function_ce24662f.ResultRef(),
    /* manager : */ main_app_manager_node00_16);
```

- 条件来源：`hmt_clutch_and_synchro_protection` 模块的发动机超速判定
- 掩码来源：`MaskOr` 的组合输出

### 32.8 完整诊断链路（6层全通）

```
第1层：条件产生
  上游业务模块输出 BoolSignal（发动机超速？油温过高？传感器断路？）

第2层：FailureCheckFunction ★本次学习★
  输入：BoolSignal + mask
  内部：FailureDetector（双定时器防抖）
  输出：FailureSignal（NORMAL / OCCUR / RECOVER）

第3层：FailureSignal 数组聚合
  多个 FailureSignal 放入 Array（最大75个元素）

第4层：DiagFailureJudge
  遍历数组，任何一个 OCCUR/RECOVER → IsFailure = true

第5层：so_switch<SoType>
  IsFailure=true → 输出安全值（固定值）
  IsFailure=false → 输出真实传感器值

第6层：下游业务模块（通过 const& 共享 so_switch 输出）
```

### 32.9 额外发现：LimpHome 系统

接线文件中发现 **28个 LimpHomeClutchAssociateFunction** 实例：
```
limp_home_clutch_when_hst_pressure_sensor_failure   // HST油压传感器故障
limp_home_clutch_when_m2_rev_sensor_failure          // M2转速传感器故障
limp_home_clutch_when_pm1_sol_open_failure           // PM1电磁阀开路故障
limp_home_clutch_when_synchro_failure                // 同步器故障
limp_home_clutch_when_eng_comm_failure               // 发动机通信故障
limp_home_clutch_when_tm_output_rev_sensor_failure   // 变速箱输出转速传感器故障
... 共28种故障 → 各自决定跛行模式的离合器状态
```

每个 limp_home 模块的输入都是 `DiagFailureJudge.IsFailureRef()` → 与诊断链直接相连。

### 32.10 本模块的面试价值

```
面试回答（加入第30章Q17的补充）：

"The fault detection pipeline has six layers. At the core is a 
FailureCheckFunction module that takes a boolean condition — like 
'is engine speed above the limit?' — and a timing mask that prevents 
false positives during startup or shutdown. Inside, it uses a 
FailureDetector with dual timers: a set timer for fault confirmation 
and a clear timer for recovery confirmation. This is essentially 
debounce filtering applied to fault detection.

When a fault is confirmed, the system has 28 separate limp-home 
modules — one for each specific failure type — that determine 
the safe clutch configuration for degraded operation."
```

### 模块阅读进度更新

| 领域 | 已读模块 | 数量 |
|------|---------|:---:|
| CAN通信 | F00400, FEF500 | 2 |
| 通用(common) | so_switch, so_comparator, so_buffer | 3 |
| 诊断(diagnosis) | DiagFailureJudge, DiagNormalJudge, MaskOr, MaskAnd, FailureCheckFunction | 5 |
| **合计** | | **10** |


---

## 三十三、fault_judge 第1层源码阅读 + 搜索技能（5/19）

> 诊断链第1层——从传感器状态判定故障条件。学习了 Template Method 设计模式和接线文件搜索技巧。

### 33.1 基类：fault_state_detection_model::Function

**文件**：module/wlh/include/fault_state_detection_model_function.h + .cc

```cpp
class Function : public framework::ModuleInterface {
public:
    struct Config {
        abnormality::FailureDetector::Config fault_detector_config;
    };

    Function(config, fault_mask, manager);
    void Update();                              // 基类定义流程
    const FailureSignal& FailureSignalRef();    // 输出

protected:
    virtual bool IsFaultCondition();    // 子类覆盖 → 默认返回true
    virtual bool IsRecoverCondition();  // 子类覆盖 → 默认返回true

private:
    abnormality::FailureDetector fault_detector_;  // 内含防抖定时器
};
```

**Update() 核心逻辑**：
```cpp
void Function::Update() {
    bool flag = false;
    switch (当前故障状态) {
        case NO / CHECK:
            flag = IsFaultCondition();                        // 正常时：只看故障条件
            break;
        case OCCUR / RECOVER:
            flag = IsFaultCondition() || !IsRecoverCondition(); // 故障时：故障条件OR未恢复
            break;
    }
    fault_detector_.Update(flag);
}
```

**安全方向偏置**：
- 进故障：IsFaultCondition()=true 就行
- 退故障：IsFaultCondition()=false **且** IsRecoverCondition()=true 才行
- → 宁可多报故障，不轻易解除

**这就是 Template Method 设计模式**——基类定义 Update() 流程，子类只填 IsFaultCondition/IsRecoverCondition。

### 33.2 子类：rotational_speed_sensor_fault_judge_model::DetectFunction

**文件**：module/wlh 下的 rotational_speed_sensor_fault_judge_model_detect_function.h + .cc

```cpp
// 构造函数
DetectFunction(config, fault_mask, refresh_state, open_failure_signal, manager)
    : Function(config.super, fault_mask, manager),   // 调基类
      refresh_state_(refresh_state),                  // 绑定传感器刷新状态
      open_failure_signal_(open_failure_signal) {}    // 绑定开路故障信号

// "传感器故障了吗？"
bool DetectFunction::IsFaultCondition() {
    if ((!refresh_state_.GetValue())                              // 信号没刷新
        && (open_failure_signal_.GetValue() == EnumFailureState::NO)) { // 且不是已知的开路故障
        return true;   // → 可能是接地短路
    }
    return false;
}

// "传感器恢复了吗？"
bool DetectFunction::IsRecoverCondition() {
    return refresh_state_.GetValue();  // 信号重新刷新了
}
```

### 33.3 open_failure_signal == NO 的含义（易错）

**错误理解**：== NO 表示"有断线故障" ❌
**正确理解**：== NO 表示"没有断线故障" ✅

```
open_failure_signal == NO → true  → 断线故障【没有】发生
open_failure_signal == NO → false → 断线故障【已经】发生（值是OCCUR）
```

**为什么需要这个条件？故障隔离**：
```
一根线断了（开路）→ 信号也会停止刷新

没有这个条件：
  模块A（断线检测）：报OCCUR ✅
  模块B（本模块）  ：也报OCCUR ❌ → 重复报告！一个根因触发两个故障

加了 == NO 条件：
  模块A 已报断线 → open_failure = OCCUR → 本模块条件不满足 → 不报
  → 一个根因只报一个故障 ✅
```

### 33.4 诊断链完整路径（Motor 2 转速传感器为例）

```
层0  device::RotationalSpeedSensorByAcPulseInput（m2_input_rev）
     → PulseRefreshStateRef()（信号刷新状态）
     → OpenFailureRef()（开路故障信号）
 ↓
层1  rotational_speed_sensor_fault_judge_model::DetectFunction
     （dg_motor_2_sensor_gnds_fault_judgement_1）
     → IsFaultCondition() + FailureDetector 防抖
     → FailureSignalRef()
 ↓
层1.5 failure_signal_conbine_model::Function
     （dg_m2_speed_sensor_ground_short_fault_conbine）
     → 合并 judgement_1 + judgement_2
 ↓
层3  failure_signal_d9b4852d 数组（3个故障信号）
 ↓
层4  DiagFailureJudgeFunction（diag_failure_judge_function_cb9da75b）
     → IsFailureRef()
 ↓
层5  LimpHomeClutchAssociateFunction
     （limp_home_clutch_when_m2_rev_sensor_failure）
     → 限速/限扭矩
```

### 33.5 接线文件搜索技巧

**核心原则**：每一步的输入都是上一步的输出，不需要猜名字。

#### 往下追（找下游）
```
已知实例名 → Ctrl+Shift+F 搜 "实例名."
→ 找到 实例名.XxxRef() 被谁使用
→ 那个使用者就是下游
```

#### 往上追（找上游）
```
已知实例名 → Ctrl+Shift+F 搜 "实例名,"
→ 找到 INSTANTIATION 定义
→ 看参数列表 → 参数里的 xxx.YyyRef() 就是上游
```

#### 参数分类（哪些要追，哪些不追）
| 参数类型 | 例子 | 要不要追 |
|---------|------|---------|
| config | xxx_config | 不追 — 编译时常量 |
| manager | main_app_manager_xxx | 不追 — 框架注册 |
| fault_mask | mask_or_function_xxx.ResultRef() | 可追 — 但是侧面控制通道 |
| 诊断信号 | m2_input_rev.PulseRefreshStateRef() | **要追** — 主数据流 |

#### 搜索口诀
```
"顺藤摸瓜"：
1. 搜 INSTANTIATION → 找到实例名 + 上游参数
2. 搜 实例名. → 找到下游使用者
3. 如果进了数组 → 搜 数组名, → 找到数组去向
4. 重复直到终点
```

### 33.6 自测题（已答部分批改 + 未答部分）

#### 已答题目批改（6/10）

| 题号 | 结果 | 说明 |
|:---:|:---:|------|
| 1 | ✅ | refresh_state=false → 传感器信号停止刷新 |
| 2 | ❌→✅ | == NO 是"没有断线"不是"有断线"。目的是故障隔离，避免重复报告 |
| 3 | ✅ | false\|\|!false = true。安全偏置——恢复条件不满足就继续保持故障 |
| 4 | ⬜→✅ | 通过搜索实践学会了完整链路追踪 |
| 5 | ✅ | 300ms恢复，定时器复位，输出NO |
| 6 | ⚠️ | mask=ON → FailureDetector暂停判定，防止启动时误报。不能只说"MaskOn" |

#### 未答题目（明天继续）

**第7题（C++ virtual）**
基类 IsFaultCondition() 是 virtual，子类覆盖它。靠什么关键字实现多态？如果忘了写 virtual 会怎样？

**第8题（MaskOr vs MaskAnd 的 break）**
MaskOr 循环里没有 break，MaskAnd 有 break。为什么？如果 MaskAnd 也不加 break 会有什么后果？

**第9题（四种故障状态）**
FailureSignal 有 NO/CHECK/OCCUR/RECOVER 四种状态。为什么不能只用 NO/OCCUR 两种？CHECK 和 RECOVER 各存在的意义是什么？

**第10题（动手设计）**
如果你要新增"液压油温度传感器故障判定"模块：
- 继承哪个类？
- 覆盖哪两个方法？
- IsFaultCondition() 里大概写什么逻辑？（假设输入是 oil_temp_valid 信号）
- 需要写 Update() 吗？为什么？


### 模块阅读进度更新

| 领域 | 已读模块 | 数量 |
|------|---------|:---:|
| CAN通信 | F00400, FEF500 | 2 |
| 通用(common) | so_switch, so_comparator, so_buffer | 3 |
| 诊断(diagnosis) | DiagFailureJudge, DiagNormalJudge, MaskOr, MaskAnd, FailureCheckFunction, **fault_state_detection_model::Function（基类）**, **rotational_speed_sensor_fault_judge_model（子类）** | **7** |
| **合计** | | **12** |



---

## 三十四、Quiz Q7-10 完成 + fault_judge 练习模块实战（5/20）

> 完成了自测题 Q7-10，学习了 C++ template 与 Template Method 的区别，掌握了"找参照物"的工程方法，动手写了4个 fault_judge 练习模块（从7个错误到0个错误）。

### 34.1 自测题 Q7-10 答案与批改

#### Q7（C++ virtual）⚠️

**问题**：基类 IsFaultCondition() 是 virtual，子类覆盖它。靠什么关键字实现多态？如果忘了写 virtual 会怎样？

**我的回答**：不知道靠什么关键字；忘了写 virtual 就不会被强制提醒 → 混淆了 virtual 和纯虚函数(=0)

**正确答案**：
- 靠 `virtual` 本身实现多态。基类写了 virtual，子类只要写同名同参数函数就自动覆盖
- C++03 没有 override 关键字（C++11 才有）
- 忘了写 virtual → **编译通过，但运行时调错版本**（静态绑定 → 永远调基类默认实现）

| | `virtual` | `virtual ... = 0`（纯虚函数） |
|---|---|---|
| 作用 | 运行时调子类版本（多态） | 强制子类必须实现，否则编译报错 |
| 基类能有默认实现？ | ✅ 能 | ❌ 不能 |
| 忘了写的后果 | 编译通过，运行时调错版本 | 编译报错 |

#### Q8（MaskOr vs MaskAnd 的 break）⚠️

**问题**：MaskOr 没有 break，MaskAnd 有 break。为什么？

**我的回答**：AND 的逻辑对了（找到OFF就终止），但 OR 的描述说反了（说成了"所有ON才true"）

**正确答案**：
- OR：**任何一个ON就true**（有一个理由就屏蔽）→ 理论上也可以 break 但没加
- AND：**任何一个OFF就false**（有一个不满足就不屏蔽）→ 提前 break 提高效率
- MaskAnd 不加 break → 功能不变，只是多余循环浪费时间

**MaskOr 实际用途（真实例子）**：
```
M2转速传感器的 mask_or（3个输入）：
├── 嵌套MaskOr的结果（系统启动中）
├── 发动机低转速mask（低速时传感器信号弱，不算故障）
└── M2转速不一致mask（另一个故障正在处理中）

任何一个ON → 暂停故障判定 → 减少误报
```

#### Q9（四种故障状态）⚠️

**问题**：为什么不能只用 NO/OCCUR，CHECK 和 RECOVER 各存在的意义？

**我的回答**：充当缓冲区使判断延迟 → 方向对但机制说错了（延迟是定时器的工作）

**正确答案**：
```
NO ──→ CHECK ──→ OCCUR ──→ RECOVER ──→ NO
  条件出现    定时器到期   恢复条件出现  定时器到期
  开始计时    确认故障     开始计时      确认恢复
```

三个意义：
1. Update() 用不同判定逻辑：NO/CHECK 用 `IsFaultCondition()`，OCCUR/RECOVER 用 `IsFaultCondition() || !IsRecoverCondition()`
2. 下游可区分状态（预警 vs 确认 vs 恢复中）
3. 防止边界震荡：NO→CHECK→（恢复）→NO，不用经过 OCCUR

#### Q10（动手设计）❌

**问题**：新增油温传感器故障判定模块

**我的回答**：全错
- 继承谁？→ 说了 ModuleInterface ❌（应该是 fault_state_detection_model::Function）
- 覆盖什么？→ 不知道 ❌（应该是 IsFaultCondition + IsRecoverCondition）
- IsFaultCondition 写什么？→ 说写 MaskOr ❌（应该写 !oil_temp_valid_.GetValue()）
- 要不要 Update？→ 不要 ✅（但理由说的是"没什么需要更新"，正确理由是"基类已经写好了"）

#### 最终成绩

| 题号 | 结果 | 关键教训 |
|:---:|:---:|------|
| 1 | ✅ | refresh_state=false → 传感器停了 |
| 2 | ❌→✅ | == NO 是"没有"不是"有" |
| 3 | ✅ | 安全偏置：false\|\|!false = true |
| 4 | ⬜→✅ | 搜索口诀：顺藤摸瓜 |
| 5 | ✅ | 防抖：300ms恢复 → 定时器复位 |
| 6 | ⚠️ | mask=ON → 暂停判定，防启动误报 |
| 7 | ⚠️ | virtual = 多态；忘写 → 静态绑定 |
| 8 | ⚠️ | OR/AND 描述反了；MaskOr更常用 |
| 9 | ⚠️ | CHECK/RECOVER是状态机中间态 |
| 10 | ❌ | 继承Function不是ModuleInterface |

**总评：3✅ + 4⚠️ + 3❌ — 理解了零件但还没建立"组装"的思维**

### 34.2 C++ template vs Template Method（关键区分）

> 混淆原因：两个概念都包含"template"这个词，但完全不同。

| | C++ template（模板） | Template Method（设计模式） |
|---|---|---|
| 代表模块 | `so_switch_model_function.h` | `fault_state_detection_model_function.h` |
| 关键字 | `template<typename SoType>` | `virtual` |
| 解决什么 | 同一套代码处理不同**数据类型** | 同一个流程填不同**判定逻辑** |
| 变化的是 | 类型（BoolSignal, Uint8Signal...） | 逻辑（转速、油温、电磁阀...） |
| 什么时候决定 | **编译时** | **运行时** |
| 需要写新文件吗？ | **不需要**，INSTANTIATION指定类型即可 | **需要**，每种逻辑一个新的子类 |

**一句话记忆**：
```
C++ template    = "类型不同，逻辑相同" → 编译器复制代码
Template Method = "流程相同，步骤不同" → 子类填空
```

### 34.3 工程方法：找参照物确定继承关系

> "不知道继承谁" → 不是背出来的，是查出来的

**流程**：
```
"我要写一个 XXX 模块"
  ↓
"系统里有没有类似的模块？" → Ctrl+P 搜关键词
  ↓
"打开一个，看它继承了谁"
  ↓
"再打开两三个，确认是不是都继承同一个"
  ↓
"那我也继承这个"
```

**实证**：所有 `*fault_judge*function.h` 都继承 `fault_state_detection_model::Function`

**不同模块类型的参照**：
| 想写什么 | 搜什么找参照 | 继承谁 |
|---------|------------|-------|
| 传感器故障判定 | `*fault_judge*function.h` | `fault_state_detection_model::Function` |
| 信号切换 | `*so_switch*function.h` | 不继承，直接用（C++ template） |
| 业务逻辑 | 同类功能模块 | 通常是 `ModuleInterface` |

### 34.4 三层继承链

```
framework::ModuleInterface               ← 最底层（.lib，有 Update() 接口）
  └── fault_state_detection_model::Function   ← 中间层（有 FailureDetector + Update 流程）
        ├── rotational_speed_sensor_fault_judge_model::DetectFunction
        ├── oil_temp_sensor_fault_judge_model::DetectFunction ← 我写的
        ├── coolant_temp_sensor_fault_judge_model::DetectFunction ← 我写的
        ├── fuel_pressure_fault_judge_model::DetectFunction ← 我写的
        ├── vehicle_speed_sensor_fault_judge_model::DetectFunction ← 我写的
        └── ...另外52个
```

**你只需要面对直接父类**，不需要管"爷爷类"。

### 34.5 安全哲学：判定前 vs 判定后

> 纠正了"宁可误报也要报告"的错误理解

| 阶段 | 策略 | 原因 |
|------|------|------|
| **判定之前**（mask） | 宽松屏蔽，少误报 | 假故障 → 不该限速却限速 → 危险 |
| **判定过程**（防抖） | 持续N毫秒才确认 | 瞬间干扰不算故障 |
| **确认之后**（LimpHome） | 立刻行动，宁可过度保护 | 真故障 → 立刻保护 |
| **恢复过程** | 严格条件才解除 | 宁可多保护一会儿 |

**一句话：判定前小心谨慎，判定后立刻行动。**

### 34.6 fault_judge 练习模块实战（4个模块）

#### 练习进度

| # | 模块名 | 错误数 | 主要错误 |
|---|--------|:---:|------|
| 1 | oil_temp_sensor | 7 | 拼写、namespace太长、public::多冒号、class缺分号、virtual多余、缺include、using写法错 |
| 2 | coolant_temp_sensor | 5→1 | BoolSignal缺&（参数和成员都忘）、构造函数缺分号、初始化列表顺序反、class缺分号 |
| 3 | fuel_pressure_sensor | 6→0 | Config用了引用、缺fault_mask/manager参数、用指针*代替引用&、getValue大小写 |
| 4 | vehicle_speed_sensor | **0** | 编译通过 ✅ |

**7个错误 → 6个 → 5→1 → 0个** = 从完全不会到零错误

#### 高频错误总结

| 错误 | 出现次数 | 原因 |
|------|:---:|------|
| 成员变量忘记加 `&`（引用） | 3次 | 不理解为什么输入信号必须是引用 |
| class 结尾 `}` 后缺 `;` | 3次 | C++ 语法规则，容易忘 |
| 构造函数初始化列表顺序错 | 1次 | 基类必须在第一个 |
| Config 成员用了 `const &` | 1次 | 结构体成员不能是引用 |
| `getValue` 大小写错 | 1次 | 应该是 `GetValue` |

#### 学到的 C++ 知识点

1. **基类构造函数必须显式调用**：如果基类构造函数有参数 → 子类必须在初始化列表里调用 `Function(config.super, fault_mask, manager)`
2. **初始化列表顺序**：基类构造函数必须写在最前面
3. **结构体成员不能是引用**：`const BoolSignal& super` ❌ → `BoolSignal super` ✅（结构体需要可赋值）
4. **引用 vs 值传递**：输入信号用 `const BoolSignal&`（引用），这样能读到运行时的最新值；如果用值传递会拷贝一份，之后更新就读不到了
5. **短名 vs 全名调用基类构造函数**：
   ```cpp
   // 短名（推荐）
   ):Function(config.super, fault_mask, manager),
   // 全名（也可以）
   ):logic::wl::diagnosis::fault_state_detection_model::Function(config.super, fault_mask, manager),
   ```
6. **子类不需要重复写 virtual**：基类已经声明了 virtual，子类直接写 `bool IsFaultCondition();` 即可
7. **`#include` 传递性**：如果你 include 的头文件已经 include 了另一个头文件，编译上不需要重复 include。加上是最佳实践但不是必须

#### 练习文件位置

```
practice_for_diagnosis/
├── oil_temp_sensor_fault_judge_model_detect_function/
│   ├── oil_temp_sensor_fault_judge_model_detect_function.h
│   └── oil_temp_sensor_fault_judge_model_detect_function.cc
├── coolant_temp_sensor_fault_judge_model/
│   ├── coolant_temp_sensor_fault_judge_model.h
│   └── coolant_temp_sensor_fault_judge_model.cc
├── fuel_pressure_sensor_fault_judge_model/
│   ├── fuel_pressure_sensor_fault_judge_model.h
│   └── fuel_pressure_sensor_fault_judge_model.cc
├── vehicle_speed_sensor_fault_judge_model/
│   ├── vehicle_speed_sensor_fault_judge_model.h
│   ├── vehicle_speed_sensor_fault_judge_model.cc
│   └── vehicle_speed_sensor_fault_judge_model.o  ← 编译成功
└── build.bat
```

### 34.7 RL离合器电磁阀故障链路（独立追踪）

用户独立追踪了一条完整的诊断链路（用了搜索口诀"顺藤摸瓜"）：

```
LimpHomeClutchAssociateFunction
  (limp_home_clutch_when_rl_clutch_open_failure)
  ↑
DiagFailureJudge (c9d1bdfd)
  ↑
failure_signal_6462162f（1个故障信号）
  ↑
dg_clutch_rl_sol_open_fault_judgement
  ↑
clutch_rl_ecmv (device::CurrentOutput)
```

**新发现**：系统不只监控输入（传感器），也监控**输出（执行器）**。
- 传感器链：RotationalSpeedSensor → 信号没刷新 → 故障
- 执行器链：CurrentOutput → 下了指令但没检测到电流反馈 → 开路故障

### 模块阅读进度更新

| 领域 | 已读模块 | 数量 |
|------|---------|:---:|
| CAN通信 | F00400, FEF500 | 2 |
| 通用(common) | so_switch, so_comparator, so_buffer | 3 |
| 诊断(diagnosis) | DiagFailureJudge, DiagNormalJudge, MaskOr, MaskAnd, FailureCheckFunction, fault_state_detection_model::Function（基类）, rotational_speed_sensor_fault_judge_model（子类） | 7 |
| **合计** | | **12** |

### 练习模块进度更新

| 领域 | 已写模块 | 数量 |
|------|---------|:---:|
| 之前练习 | parking_lamp, reverse_buzzer, fan_control 等 | 18 |
| **诊断(新)** | **oil_temp_sensor, coolant_temp_sensor, fuel_pressure_sensor, vehicle_speed_sensor** | **4** |
| **合计** | | **22** |




---

## 三十五、RTOS 基础知识 + CAN中断机制 + clutch_slip 练习（5/21）

> 今日学习内容：(1) clutch_slip_fault_judge_model 编写+调试 (2) RTOS 基础知识专题 (3) CAN中断机制问答

### 35.1 clutch_slip_fault_judge_model 练习

**模块功能**：检测离合器打滑故障。当离合器处于啮合状态（FL/RL）且发动机转速与变速箱输出轴转速差超过阈值时，判定为打滑。

**继承自**：fault_state_detection_model::Function（Template Method 模式）

**关键代码结构**：
```
Config {
    fault_state_detection_model::Function::Config super;  // 基类config
    float32 engine_speed_threshold;   // 发动机转速阈值
    float32 tm_output_speed_threshold; // 变速箱输出轴转速阈值
};

输入：
├── engine_speed_（const RotationalSpeed&）
├── tm_output_speed_（const RotationalSpeed&）
└── clutch_state_（const TmState&）

IsFaultCondition()：
  (clutch == FL || clutch == RL) 
  && engine_speed > threshold 
  && tm_output_speed > threshold

IsRecoverCondition()：
  engine_speed ≤ threshold || clutch == NN
```

#### 编译过程中遇到的6个错误

```
1. 构造函数参数类型写法
   ❌ const Config& config → 写成了别的
   ✅ const ClutchStateDetectFunction::Config& config

2. 基类构造函数调用
   ❌ Function(config, fault_mask, manager)
   ✅ Function(config.super, fault_mask, manager)  ← 必须传 config.super

3. 信号读取忘了 .GetValue()
   ❌ engine_speed_ > threshold
   ✅ engine_speed_.GetValue() > threshold

4. 枚举值比较运算符优先级
   ❌ clutch_state_.GetValue() == FL || clutch_state_.GetValue() == RL && ...
   ✅ (clutch_state_.GetValue() == FL || clutch_state_.GetValue() == RL) && ...
   → || 优先级低于 &&，必须加括号

5. IsRecoverCondition 逻辑
   ❌ engine_speed ≤ threshold && clutch == NN （&&太严格）
   ✅ engine_speed ≤ threshold || clutch == NN  （||，任一条件满足即可恢复）

6. config_ 下划线一致性
   → 成员变量 config_ 带下划线，全文统一
```

#### tm_state.h 路径发现

```
问题：编译报 "tm_state.h not found"
原因：tm_state.h 不在 middleware/signal_object/ 里！
      它在 Application/1_14_0/signal_object/include/ 里

解决：build.bat 添加 Application 库的 include 路径
  set AP=.\..\..\..\Application\1_14_0
  加 -I%AP%\signal_object\include

教训：middleware 的 signal_object 有通用类型（RotationalSpeed、Pressure等）
      Application 的 signal_object 有项目专用类型（TmState = 变速箱状态枚举）
```

**TmState 类型定义**：
```cpp
namespace signal_object {
  struct EnumTmState {
    enum Type { NN, FL, FM, FH, FLFM_DIRECT, FMFH_DIRECT, RL, RM, RLRM_DIRECT };
  };
  typedef SignalObject<EnumTmState::Type> TmState;
}
```

### 35.2 RTOS 基础知识

#### Task vs ISR（中断服务程序）

```
Task（任务）：
├── 由 RTOS 调度器管理
├── 有优先级，可以被抢占
├── 可以等待/睡眠
├── 例：main_task（10ms）、fast_task（1-5ms）、slow_task（100ms）

ISR（Interrupt Service Routine = 中断服务程序）：
├── 由硬件中断直接触发（不经过调度器）
├── 优先级高于所有 Task
├── 必须极短（微秒级）— 不能等待/睡眠
├── 运行时所有 Task 被暂停
├── 例：CAN帧到达、ADC转换完成、定时器溢出
```

#### volatile 关键字

```
问题：编译器优化可能把变量缓存到寄存器
      如果 ISR 修改了内存中的变量 → Task 读的还是寄存器里的旧值

解决：volatile — 告诉编译器"不要缓存，每次都从内存读"

用法：volatile bool can_data_ready = false;  // ISR 会修改这个变量

规则：凡是 ISR 和 Task 共享的变量 → 必须加 volatile
```

#### 临界区（Critical Section）

```
问题：ISR 和 Task 同时访问同一个变量 → 数据竞争

解决方案1 — 关中断：
  disable_interrupts();     // 暂时关掉所有中断
  shared_data = new_value;  // 安全操作共享数据
  enable_interrupts();      // 恢复中断
  → 简单粗暴，但关中断期间会错过中断事件 → 必须极短

解决方案2 — Mutex（互斥锁）：
  mutex_lock(&data_mutex);    // 上锁
  shared_data = new_value;    // 安全操作
  mutex_unlock(&data_mutex);  // 解锁
  → 只阻塞竞争者，不影响其他中断/任务
```

#### 优先级反转（Priority Inversion）+ Priority Inheritance

```
问题场景：
  Low 优先级任务持有锁 → High 优先级任务等锁 → Medium 优先级任务抢占 Low
  → High 被 Medium 间接阻塞了！（High 等 Low 释放锁，Low 被 Medium 抢占）

解决：Priority Inheritance（优先级继承）
  Low 持有锁时，如果 High 在等 → RTOS 临时把 Low 提升到 High 的优先级
  → Medium 无法抢占 Low → Low 快速释放锁 → High 拿到锁继续执行
```

#### K37TM 的 Task 结构

```
main_task（10ms周期）：
├── 所有业务模块的 Update() 在这里执行
├── Manager 树形调用 → 遍历 400+ 模块
├── 这就是你平时分析的代码所在的任务

fast_task（1-5ms 周期）：
├── 传感器采样、ADC 读取
├── 需要更高频率的信号处理

slow_task（100ms 周期）：
├── 日志记录、诊断报告
├── 对实时性要求低的处理
```

### 35.3 CAN 中断机制

#### 上半部/下半部模式（Top-half / Bottom-half）

```
CAN帧到达时的完整流程：

① 硬件触发 CAN 接收中断
② ISR（上半部 / Top-half）—— 微秒级
   ├── 从 CAN 控制器的硬件寄存器复制 8 字节到内存缓冲区
   ├── 设置标志位 can_frame_ready = true
   └── 立即返回（不做任何解析）
③ Task（下半部 / Bottom-half）—— 在 main_task 的 10ms 周期里
   ├── 检查 can_frame_ready 标志
   ├── 从缓冲区读取 8 字节
   ├── 按 J1939 格式解析（DecodeData、resolution×原始值+offset）
   └── 写入 SignalObject（供下游模块使用）

为什么分两步？
├── ISR 必须快（微秒级）：长时间占用 ISR → 其他中断被延迟 → 系统不稳定
├── 解析/转换可以稍后做：放到 Task 里，在 10ms 周期内处理完就行
├── 类比：快递员（ISR）只管把包裹放门口 → 你（Task）有空了再拆包裹
```

#### ISR vs Task 的分工

```
ISR 做什么（必须极短）：
├── 复制数据到缓冲区
├── 设置标志位
├── 清除中断标志（告诉硬件"我处理了"）
└── ❌ 不做：解析、计算、信号转换、调用模块

Task 做什么（允许较长）：
├── 检查标志位
├── 从缓冲区读数据
├── J1939 解析（PGN、SPN、resolution 转换）
├── 更新 SignalObject
└── 调用下游模块的 Update()

K37TM 中你读过的 CAN 模块（如 FEF500、F00400）：
├── 它们的 Update() 在 main_task 里执行 = 下半部
├── GetFrame() 从缓冲区读数据 = 读 ISR 之前存好的数据
├── DecodeData() 做解析 = 下半部的核心工作
└── ISR 部分在 middleware/设备驱动层，你看不到源码
```

### 35.4 练习模块进度更新

| 领域 | 已写模块 | 数量 |
|------|---------|:---:|
| 之前练习 | parking_lamp, reverse_buzzer, fan_control 等 | 18 |
| 诊断(之前) | oil_temp_sensor, coolant_temp_sensor, fuel_pressure_sensor, vehicle_speed_sensor | 4 |
| **诊断(新)** | **mismatch_detection, clutch_slip_fault_judge_model** | **2** |
| **合计** | | **26** |

### 35.5 今日学习检查清单

```
- [x] clutch_slip_fault_judge_model 从零编写 → 编译通过 ✅
- [x] 发现 tm_state.h 在 Application 库（不在 middleware）→ build.bat 加路径
- [x] 理解 RTOS 基础：Task vs ISR、volatile、临界区、Mutex
- [x] 理解优先级反转 + Priority Inheritance 解决方案
- [x] 理解 K37TM 的三层 Task 结构（fast/main/slow）
- [x] 理解 CAN 中断的上半部/下半部模式
- [x] 理解 ISR 和 Task 的分工（ISR=搬数据，Task=解析数据）
- [x] 关联到已读过的 CAN 模块（FEF500/F00400 的 Update() = 下半部）
```



---

## 三十六、Mini Control Sim — 个人作品集项目（5/22）

> ChatGPT 建议将练习代码重构为独立的、可公开的个人项目。Copilot 协助完成了从零搭建到编译运行的全过程。

### 36.1 项目动机

ChatGPT 分析了我的练习代码（brake_oil_warning、clutch_slip_fault_judge_model 等），指出：
- 代码质量不错，模式清晰（Config、const&输入、Manager注册、Update周期执行、状态机）
- **最大问题**：太像公司内部代码，不能原样公开
- **解决方案**：保留思想，完全脱敏，做成独立可编译的个人项目

### 36.2 脱敏命名对照

| 公司内部名 | 脱敏后名称 | 说明 |
|-----------|-----------|------|
| brake_oil_warning | HydraulicOilWarning | 模块名脱敏 |
| clutch_slip_fault_judge_model | ClutchSlipDetector | 模块名脱敏 |
| signal_object::Temperature | signals::TemperatureSignal | 信号类脱敏 |
| signal_object::ElectricalOnOffState | signals::OnOffSignal | 信号类脱敏 |
| signal_object::EnumSignalState | signals::SignalValidity | 枚举脱敏 |
| framework::Manager.RegistModule | framework::Manager.RegisterModule | 方法名修正 |
| utility::HysteresisFloat32 | utility::Hysteresis | 工具类脱敏 |
| utility::IncrementTimer | utility::IncrementTimer | 保留（通用名） |
| fault_state_detection_model::Function | modules::FaultDetectorBase | 基类脱敏 |
| EnumTmState::FL/RL/NN | ClutchState::CLUTCH_FORWARD/REVERSE/NEUTRAL | 枚举脱敏 |
| float32 | float | 去掉项目专用typedef |
| kcommon.h | 不再需要 | 用标准C++11 |

### 36.3 项目结构

```
mini_control_sim/
├── CMakeLists.txt              ← CMake编译配置
├── README.md                   ← 英文说明（含脱敏声明）
└── src/
    ├── main.cpp                ← 两个测试场景（油温报警 + 离合器打滑）
    ├── framework/
    │   ├── module_interface.h  ← ModuleInterface（纯虚函数Update）
    │   └── manager.h           ← Manager（收集模块，调UpdateAll）
    ├── signals/
    │   └── signal.h            ← Signal<T>模板（value + validity）
    ├── utility/
    │   ├── increment_timer.h   ← 周期累加定时器
    │   └── hysteresis.h        ← 迟滞比较器
    └── modules/
        ├── hydraulic_oil_warning.h/.cc   ← 油温报警（两相状态机）
        ├── fault_detector_base.h/.cc     ← 故障检测基类（Template Method）
        └── clutch_slip_detector.h/.cc    ← 离合器打滑检测（继承基类）
```

### 36.4 与原代码的关键改进

#### 改进1：状态机输出时序（ChatGPT指出）

```
原代码（brake_oil_warning.cc）：
  switch (status_) {
    case state_off:
      output = OFF;          ← 先设output
      if (条件满足) {
        status_ = state_on;  ← 再改status
      }                      → 本周期output和status不一致！
  }

改进后（hydraulic_oil_warning.cc）：
  switch (status_) {
    case STATE_OFF:
      if (条件满足) {
        status_ = STATE_ON;  ← 先更新status
      }
  }
  output = (status_ == STATE_ON) ? ON : OFF;  ← 再统一决定output
  → 状态和输出本周期同步！
```

#### 改进2：枚举命名风格

```
原代码：enum STATUS { state_on, state_off };    ← 容易和变量混淆
改进后：enum Status { STATE_OFF, STATE_ON };    ← 大写前缀更醒目
```

#### 改进3：阈值命名（ChatGPT指出注释和逻辑矛盾）

```
原代码：
  float32 engine_speed_threshold;       // 注释写"上限"但实际是判断"大于"
  float32 tm_output_speed_threshold;    // 注释写"下限"但实际是判断"小于"

改进后：
  float engine_speed_high_threshold;    // 明确：engine_speed > 此值 = 异常
  float output_speed_low_threshold;     // 明确：output_speed < 此值 = 异常
```

#### 改进4：FaultDetectorBase 增加确认/恢复定时器

```
原代码（fault_state_detection_model）：
  依赖中间件的 FailureDetector 组件做防抖

改进后（FaultDetectorBase）：
  自己实现 confirm_timer 和 recover_timer
  → 故障条件持续N周期才确认故障
  → 恢复条件持续N周期才确认恢复
  → 完全独立，不依赖任何外部库
```

### 36.5 编译环境

| 项目 | 公司项目 | 个人项目 |
|------|---------|---------|
| 编译器 | cxrh850（GHS，C++03） | MSVC 19.44（VS2022，C++11） |
| 目标平台 | RH850 MCU | Windows PC |
| 构建工具 | Makefile + Shell脚本 | CMake 3.10 |
| 输出 | .mot2 烧写文件 | .exe 可执行文件 |

### 36.6 编译运行方法

```bash
cd mini_control_sim
mkdir build && cd build
cmake ..
cmake --build . --config Release
.\Release\mini_control_sim.exe
```

### 36.7 测试场景输出

```
Test 1: Hydraulic Oil Temperature Warning
Cycle   OilTemp   Diag    Warning   Description
0       80        OK      OFF       Normal temperature
2       105       OK      ON        Reaches high threshold -> ON
5       93        OK      OFF       Temp below low threshold, timer done
8       108       NG      OFF       Diagnosis abnormal -> OFF

Test 2: Clutch Slip Fault Detection
Cycle   EngSpd    OutSpd    Clutch    Fault     Description
2       2500      150       F         OK        Slip condition (1/2)
3       2500      100       F         FAULT     Confirmed after 2 cycles
6       1200      400       F         OK        Recovered after 2 cycles
```

### 36.8 这个项目能证明什么 / 不能证明什么

**能证明**：
- 理解模块化控制软件结构（信号、状态机、定时器、迟滞、故障检测）
- 能写 C++ 代码（不是只会测试）
- 理解 Template Method 设计模式（FaultDetectorBase + ClutchSlipDetector）
- 有主动学习意识

**不能证明**：
- CAN通信、RTOS调度、寄存器操作、中断处理
- 真实硬件上的开发经验

**补硬件的最低成本方案**：
- 买一块 STM32 或 Arduino 开发板（30-80元）
- 把控制逻辑移植过去（Manager.UpdateAll()放到定时器中断里）
- ADC读模拟传感器，GPIO控制LED
- 控制逻辑代码几乎不用改 — 只改 main.cpp 的输入输出部分

### 36.9 简历/面试中的表述

```
Personal Project: Embedded Control Logic Simulation in C++

Built a small C++ practice project to simulate module-based 
control logic, including signal objects, timer-based debounce 
logic, warning lamp control, and fault condition detection. 
The project was created for self-study and does not contain 
any proprietary code or company-specific information.
```

### 36.10 下一步

- [ ] 逐个文件深入理解每一行代码（从module_interface.h开始）
- [ ] 考虑移植到 STM32/Arduino（真实硬件）
- [ ] 放到 GitHub 公开仓库
- [ ] 加更多测试场景



---

## 三十七、transmission_oil_temp_warning 实战 + sim项目独立化（5/22）

> 从零编写了 transmission_oil_temp_warning 模块（K37TM版 + sim版），首次同时使用 Hysteresis + Timer，深入理解了两种定时器设计策略。

### 37.1 transmission_oil_temp_warning 模块概述

**功能**：变速箱油温过高时亮警告灯，带防闪烁保护。

**输入**：
- `tm_oil_temp_`（Temperature）— 变速箱油温
- `is_diag_normal_`（BoolSignal）— 诊断是否正常

**输出**：
- `warning_lamp_output_`（ElectricalOnOffState）— 警告灯 ON/OFF

**核心工具组合**（首次两个同时使用）：

| 工具 | 功能 | 防闪烁维度 |
|------|------|-----------|
| **Hysteresis** | 温度阈值迟滞（如120°C ON / 110°C OFF） | **幅度**上的波动 |
| **IncrementTimer** | 确认延迟（持续N周期才触发） | **时间**上的波动 |

### 37.2 最终代码（K37TM版）

```cpp
void Function::Update() {
    hyst_output_.Update(tm_oil_temp_.GetValue());
    timer_output_.Update();

    switch (status_) {
    case STATE_OFF:
        output_ = OFF;
        if (is_diag_normal_.GetValue() && hyst_output_.GetState() && timer_output_.IsTimeUp()) {
            output_ = ON;
            status_ = STATE_ON;
            timer_output_.Clear();
        } else if (!hyst_output_.GetState()) {  // 降温→清零，否则下次超温马上报警
            timer_output_.Clear();
        }
        break;

    case STATE_ON:
        output_ = ON;
        if (!is_diag_normal_.GetValue() || (!hyst_output_.GetState() && timer_output_.IsTimeUp())) {
            output_ = OFF;
            status_ = STATE_OFF;
            timer_output_.Clear();
        } else if (hyst_output_.GetState()) {   // 温度还高→timer归零，不开始倒计时
            timer_output_.Clear();
        }
        break;
    }

    warning_lamp_output_ = ElectricalOnOffState(output_, VALID);
}
```

### 37.3 两种定时器设计策略（重要设计决策）

#### 设计A：最短显示时间（不推荐）

```
timer 从进入 STATE_ON 时 Clear → 一直跑
→ 亮灯超过N周期后 IsTimeUp()=true
→ 温度一降 hyst=false → 瞬间满足 !hyst && IsTimeUp() → 立刻灭灯
→ timer 只在"刚亮灯的前N周期"有保护作用，之后失效
```

**问题**：亮灯时间长了以后，降温就立刻灭灯，没有确认延迟，跟没加 timer 一样。

#### 设计B：关灯确认延迟（最终采用）

```
STATE_ON 中，hyst=true 时持续 Clear() → timer 保持为0
→ 温度降了(hyst=false) → timer 才开始计数
→ 持续N周期低温 → IsTimeUp()=true → 灭灯
```

**关键代码差异**：
```cpp
// 设计B 在 STATE_ON 中加了：
else if (hyst_output_.GetState()) {
    timer_output_.Clear();  // 温度还高 → timer归零
}
```

**效果对比**：

```
                Design A (旧)          Design B (新)
Cycle  9:       ON  STATE_ON           ON  STATE_ON      ← 温度刚降
Cycle 10:       ON  STATE_ON           ON  STATE_ON
Cycle 11:       OFF STATE_OFF ← 灭了   ON  STATE_ON  ← 还亮
Cycle 12:       OFF STATE_OFF          ON  STATE_ON  ← 还亮
Cycle 13:       OFF STATE_OFF          OFF STATE_OFF ← 确认5周期后才灭
```

**设计B的含义**：温度要**降到低阈值以下并持续N周期**才灭灯。每次降温都完整等待，防闪烁更彻底。

### 37.4 Hysteresis 的迟滞死区详解

```
                    GetState() = false          GetState() = true
                   ◄──────────────────┐    ┌──────────────────►
  温度上升 ──────────────────────────────────────────────────►
        80    90    100   110   120   130
                          │           │
                     low_threshold   high_threshold
                       (110)          (120)

关键：110~120之间是死区 → GetState() 不变
温度从80升到115：false（没到120）
温度升到120：    → true
温度降到115：    还是 true（没到110）
温度降到108：    → false
```

### 37.5 STATE_OFF 和 STATE_ON 的对称设计

| | STATE_OFF → ON | STATE_ON → OFF |
|---|---|---|
| **条件** | 诊断正常 + 温度超阈值 + 持续N周期 | 诊断异常（立刻）或 温度降了 + 持续N周期 |
| **timer Clear 时机** | 温度没超阈值时 Clear | 温度还高时 Clear |
| **设计意图** | 升温确认延迟 | 降温确认延迟 |

### 37.6 创建 tm_oil_temp_warning_sim（独立可运行项目）

将 K37TM 版本移植到独立的 sim 项目中，与 mini_control_sim 相同的结构。

**项目结构**：
```
tm_oil_temp_warning_sim/
├── CMakeLists.txt
├── build/
└── src/
    ├── main.cpp                              ← 24个测试场景
    ├── framework/
    │   ├── module_interface.h
    │   └── manager.h
    ├── signals/
    │   └── signal.h
    ├── utility/
    │   ├── hysteresis.h
    │   └── increment_timer.h
    └── modules/
        ├── transmission_oil_temp_warning.h
        └── transmission_oil_temp_warning.cc
```

**编译运行（cmake不在PATH时）**：
```powershell
$cmake = "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"

# 首次（生成项目）：
cd tm_oil_temp_warning_sim
mkdir build; cd build
& $cmake .. -G "Visual Studio 17 2022" -A x64

# 编译：
& $cmake --build . --config Release

# 运行：
.\Release\tm_oil_temp_warning_sim.exe
```

**注意**：PowerShell 中直接写 `cmake` 会报 CommandNotFoundException。必须用 `& "完整路径"` 或先定义变量 `$cmake`。

**可移植性**：只需 Visual Studio 2022 + C++ 桌面开发组件，不依赖 K37TM 环境。

### 37.7 sim 项目的工作流

以后写新模块的标准流程：
1. 在 `modules/` 下写 `.h` 和 `.cc`（用 sim 的 signals/utility 类型）
2. 更新 `CMakeLists.txt` 加入新 `.cc`
3. 在 `main.cpp` 写测试场景
4. 编译运行 → 看每个周期的输入输出，验证逻辑

### 37.8 练习模块进度更新

| 领域 | 已写模块 | 数量 |
|------|---------|:---:|
| 之前练习 | parking_lamp, reverse_buzzer, fan_control 等 | 18 |
| 诊断 | oil_temp_sensor, coolant_temp_sensor 等 | 6 |
| **业务（新）** | **transmission_oil_temp_warning（K37TM版 + sim版）** | **1** |
| **合计** | | **27** |


### 37.9 下周计划（5/26~）

- [ ] 下游追踪：`tm_oil_temperature_model` 的输出被谁消费（搜 `tm_oil_temperature_model.`）
- [ ] 上游追踪：`diag_normal_judge_function_40d3027a` 的输入从哪来
- [ ] 画一条完整闭环：传感器 → 业务模块 → 诊断 → 故障报告
- [ ] 新模块练习（第28个）：仿写 `oil_temperature_model` 或尝试 OR-merge + mask 模式
- [ ] sim 项目扩展：加边界测试 or 把 MaskOrFunction 加进 sim



---

## 三十八、信号追踪实战 — tm_oil_temp 完整上游链路（5/22）

> 第一次在真实代码库中做信号追踪（upstream tracing），从业务模块出发，一直追到物理传感器和故障屏蔽条件。

### 38.1 追踪目标

从 `tm_oil_temperature_model`（变速箱油温业务模块）出发，追踪其所有输入信号的来源。

### 38.2 追踪过程（Step by Step）

**Step 1：找 tm_oil_temperature_model 的 instantiation**

```cpp
// instantiation_app_userland.h 第 27969 行
INSTANTIATION(
    (logic::wl::vehicle_observer::oil_temperature_model::Function),
    tm_oil_temperature_model,
    /* config : */                    tm_oil_temperature_model_config,
    /* engine_status : */             tm1_receiver_j1939_ch2_0x00FF2900.EngineStatusRef(),
    /* is_count_prohibit_diag_normal : */ diag_normal_judge_function_40d3027a.IsDiagNormalRef(),
    /* temperature : */               tm_oil_temp.TemperatureRef(),
    /* oil_temp_frequency_mdo : */    array_mdo_inavlid_treat_function_b43f663b,
    /* manager : */                   main_app_manager_node01_28);
```

**发现**：同一个 `oil_temperature_model::Function` 类被实例化了 3 次（hst/tm/axle），配置不同代码相同 — 配置分离的威力。

**Step 2：追踪 temperature 来源 → tm_oil_temp**

```cpp
// 第 9666 行
INSTANTIATION((device::TemperatureSensorByAnalogInput), tm_oil_temp,
    /* config : */                  tm_oil_temp_config,
    /* buffer : */                  tm_oil_temp_buffer,        // 16个校准点的插值表
    /* low_voltage_fault_mask : */  function_db982ca5.OutputRef(),
    /* high_voltage_fault_mask : */ function_db982ca5.OutputRef(),
    /* manager : */                 main_app_manager_node00_20);
```

**发现**：tm_oil_temp 不只读温度，还有高/低电压自检。fault_mask 控制是否执行故障检查。

**Step 3：追踪 fault_mask → function_db982ca5**

```cpp
// 第 27667 行 — so_buffer_model（缓冲一个周期）
INSTANTIATION((so_buffer_model::Function<FaultTransitionMaskOrder>),
    function_db982ca5,
    /* initial_input : */ initial_mask_value,
    /* input : */         mask_or_function_17cf2b8a.ResultRef(),
    /* manager : */       main_app_manager_node01_26);
```

**Step 4：追踪 → mask_or_function_17cf2b8a**

```cpp
// 第 22842 行 — MaskOrFunction（OR 合并）
INSTANTIATION((diagnosis_check_model::MaskOrFunction),
    mask_or_function_17cf2b8a,
    /* request : */ fault_transition_mask_order_45107544,
    /* manager : */ main_app_manager_node01_19);
```

**Step 5：追踪 → fault_transition_mask_order_45107544（到底了！）**

```cpp
// 第 21275 行 — 3 个屏蔽条件
fault_transition_mask_order_45107544_body[] = {
    &dg_regular_mask.WaitForSystemStableFaultMaskRef(),  // ① 系统初始化中
    &dg_regular_mask.LowVoltageBattFaultMaskRef(),       // ② 电池低电压
    &dg_engine_stop_at_key_off_mask.FaultMaskRef()       // ③ 发动机已熄火
};
```

### 38.3 完整信号链路图

```
Layer 5（屏蔽条件定义）
  fault_transition_mask_order_45107544
  ┌─ ① WaitForSystemStable（系统初始化中 → 传感器没稳定）
  ├─ ② LowVoltageBatt（电池低电压 → ADC读数不可靠）
  └─ ③ EngineStopAtKeyOff（发动机熄火 → 传感器断电）
           │
           ↓ 作为 request 数组传入
Layer 4（OR 合并）
  mask_or_function_17cf2b8a (MaskOrFunction)
  → 遍历数组，任一为 ON → 输出 ON
           │
           ↓ .ResultRef()
Layer 3（缓冲同步）
  function_db982ca5 (so_buffer_model)
  → 延迟一个周期（同步用）
           │
           ↓ .OutputRef()
Layer 2（设备驱动 — 传感器）
  tm_oil_temp (TemperatureSensorByAnalogInput)
  ├─ low_voltage_fault_mask  ← mask 输入
  ├─ high_voltage_fault_mask ← mask 输入（同一个）
  ├─ ADC采样 → 插值表(16点) → 温度°C
  └─ .TemperatureRef()
           │
           ↓
Layer 1（业务逻辑）
  tm_oil_temperature_model (oil_temperature_model)
  ├─ temperature ← tm_oil_temp.TemperatureRef()
  ├─ engine_status ← CAN J1939 (0x00FF2900)
  ├─ is_diag_normal ← diag_normal_judge
  └─ → 油温过高 → 警告灯
```

### 38.4 MaskOrFunction 源码解读

```cpp
void MaskOrFunction::Update() {
    Type result = OFF;                              // 默认不屏蔽
    for (i = 0; i < request_.GetSize(); ++i) {      // 遍历所有条件
        if (request_(i)->GetValue() == ON) {         // 任一为 ON
            result = ON;                             // → 屏蔽
        }
    }
    result_ = FaultTransitionMaskOrder(result, VALID);
}
```

**关键细节**：
- `request_(i)` 是**指针数组**的访问方式（Array 的 `operator()`），返回指针
- `->GetValue()` 是通过指针调用 SignalObject 的取值方法
- 没有 `break` — 简单写法，3 个元素无性能问题
- OR 逻辑：默认 OFF，有一个 ON 就变 ON

### 38.5 屏蔽 vs 触发（容易混淆的概念）

```
fault_mask = true（ON）时：
  → 屏蔽（suppress）传感器的故障诊断
  → 不检查高/低电压 → 不报故障
  → 温度值本身照常输出（mask 不影响 .TemperatureRef()）

fault_mask = false（OFF）时：
  → 正常检查传感器电压范围
  → 电压异常 → 报故障

含义："不要在不可靠的条件下做诊断"
```

### 38.6 信号追踪方法论总结

| 步骤 | 搜索方法 | 目的 |
|:---:|---------|------|
| 1 | 搜 `module_name,`（带逗号） | 找到目标模块的 INSTANTIATION |
| 2 | 读参数注释 `/* param : */` | 知道每个输入来自哪里 |
| 3 | 搜 `source_name,`（带逗号） | 找到上游模块的 INSTANTIATION |
| 4 | 重复 2-3 | 直到到达硬件层或数据定义 |

**搜索技巧**：
- `name,`（逗号）→ 精确定位 INSTANTIATION 定义
- `name.`（点号）→ 找下游消费者
- 数据数组（如 `fault_transition_mask_order_xxx_body[]`）就是链路终点



---

## 三十九、Signal Tracing 进阶 — diag_normal_judge 输入链 + CAN 输出链（5/25）

> 独立完成了两条信号追踪：(1) 自己画图追踪 diag_normal_judge 的输入链并验证正确，(2) 追踪了 tm_oil_temp 到 CAN 总线的输出链。

### 39.1 diag_normal_judge_function_40d3027a 输入链（独立完成 ✓）

**自己画的图，验证全部正确。**

`diag_normal_judge` 接收 `failure_signal_0c599085`（2个 FailureSignal 的数组），来自两条分支：

```
            左分支（传感器内置检查）              右分支（外部诊断模块）
            ─────────────────                ───────────────────

fault_transition_mask_order         fault_transition_mask_order
     _45107544_body[] (3个)              _701dd350_body[] (2个)
┌─ WaitForSystemStable              ┌─ mask_or_function_e349568a.ResultRef()
├─ LowVoltageBatt                   └─ dg_tm_oil_temperature_sensor_fault_mask
└─ EngineStopAtKeyOff                     .FaultMaskRef()
         ↓                                    ↓
mask_or_function_17cf2b8a           mask_or_function_08e862fb
         ↓                                    ↓
function_db982ca5 (buffer)          dg_tm_oil_temperature_sensor
         ↓                           _open_or_hots_fault_judgement
tm_oil_temp                              ├─ fault_mask ← mask_or上面的
  └─ .LowVoltageFailureRef()             └─ voltage_input ← tm_oil_temp.RawVoltageRef()
         ↓                                    ↓ .FailureSignalRef()
         └──────────┐    ┌─────────────────────┘
                    ↓    ↓
          failure_signal_0c599085_body[] (2个)
          ├─ [0] tm_oil_temp.LowVoltageFailureRef()
          └─ [1] dg_tm_oil_...fault_judgement.FailureSignalRef()
                    ↓
          diag_normal_judge_function_40d3027a
          "两个故障源都正常 → IsDiagNormal = true"
```

**两种故障源的含义**：

| 故障源 | 检查什么 | 由谁做 |
|--------|---------|-------|
| `tm_oil_temp.LowVoltageFailureRef()` | 传感器电压太低（短路到地） | 传感器模块内置 |
| `dg_tm_oil_...fault_judgement.FailureSignalRef()` | 传感器电压太高（开路或短路到电源） | 外部诊断模块 |

### 39.2 CAN 输出链（tm_oil_temp → CAN 总线）

**独立追踪完成。** 这是一条只有3层的短链：

```
Layer 1：tm_oil_temp (TemperatureSensorByAnalogInput)
  └─ .TemperatureRef()
           ↓
Layer 2：status_overwrite_function_ad393fa7 (StatusOverwriteFunction<Temperature>)
  ├─ input:  tm_oil_temp.TemperatureRef()              ← 温度值
  └─ status: diag_normal_judge_function_7c23dc7b.IsDiagNormalRef()  ← 诊断状态
           ↓ .OutputRef()（诊断异常时温度的 status 被覆盖为"无效"）
Layer 3：tm1_sender_rtcdbrx_0x000_100ms (CAN 发送模块)
  └─ /* tm_oil_temp : */ → RTCDB 总线，100ms 周期发送
           ↑ 系统边界，到头了
```

### 39.3 CAN 发送模块解读 (Tm1SenderRtcdbrx0x000100ms)

**文件**：
- .h: `system/network/include/tm1_sender_rtcdbrx_0x000_100ms.h`
- .cc: `system/KDOG2/tm1_sender_rtcdbrx_0x000_100ms.cc`

**特征**：KDOG2 自动生成的代码。

**核心逻辑**：
```cpp
void OutputUpdate() {
    uint8 buffer[28];                          // 28字节 CAN 帧
    memset(buffer, 0x00, sizeof(buffer));       // 清零

    // 对每个信号编码 Data + Status
    EncodeData<float32, sint16>(buffer, config, tm_oil_temp_.GetValue());
    EncodeStatus<float32, sint16>(buffer, config, tm_oil_temp_.GetState());
    // ...（共15个信号）

    proxy_.SetFrame(buffer, sizeof(buffer));    // 发送
}
```

**15个信号打包在一帧中**：
hydraulc_oil_temp, hst_oil_temp, **tm_oil_temp**, target_vehicle_speed,
brake_stroke, body_pitch_angle, body_roll_angle, fan_indicator,
right_fnr_indicator, fan_revolution, ecss_enable_speed, pdc_network_failure,
creep_speed_level_forward, creep_speed_level_reverse, is_acc_high_press

**关键设计**：
- `Update()` = do nothing（不做计算）
- `OutputUpdate()` = 真正干活（编码 + 发送）
- 每个信号编码**两部分**：Data（值）+ Status（正常/无效）
- `rtcdbrx_config` 决定每个信号在 buffer 中的偏移和位宽

### 39.4 StatusOverwriteFunction 的作用

```
正常时：
  tm_oil_temp status=VALID → StatusOverwrite → 原样传递 → CAN发送

诊断异常时：
  IsDiagNormal=false → StatusOverwrite → status 覆盖为 INVALID → CAN发送
  → 接收方看到 INVALID → 知道这个温度值不可信，不拿来做判断
```

**设计思想**：传感器故障时不是发错误的温度值，而是标记"无效"让接收方自己处理。

### 39.5 Signal Tracing 系统边界总结

**上游终点（信号从哪来）**：

| 终点类型 | 特征 | 例子 |
|---------|------|------|
| 物理传感器 | ADC读硬件电压 | `tm_oil_temp` |
| CAN/网络输入 | 从总线接收报文 | `engine_status`(J1939) |
| MDO读取 | 从非易失存储读 | `mdo.GetValue()` |
| 编译时常量 | Config结构体 | `*_config` |

**下游终点（信号到哪去）**：

| 终点类型 | 特征 | 例子 |
|---------|------|------|
| MDO写入 | 写非易失存储 | `tm_oil_temperature_model` |
| CAN发送 | 发报文到总线 | `tm1_sender_rtcdbrx_0x000_100ms` |
| 没有OutputRef消费者 | 搜`.`找不到 | data sink 模块 |

**判断方法**：
- 上游追：参数是 `xxx.SomeRef()` → 继续追 xxx；参数是 config/manager/MDO → 到头了
- 下游追：搜 `module.` 找到消费者 → 继续追；找不到 → 到头了

### 39.6 分支处理策略

信号链出现分支是常态，处理方法：
1. **记下来，挑一条先追到底**（深度优先）
2. **判断值不值得追**：主信号 → 追；mask条件看名字就知道是什么 → 通常不深追；config/manager → 不追
3. **每条分支追到系统边界就停**

### 39.7 练习模块进度更新

| 领域 | 已写模块 | 数量 |
|------|---------|:---:|
| 练习模块（parking_lamp 等） | 18 |
| 诊断模块 | 6 |
| Signal Tracing 实践 | 2条完整链路 |
| **总模块数** | **27** |

### 39.8 今日学习检查清单

```
- [x] 独立画出 diag_normal_judge 输入链并验证正确 ✅
- [x] 发现两种故障源设计（内置 LowVoltage + 外部 OpenOrHots）
- [x] 独立追踪 CAN 输出链（tm_oil_temp → StatusOverwrite → Sender）✅
- [x] 阅读 CAN 发送模块源码（.h + .cc），理解 EncodeData/EncodeStatus
- [x] 理解 StatusOverwriteFunction 的作用（诊断异常→标记无效）
- [x] 确认 CAN 发送模块是系统边界（data sink，无下游消费者）
- [x] 总结 signal tracing 系统边界判断方法
- [x] 总结分支处理策略（记下来→挑一条→不重要的看名字停）
```



---

## 四十、练习模块 #28-29：sensor_value_validator + multi_level_oil_temp_protection（5/25）

> 一口气写了两个模块，难度递进。第一个简单（信号状态覆盖），第二个是迄今最综合的模块（三状态机 + 双Hysteresis + 双Timer + 双输出 + 诊断安全）。

### 40.1 模块28：sensor_value_validator

**文件**：`MyEmbeddedPractice/BusinessModules/sensor_value_validator/`

**功能**：验证传感器值——诊断正常时透传，诊断异常时输出默认值 + INVALID 状态。

**模仿对象**：K37TM 的 `StatusOverwriteFunction`（`so_convert_model_status_overwrite_function.h`）

**与原版的区别**：
| | StatusOverwriteFunction | sensor_value_validator |
|---|---|---|
| 诊断异常时 | 值不变，只改 status → INVALID | 值替换为 default_value + INVALID |
| Config | 无 | 有（default_value） |
| 模板 | template<SoType> | 固定为 Temperature |

**核心逻辑**：
```cpp
void Update() {
    if (is_diag_normal_.GetValue()) {
        output_ = Temperature(sensor_temp_.GetValue(), VALID);
    } else {
        output_ = Temperature(config_.default_value, INVALID);
    }
}
```

**编写过程中的错误**：
1. 输出变量声明为 `const Temperature&` → 应为值类型 `Temperature`（要赋值就不能是引用）
2. 诊断异常时 status 写成 VALID → 应为 INVALID
3. `.cc` 的 `OutputRef()` 忘了 `const`
4. 类定义 `}` 后忘了 `;`

### 40.2 模块29：multi_level_oil_temp_protection ⭐⭐⭐ 最综合

**文件**：`MyEmbeddedPractice/BusinessModules/multi_level_oil_temp_protection/`

**功能**：三级油温保护——NORMAL → WARNING（亮警告灯） → CRITICAL（限制功率）

**状态迁移图**：
```
                  warning阈值+timer        critical阈值+timer
    NORMAL ──────────────→ WARNING ──────────────→ CRITICAL
      ↑                      │                        │
      └──(!warning+timer)────┘                        │
                             ↑                        │
                             └──(!critical+timer)─────┘
      ↑                                               │
      └────────────(诊断异常 → 强制回NORMAL)───────────┘
```

**综合运用的模式**：

| 模式 | 用法 |
|------|------|
| 三状态状态机 | NORMAL / WARNING / CRITICAL |
| 双 Hysteresis | warning_threshold_ + critical_threshold_ |
| 双 Timer | confirm_time_（升级确认） + recover_time_（恢复确认） |
| 双输出 | warning_lamp + power_limit |
| 诊断安全 | is_diag_normal = false → 强制回 NORMAL + 清 timer |
| 设计B timer策略 | 温度还高时持续 Clear → 降温后才开始计时 |

**Config 结构**：
```cpp
struct Config {
    HysteresisFloat32::Config warning_threshold_config;
    HysteresisFloat32::Config critical_threshold_config;
    IncrementTimer::Config confirm_time_config;
    IncrementTimer::Config recover_time_config;
};
```

### 40.3 Timer Clear 分析（本模块最大收获）

**分析方法**：问两个问题 →
1. 这个 timer 在下一个状态里有没有人检查？
2. 离开那个状态时会不会被清？

**结论**：状态转换时清 timer，只需要在下一个状态**开始使用之前**的某个路径上清就够了。多清不错，少清出 bug。

**具体分析结果**：
- NORMAL→WARNING 时清 confirm + recover → ✅ 都需要（WARNING 两个都用）
- WARNING→NORMAL 时清 confirm → ✅ 需要（NORMAL 用 confirm 确认升级）
- WARNING→NORMAL 时清 recover → 多余（NORMAL→WARNING 转换时会清）
- WARNING→CRITICAL 时清 confirm → 多余（CRITICAL 不检查 confirm）
- CRITICAL→WARNING 时清 confirm + recover → ✅ 都需要

### 40.4 安全设计思考：NORMAL→CRITICAL 直跳？

**结论：油温不需要。**

原因：油的热惯性大（几十秒~分钟才升到危险温度），confirm_time 总延迟（~1秒）远小于物理变化速度。

**需要直跳的场景**：电流/电压等毫秒级变化的物理量。这些通常用硬件中断直接保护，不走软件状态机。

**面试加分点**：主动思考"万一来不及怎么办" = 安全关键系统思维。

### 40.5 练习模块进度更新

| 领域 | 已写模块 | 数量 |
|------|---------|:---:|
| 练习模块（parking_lamp 等） | 18 |
| 诊断模块 | 6 |
| **信号处理（新）** | **sensor_value_validator** | **1** |
| **多级保护（新）** | **multi_level_oil_temp_protection** | **1** |
| Signal Tracing | 3条完整链路 |
| **总模块数** | **29** |
