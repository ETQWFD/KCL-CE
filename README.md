# KCL2

KCL2 是一个基于 Plain Craft Launcher 2（PCL2）的独立第三方 Minecraft 启动器修改版。

- 产品名称：KCL2
- 修改维护：ETC Team
- 原作名称：Plain Craft Launcher / PCL
- 原作者：龙腾猫跃（LTCat）
- 上游仓库：<https://github.com/Hex-Dragon/PCL2>
- 当前仓库：<https://github.com/ETQWFD/KCL2>

## 状态

KCL2 保留 PCL2 的版本下载、加载器安装、整合包管理、版本隔离和微软正版登录工作流。开源分支原本通过 `RestrictMcDownload` 关闭 Minecraft 下载入口；KCL2 已将该开关恢复为可用状态。

当前工程是 Windows WPF / .NET Framework 4.8 应用，Windows EXE 由 GitHub Actions 的 Windows runner 构建。现有代码不能直接编译成 Java JAR 或原生 Linux ELF；Linux/JAR 需要单独进行跨平台界面移植，不能把 WPF 程序改名后伪装成跨平台产物。

## 为什么会有 KCL2

当常见启动器因为网络、维护或外部压力暂时不可用时，玩家仍需要一个可以进入本地游戏的备选入口。KCL2 的目标是在这种情况下延续版本下载、正版登录和本地实例管理能力。

KCL2 不运营 Minecraft，也不运营任何多人服务器。未成年人充值、服务器封禁、道具纠纷、支付退款或服务器管理规则，请联系对应服主、服务器管理组、支付渠道或 Microsoft/Mojang 官方支持；这些事项不属于启动器可处理范围。

## 公平启动保护

KCL2 在启动前执行静态公平性检查：

- 识别常见作弊客户端和作弊 Mod 的文件名、Mod 元数据与内部包名；
- 检查实例、全局 mods/coremods、版本 Classpath；
- 阻止未知 `-javaagent`、`-agentpath`、`-agentlib`、关闭校验等 JVM 注入参数；
- 检查 `JAVA_TOOL_OPTIONS`、`_JAVA_OPTIONS`、`JDK_JAVA_OPTIONS` 等 Java 注入环境变量；
- 命中后不会启动游戏，并把原因写入 `KCL\FairPlayBlock.txt`。

该保护不会下载或运行真实外挂，也不能承诺拦截所有驱动级、系统级或未知混淆注入。移除作弊 Mod/客户端和注入参数后即可再次启动。

## 首次运行

首次运行会显示第三方修改与来源声明。继续即表示你理解：

1. KCL2 是第三方修改版，不是原作者龙腾猫跃运营或代言的版本。
2. 仓库和关于页保留原作者、贡献者和 PCL 分发许可证信息。
3. KCL2 修改部分按现状提供，不提供额外担保。
4. Minecraft 账号、购买、服务器充值和服务器管理争议应通过 Microsoft/Mojang、支付渠道或对应服主处理。

程序目录中的启动器数据文件夹使用 `kcl`。游戏版本内为兼容现有 PCL 生态仍保留 PCL 元数据目录。

## 微软正版登录

KCL2 使用设备码授权流程，并且需要独立的 Azure 公共客户端 Application (client) ID：

- 可在首次正版登录时按弹窗输入并保存在本机；
- 或设置 `KCL2_MS_CLIENT_ID` 环境变量；
- CI 可在仓库 Secrets 中设置 `CLIENT_ID`。

不要复用来源不明的客户端 ID，也不要把客户端密钥放进启动器。设备码公共客户端通常不需要 client secret。

## 构建

```powershell
msbuild KCL2.sln /p:Configuration=Release /m
```

推送 main 分支后，`.github/workflows/build.yml` 会在 Windows runner 上构建 Release，并把完整便携包发布到：

<https://github.com/ETQWFD/KCL2/releases/download/continuous/KCL2-portable.zip>

压缩包内包含 `KCL2.exe` 及其同目录运行文件；单独拷贝 EXE 可能缺少依赖，不建议这样运行。

## 官网与后台

- 官网：`website/index.html`
- 使用手册：`website/manual.html`
- 未列表统计页：`website/dan.html`

统计页读取 GitHub Release 的公开下载次数。GitHub API 不提供去重后的真实“下载人数”，因此页面只展示下载次数，不虚构人数。

## 许可证与署名

本仓库根目录保留 `LICENCE`（PCL 分发有限许可）。请在再分发时继续保留原作者与贡献者署名。KCL2 不得被描述为龙腾猫跃官方版本，也不应移除 PCL 原始许可声明。
