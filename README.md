# 📚 学习日志

> 每天一篇学习笔记，记录「学了什么、为什么、怎么用」。
> 索引与统计由脚本自动生成，人只负责写内容。

[![每日笔记与索引](https://github.com/L1ncheng/learning-notes/actions/workflows/notes.yml/badge.svg)](https://github.com/L1ncheng/learning-notes/actions/workflows/notes.yml)

<!-- STATS:START -->
<!-- 这一段由 scripts/build-index.mjs 自动生成，不要手动编辑 -->
**已坚持 2 天**（最长 2 天） · 共 **2** 篇笔记 · 累计 **4 小时 30 分钟** · 最近更新 `2026-09-17`

| 最近 5 篇 | 标签 |
| :--- | :--- |
| `2026-09-17` 星期四 · [2026-09-17 星期四 学习日志](notes/2026/09/2026-09-17.md) | `算法` `C++` `栈` `时间轴` `字符串解析` `调试` `AddressSanitizer` `复盘` |
| `2026-09-16` 星期三 · [2026-09-16 星期三 学习日志](notes/2026/09/2026-09-16.md) | `算法` `C++` `栈` `逆波兰表达式` `表达式求值` `调试` `stoi` |

完整索引见 [`INDEX.md`](INDEX.md)。
<!-- STATS:END -->

---

## 目录结构

```text
learning-notes/
├── README.md                       # 本文件（含自动生成的统计区块）
├── INDEX.md                        # 全量索引：总览 / 最近 7 天 / 按月归档 / 标签
├── notes/
│   ├── templates/
│   │   └── daily-template.md       # 每日笔记模板
│   └── 2026/
│       └── 09/
│           └── 2026-09-16.md       # 笔记正文，按 年/月 分目录
├── scripts/
│   ├── lib/notes.mjs               # 解析、路径、连续打卡等公共逻辑
│   ├── new-note.mjs                # 创建某天的笔记
│   ├── build-index.mjs             # 重新生成 INDEX.md 与 README 统计
│   ├── note.cmd                    # Windows 快捷入口（推荐）
│   ├── note.ps1                    # Windows / PowerShell 快捷入口
│   └── note.sh                     # macOS / Linux 快捷入口
└── .github/workflows/notes.yml     # 每天自动建笔记 + 每次 push 自动重建索引
```

## 环境要求

本地跑脚本需要 [Node.js](https://nodejs.org) 18 或更高版本，没有任何第三方依赖。
什么都不装也能用：在 GitHub 网页上直接新建或编辑笔记，Actions 会照常更新索引。

## 日常用法

### 1. 写今天的笔记

```bat
:: Windows（推荐，不受 PowerShell 执行策略限制）
scripts\note.cmd                  :: 创建今天的笔记
scripts\note.cmd 2026-09-15       :: 补写某一天
scripts\note.cmd yesterday        :: 补写昨天
scripts\note.cmd --index          :: 只重建索引
```

```powershell
# Windows / PowerShell
# 若提示「禁止运行脚本」，先执行一次：
#   Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
.\scripts\note.ps1                 # 创建今天的笔记
.\scripts\note.ps1 2026-09-15      # 补写某一天
.\scripts\note.ps1 -IndexOnly      # 只重建索引
```

```bash
# macOS / Linux
./scripts/note.sh
./scripts/note.sh 2026-09-15
./scripts/note.sh --index
```

也可以直接手写：复制 `notes/templates/daily-template.md` 到
`notes/<年>/<月>/<YYYY-MM-DD>.md`，再跑一次索引脚本即可。

### 2. 提交

```bash
git add -A
git commit -m "notes: 2026-09-16"
git push
```

推送后 GitHub Actions 会自动重建 `INDEX.md` 和 README 的统计区块并提交回仓库。

### 3. 什么都不做

仓库配置了定时任务，每天 `00:00`（北京时间）自动生成当天的空白笔记。
就算某天忘了写，文件也已经在那里了。

## 笔记格式

每篇笔记开头是一段 frontmatter：

```yaml
---
date: 2026-09-16      # 必填，决定文件路径与归档
weekday: 星期三        # 自动填充
tags: [算法, 动态规划]  # 用于 INDEX.md 的标签索引
mood: 专注             # 随意
study_minutes: 90     # 用于累计学习时长统计
---
```

正文里的第一个 `# 标题` 会被索引当作笔记标题。缺字段不会报错，
只是不会被统计到。

## 自动化

| 触发 | 行为 |
| --- | --- |
| `schedule`（每天 16:00 UTC） | 用模板创建当天的笔记，然后重建索引并提交 |
| `push` 到 `main` | 重建 `INDEX.md` 与 README 统计，有变化才提交 |
| `workflow_dispatch` | 手动指定日期补建笔记 |

工作流使用仓库自带的 `GITHUB_TOKEN`，因此它自己产生的推送不会再触发新的工作流。

## 许可

笔记内容版权归作者所有。仓库内的脚本以 MIT 许可使用，见 `LICENSE`。
