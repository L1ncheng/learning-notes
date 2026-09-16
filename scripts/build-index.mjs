// 重新生成 INDEX.md，并刷新 README.md 里 STATS 标记之间的统计区块。
// 用法：node scripts/build-index.mjs [--quiet] [--today YYYY-MM-DD]

import { writeFile, readFile } from 'node:fs/promises'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

import {
  INDEX_PATH,
  README_PATH,
  STATS_START,
  STATS_END,
  listNotes,
  computeStreaks,
  formatMinutes,
  formatNow,
  parseISO,
  shiftISO,
  todayISO,
  weekdayCN,
} from './lib/notes.mjs'

function parseArgs(argv) {
  const opts = { quiet: false, today: todayISO() }
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i]
    if (arg === '--quiet' || arg === '-q') opts.quiet = true
    else if (arg === '--today') opts.today = String(argv[++i] ?? '').trim()
    else if (arg.startsWith('--today=')) opts.today = arg.slice('--today='.length).trim()
  }
  if (!parseISO(opts.today)) opts.today = todayISO()
  return opts
}

function summarize(notes, today) {
  const isoDates = notes.map((n) => n.iso)
  const { current, longest } = computeStreaks(isoDates, today)
  const minutes = notes.reduce((sum, n) => sum + n.minutes, 0)

  const tagMap = new Map()
  for (const note of notes) {
    for (const tag of note.tags) {
      if (!tagMap.has(tag)) tagMap.set(tag, [])
      tagMap.get(tag).push(note)
    }
  }

  const monthMap = new Map()
  for (const note of notes) {
    const key = `${note.year}-${note.month}`
    if (!monthMap.has(key)) monthMap.set(key, [])
    monthMap.get(key).push(note)
  }

  return {
    total: notes.length,
    first: notes[0]?.iso ?? '',
    last: notes[notes.length - 1]?.iso ?? '',
    current,
    longest,
    minutes,
    tagMap,
    monthMap,
    days: new Set(isoDates),
    byIso: new Map(notes.map((n) => [n.iso, n])),
  }
}

function renderIndex(notes, stats, today) {
  const lines = []
  const push = (s = '') => lines.push(s)

  push('# 📚 学习日志 · 索引')
  push()
  push('> 本文件由 `node scripts/build-index.mjs` 自动生成，**请勿手动编辑**。')
  push(`> 最后生成：${formatNow()}`)
  push()

  push('## 总览')
  push()
  push('| 指标 | 数值 |')
  push('| :--- | ---: |')
  push(`| 笔记总数 | **${stats.total}** 篇 |`)
  push(`| 起始日期 | ${stats.first || '—'} |`)
  push(`| 最近记录 | ${stats.last || '—'} |`)
  push(`| 当前连续 | **${stats.current}** 天 |`)
  push(`| 最长连续 | ${stats.longest} 天 |`)
  push(`| 累计学习 | ${formatMinutes(stats.minutes)} |`)
  push(`| 标签数 | ${stats.tagMap.size} |`)
  push()

  push('## 最近 7 天')
  push()
  push('| 日期 | 星期 | 笔记 | 状态 |')
  push('| :--- | :--- | :--- | :---: |')
  for (let i = 6; i >= 0; i -= 1) {
    const iso = shiftISO(today, -i)
    const note = stats.byIso.get(iso)
    const cell = note ? `[${escapeCell(note.title)}](${note.rel})` : '—'
    push(`| ${iso} | ${weekdayCN(iso)} | ${cell} | ${note ? '✅' : '⬜'} |`)
  }
  push()

  push('## 按月归档')
  push()
  if (stats.monthMap.size === 0) {
    push('_还没有笔记。_')
    push()
  } else {
    const months = [...stats.monthMap.keys()].sort().reverse()
    for (const key of months) {
      const group = stats.monthMap.get(key).slice().reverse()
      push(`<details open>`)
      push(`<summary><b>${key}</b> · ${group.length} 篇</summary>`)
      push()
      for (const note of group) {
        const tagPart = note.tags.length ? ` · ${note.tags.map((t) => `\`${t}\``).join(' ')}` : ''
        const mins = note.minutes ? ` · ${note.minutes} 分钟` : ''
        push(`- \`${note.iso.slice(5)}\` ${note.weekday} · [${escapeCell(note.title)}](${note.rel})${tagPart}${mins}`)
      }
      push()
      push('</details>')
      push()
    }
  }

  push('## 标签')
  push()
  if (stats.tagMap.size === 0) {
    push('_还没有标签。在笔记本的 frontmatter 里写 `tags: [算法, 英语]` 即可。_')
    push()
  } else {
    push('| 标签 | 篇数 | 相关笔记 |')
    push('| :--- | ---: | :--- |')
    const tags = [...stats.tagMap.entries()].sort((a, b) => b[1].length - a[1].length || a[0].localeCompare(b[0], 'zh'))
    for (const [tag, group] of tags) {
      const links = group
        .slice()
        .sort((a, b) => (a.iso < b.iso ? 1 : -1))
        .map((n) => `[${n.iso.slice(5)}](${n.rel})`)
        .join(' · ')
      push(`| \`${escapeCell(tag)}\` | ${group.length} | ${links} |`)
    }
    push()
  }

  return `${lines.join('\n').replace(/\n+$/, '')}\n`
}

