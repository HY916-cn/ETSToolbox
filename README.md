# ETSToolbox

ETSToolbox 是一个面向 Windows 版 E听说客户端的实验性扩展。它通过 `winmm.dll` 代理加载，在客户端内置的 CEF 页面中注入配套前端脚本。

此维护版本修复了 1.0.2 发布包误用 Debug CRT 的问题，并补充了可复现的 x86 Release 构建、依赖检查和备份替换流程。

## 项目状态

- 已在 x86 `ETSShell.exe` 上完成 Release 构建与启动验证。
- 生成的 DLL 使用 MSVC Release CRT（`/MD`），不再依赖 `MSVCP140D.dll`、`VCRUNTIME140D.dll` 或 `ucrtbased.dll`。
- 主页面、F1 设置面板和 F12 开发者工具可以正常打开。
- 上游项目已停止维护，客户端更新后部分功能可能失效。
- “显示答案”目前只有界面占位，没有答案提取实现；“最大误差”配置也未接入实际逻辑。

配套前端源码位于 [Howie114514/ETSToolbox-js](https://github.com/Howie114514/ETSToolbox-js)。

## 安装

1. 关闭 E听说，并确认任务管理器中没有 `ETSShell.exe`。
2. 从维护仓库的 [Releases](https://github.com/HY916-cn/ETSToolbox/releases) 下载 Windows x86 修复包。
3. 备份 E听说安装目录中原有的 `winmm.dll`。
4. 将压缩包内的 `winmm.dll` 和 `etstoolbox` 文件夹完整解压到 E听说安装目录。默认目录通常为：

   ```text
   C:\Program Files (x86)\ETS
   ```

5. 手动启动 E听说进行测试。弹出的控制台中应显示 CEF Hook 成功和本地服务启动信息。

快捷键：

- `F1`：打开设置面板。
- `F12`：打开客户端网页开发者工具。

不要从第三方网站下载单独的运行库 DLL，也不要把文件复制到 `System32` 或 `SysWOW64`。

## 从源码构建

准备以下环境：

- Visual Studio 2022 或更高版本，并安装“使用 C++ 的桌面开发”、MSVC x86/x64 工具、CMake 工具和 Windows SDK。
- 官方独立版 [vcpkg](https://github.com/microsoft/vcpkg)，并配置 `VCPKG_ROOT`。
- PowerShell 5.1 或更高版本。

克隆仓库并初始化子模块：

```powershell
git clone --recurse-submodules https://github.com/HY916-cn/ETSToolbox.git
Set-Location .\ETSToolbox
```

只构建和检查，不修改 E听说安装目录：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-release.ps1
```

确认报告无误后，可以在管理员 PowerShell 中执行备份和替换：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-release.ps1 -Deploy
```

脚本会检查 `ETSShell.exe` 的实际 PE 架构、安装 `detours:x86-windows`、构建 Release DLL，并使用 `dumpbin /dependents` 检查 Debug CRT 依赖。部署前还会为原 DLL 创建带时间戳的备份，完成后不会自动启动 E听说。

更完整的说明见 [WINDOWS-RELEASE.md](WINDOWS-RELEASE.md)。

## 使用说明

该项目依赖未公开的客户端实现细节，不能保证兼容所有 E听说版本。请先保留原文件备份，并仅在获得授权的测试环境中使用。使用过程中不要在截图、日志或 Issue 中公开姓名、学校等个人信息。

## 截图

![设置面板](assets/image.png)

![运行界面](assets/screenshot1.png)
