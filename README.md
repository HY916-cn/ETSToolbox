# ETSToolbox

ETSToolbox 是一个面向 Windows 版 E听说客户端的实验性扩展。它通过 `winmm.dll` 代理加载，在客户端的听说练习页面中注入前端脚本。

## 功能

- 按百分比调整听说题目的最终得分，可设置每道题的随机偏移上限。
- 设置作业记录中的提交用时。
- 在独立终端中显示当前题目的参考答案。
- 自动开始录音，并在客户端允许的 2 秒后停止。
- 用户手动点击整套练习的“开始”后，可自动点击后续“下一步”，直到流程结束。

按 `F1` 打开设置面板。“自动录音”和“自动下一步”可以分别开启；两者同时开启时组成完整自动流程。按 `F12` 可打开客户端网页开发者工具查看实时状态日志。

## 适用范围

所有扩展功能只在已识别的听说练习页面生效。读写、单词、配音、主页面和结果页面不会执行改分、改用时、答案输出或自动点击。

本项目只用于个人、本地、离线学习测试。请勿在学校、机构、正式考试、正式作业、共享设备或其他公共环境中使用，也不要用它影响真实成绩或他人数据。

## 安装

1. 从本仓库的 [Releases](https://github.com/HY916-cn/ETSToolbox/releases) 下载最新的 Windows x86 完整包。
2. 关闭 E听说，确认任务管理器中没有 `ETSShell.exe`。
3. 把压缩包解压到单独目录，以管理员身份打开 PowerShell。
4. 进入解压后的目录并运行：

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File '.\install.ps1'
   ```

安装脚本会校验文件、备份原文件并复制新版本，但不会自动启动 E听说。默认安装目录是：

```text
C:\Program Files (x86)\ETS
```

安装完成后手动启动 E听说。启用“显示答案”后，可直接在 `F1` 面板点击“打开答案终端”。

不要从第三方网站下载单独 DLL，也不要修改 `System32` 或 `SysWOW64`。

## 使用

1. 启动 E听说，按 `F1` 打开面板。
2. 按需设置控分百分比、每题随机偏移上限、提交用时、答案显示、自动录音和自动下一步。偏移单位是百分点，例如基准 `80`、上限 `5` 表示每题在 `75%` 至 `85%` 内独立取值。开启“显示答案”后，点击同一行的“打开答案终端”。
3. 如果要自动完成听说流程，同时开启“自动录音”和“自动下一步”。
4. 进入练习后，由用户手动点击第一次“开始”。
5. 扩展会实时检测按钮状态：自动推进、开始录音、确认录音状态、等待 2 秒、停止录音，再继续推进。

如果按钮状态没有变化，扩展会按受控间隔重试。可在 `F12` 控制台中查看带有 `[ETSToolbox 自动流程]` 前缀的状态记录。

## 从源码构建

### 后端 DLL（Windows）

需要 Visual Studio C++、Windows SDK、CMake、PowerShell 和 vcpkg。克隆时要初始化子模块：

```powershell
git clone --recurse-submodules https://github.com/HY916-cn/ETSToolbox.git
Set-Location '.\ETSToolbox'
$env:VCPKG_ROOT = 'C:\File\build\vcpkg'
powershell -NoProfile -ExecutionPolicy Bypass -File '.\scripts\build-release.ps1' -VcpkgRoot $env:VCPKG_ROOT
```

构建脚本会读取 `ETSShell.exe` 的 PE 架构、安装匹配的 Detours、使用 MSVC Release CRT 构建，并检查 Debug CRT 依赖。确认报告后才可加 `-Deploy` 安装。

### 前端脚本（macOS、Linux 或 Windows）

```bash
git clone https://github.com/HY916-cn/ETSToolbox-js.git
cd ETSToolbox-js
npm ci
npm test
npm run build
```

完整测试与构建命令：

```bash
npm ci
npm test
npm run build
git diff --check
```

Windows 安装器测试：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File '.\scripts\test-install.ps1'
```

该测试覆盖“目标 `etstoolbox` 目录完全不存在”和“目录存在但 `index.js` 缺失”两种情况。

如果客户端启动后立即退出，请不要反复启动。先在 Developer PowerShell 中收集最近的应用程序错误、已安装文件哈希和备份清单：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File '.\scripts\collect-crash-diagnostics.ps1'
```

脚本只读取本机信息，结果保存在当前目录的 `ETSToolbox-crash-diagnostics.txt`，不会启动或修改 E听说。

## 技术说明

- `winmm.dll` 使用 Win32 Release 和 MSVC `/MD` 构建，不依赖 `VCRUNTIME140D.dll`、`MSVCP140D.dll` 或 `ucrtbased.dll`。
- 控分只处理 `/m/audio/sync-v2`。基准百分比和随机偏移上限均限制为 `0` 至 `100`；每道题独立随机，越界时重新生成。最终百分比和分数最多保留两位小数，并遵循客户端分值步进。
- 作业用时只处理 `m/homework/set-use-time`。
- 自动流程只识别听说页面操作栏中的录音、停止录音和下一步按钮。录音按钮实际变为“录音中”后才开始 2 秒计时。
- 答案提取覆盖选择、填空、口语参考表达和常见嵌套答案字段。客户端更新内部数据结构后可能需要重新适配。
- 本地 HTTP 服务在 DLL 工作线程中延迟初始化，答案终端按钮只能启动安装目录内固定的 `open-answer-console.cmd`，不接受外部命令或路径参数。
- 原生日志源码、编译选项和运行时控制台统一使用 UTF-8；Windows 构建脚本也会把控制台代码页切换到 UTF-8。
- 目前构建的 DLL 架构是 x86，因为实测 `ETSShell.exe` 为 x86；安装目录名称本身不作为架构判断依据。

更详细的 DLL 构建和依赖检查见 [WINDOWS-RELEASE.md](WINDOWS-RELEASE.md)。

## 许可证

本维护分支新增的原创代码以 [MIT License](LICENSE) 提供。上游仓库没有声明许可证，因此上游既有代码的权利状态不会因本文件而改变；MIT 授权仅覆盖维护者有权许可的新增内容。