function renderReadmeStats(notes, stats) {
  const lines = []
  lines.push(STATS_START)
  lines.push('<!-- 这一段由 scripts/build-index.mjs 自动生成，不要手动编辑 -->')
  if (stats.total === 0) {
    lines.push('_还没有笔记，运行 `node scripts/new-note.mjs` 开始第一篇。_')
  } else {
    lines.push(
      `**已坚持 ${stats.current} 天**（最长 ${stats.longest} 天） · 共 **${stats.total}** 篇笔记 · 累计 **${formatMinutes(stats.minutes)}** · 最近更新 \`${stats.last}\``,
    )
    lines.push('')
    lines.push('| 最近 5 篇 | 标签 |')
    lines.push('| :--- | :--- |')
    for (const note of notes.slice(-5).reverse()) {
      const tags = note.tags.length ? note.tags.map((t) => `\`${t}\``).join(' ') : '—'
      lines.push(`| \`${note.iso}\` ${note.weekday} · [${escapeCell(note.title)}](${note.rel}) | ${tags} |`)
    }
    lines.push('')
    lines.push('完整索引见 [`INDEX.md`](INDEX.md)。')
  }
  lines.push(STATS_END)
  return lines.join('\n')
}

function escapeCell(text) {
  return String(text).replace(/\|/g, '\\|').replace(/\r?\n/g, ' ').trim()
}

async function writeIfChanged(file, next, label, quiet) {
  let prev = null
  try {
    prev = await readFile(file, 'utf8')
  } catch {
    prev = null
  }
  if (prev === next) {
    if (!quiet) console.log(`= ${label} 无变化`)
    return false
  }
  await writeFile(file, next, 'utf8')
  if (!quiet) console.log(`✓ ${label} 已更新`)
  return true
}

export async function buildIndex({ quiet = false, today = todayISO() } = {}) {
  const notes = await listNotes()

  const seen = new Map()
  for (const note of notes) {
    if (seen.has(note.iso)) {
      console.warn(`! 日期重复：${note.iso} → ${seen.get(note.iso)} 与 ${note.rel}`)
    } else {
      seen.set(note.iso, note.rel)
    }
  }

  const stats = summarize(notes, today)
  await writeIfChanged(INDEX_PATH, renderIndex(notes, stats, today), 'INDEX.md', quiet)

  let readme = null
  try {
    readme = await readFile(README_PATH, 'utf8')
  } catch {
    if (!quiet) console.warn('! 找不到 README.md，跳过统计区块')
  }
  if (readme !== null) {
    const start = readme.indexOf(STATS_START)
    const end = readme.indexOf(STATS_END)
    if (start === -1 || end === -1 || end < start) {
      if (!quiet) console.warn('! README.md 缺少 STATS:START/STATS:END 标记，跳过统计区块')
    } else {
      const next = readme.slice(0, start) + renderReadmeStats(notes, stats) + readme.slice(end + STATS_END.length)
      await writeIfChanged(README_PATH, next, 'README.md 统计区块', quiet)
    }
  }

  if (!quiet) {
    console.log(`  共 ${stats.total} 篇笔记，当前连续 ${stats.current} 天，累计 ${formatMinutes(stats.minutes)}`)
  }
  return stats
}

const invokedDirectly =
  Boolean(process.argv[1]) && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)

if (invokedDirectly) {
  const opts = parseArgs(process.argv.slice(2))
  await buildIndex(opts)
}
