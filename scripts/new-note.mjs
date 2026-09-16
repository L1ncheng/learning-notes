// 用模板创建某一天的笔记，然后刷新索引。
// 用法：node scripts/new-note.mjs [YYYY-MM-DD|today|yesterday] [--force] [--no-index] [--quiet]

import { mkdir, readFile, writeFile } from 'node:fs/promises'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

import {
  TEMPLATE_PATH,
  notePathFor,
  parseISO,
  relFromRoot,
  shiftISO,
  todayISO,
  weekdayCN,
} from './lib/notes.mjs'
import { buildIndex } from './build-index.mjs'

const DEFAULT_TEMPLATE = `---
date: {{date}}
weekday: {{weekday}}
tags: []
mood: 
study_minutes: 0
---

# {{date}} {{weekday}} 学习日志

## 🎯 今日目标

- [ ] 

## 📚 学习内容

## ✅ 今日总结

- 

## 🚀 明日计划

- [ ] 
`

function usage() {
  console.log(`用法: node scripts/new-note.mjs [日期] [选项]

日期     YYYY-MM-DD、today（默认）或 yesterday
选项
  --date <日期>   等价于位置参数
  --force         已存在时覆盖（默认不覆盖）
  --no-index      创建后不刷新索引
  --quiet         只输出必要信息
  -h, --help      显示本帮助`)
}

function parseArgs(argv) {
  const opts = { date: '', force: false, index: true, quiet: false, help: false }
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i]
    if (arg === '-h' || arg === '--help') opts.help = true
    else if (arg === '--force' || arg === '-f') opts.force = true
    else if (arg === '--no-index') opts.index = false
    else if (arg === '--quiet' || arg === '-q') opts.quiet = true
    else if (arg === '--date') opts.date = String(argv[++i] ?? '')
    else if (arg.startsWith('--date=')) opts.date = arg.slice('--date='.length)
    else if (!arg.startsWith('-') && !opts.date) opts.date = arg
    else {
      console.error(`未知参数：${arg}`)
      opts.help = true
    }
  }
  return opts
}

function resolveDate(input) {
  const raw = String(input ?? '').trim()
  if (!raw || raw === 'today' || raw === '今天') return todayISO()
  if (raw === 'yesterday' || raw === '昨天') return shiftISO(todayISO(), -1)
  if (parseISO(raw)) return raw
  return null
}

export async function createNote({ date = '', force = false, quiet = false, index = true } = {}) {
  const iso = resolveDate(date)
  if (!iso) {
    console.error(`✗ 无法识别的日期：${date}（应为 YYYY-MM-DD / today / yesterday）`)
    return null
  }

  const target = notePathFor(iso)
  let template
  try {
    template = await readFile(TEMPLATE_PATH, 'utf8')
  } catch {
    if (!quiet) console.warn(`! 读不到模板 ${relFromRoot(TEMPLATE_PATH)}，使用内置模板`)
    template = DEFAULT_TEMPLATE
  }

  const content = template
    .replaceAll('{{date}}', iso)
    .replaceAll('{{weekday}}', weekdayCN(iso))
    .replaceAll('{{title}}', `${iso} ${weekdayCN(iso)} 学习日志`)

  await mkdir(path.dirname(target), { recursive: true })

  try {
    await writeFile(target, content, { encoding: 'utf8', flag: force ? 'w' : 'wx' })
    if (!quiet) console.log(`✓ 已创建 ${relFromRoot(target)}`)
  } catch (err) {
    if (err && err.code === 'EEXIST') {
      if (!quiet) console.log(`= 已存在，跳过 ${relFromRoot(target)}（用 --force 覆盖）`)
    } else {
      throw err
    }
  }

  if (index) await buildIndex({ quiet, today: todayISO() })
  return target
}

const invokedDirectly = Boolean(process.argv[1]) && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)

if (invokedDirectly) {
  const opts = parseArgs(process.argv.slice(2))
  if (opts.help) {
    usage()
  } else {
    const result = await createNote(opts)
    if (result === null) process.exitCode = 1
  }
}
