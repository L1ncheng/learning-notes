// 学习日志的公共逻辑：路径、日期、frontmatter 解析、连续打卡统计。
// 被 new-note.mjs 与 build-index.mjs 共用。零依赖，只用 Node 内置模块。

import { readdir, readFile } from 'node:fs/promises'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

/** 仓库根目录（scripts/lib/ 往上两级） */
export const ROOT = path.resolve(fileURLToPath(new URL('../..', import.meta.url)))
export const NOTES_DIR = path.join(ROOT, 'notes')
export const TEMPLATE_PATH = path.join(NOTES_DIR, 'templates', 'daily-template.md')

export const INDEX_PATH = path.join(ROOT, 'INDEX.md')
export const README_PATH = path.join(ROOT, 'README.md')

export const STATS_START = '<!-- STATS:START -->'
export const STATS_END = '<!-- STATS:END -->'

const WEEKDAYS = ['星期日', '星期一', '星期二', '星期三', '星期四', '星期五', '星期六']

export const pad2 = (n) => String(n).padStart(2, '0')

/** 用本地时间（而非 UTC）格式化成 YYYY-MM-DD */
export function toISODate(date) {
  return `${date.getFullYear()}-${pad2(date.getMonth() + 1)}-${pad2(date.getDate())}`
}

export function todayISO(now = new Date()) {
  return toISODate(now)
}

/** 解析 YYYY-MM-DD；非法返回 null */
export function parseISO(iso) {
  const m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(String(iso ?? '').trim())
  if (!m) return null
  const [, y, mo, d] = m.map(Number)
  const date = new Date(y, mo - 1, d)
  if (date.getFullYear() !== y || date.getMonth() !== mo - 1 || date.getDate() !== d) return null
  return date
}

export function weekdayCN(iso) {
  const date = parseISO(iso)
  return date ? WEEKDAYS[date.getDay()] : ''
}

export function shiftISO(iso, days) {
  const date = parseISO(iso)
  if (!date) return null
  date.setDate(date.getDate() + days)
  return toISODate(date)
}

/** notes/<年>/<月>/<YYYY-MM-DD>.md */
export function notePathFor(iso) {
  const [y, m] = iso.split('-')
  return path.join(NOTES_DIR, y, m, `${iso}.md`)
}

export function relFromRoot(abs) {
  return path.relative(ROOT, abs).split(path.sep).join('/')
}

function unquote(value) {
  const v = String(value).trim()
  if (v.length >= 2 && ((v.startsWith('"') && v.endsWith('"')) || (v.startsWith("'") && v.endsWith("'")))) {
    return v.slice(1, -1)
  }
  return v
}

/**
 * 解析极简 YAML frontmatter。只支持 `key: value`、`key: [a, b]` 和 `key:` 后跟 `- item` 列表。
 * 解析失败不抛错，只会得到空对象 —— 笔记不该因为格式问题而无法统计。
 */
export function parseFrontmatter(text) {
  const normalized = String(text).replace(/^\uFEFF/, '')
  const match = /^---[ \t]*\r?\n([\s\S]*?)\r?\n---[ \t]*(?:\r?\n|$)/.exec(normalized)
  if (!match) return { data: {}, body: normalized }

  const data = {}
  let key = null
  for (const rawLine of match[1].split(/\r?\n/)) {
    const line = rawLine.trimEnd()
    if (!line.trim() || line.trim().startsWith('#')) continue

    const listItem = /^\s*-\s+(.*)$/.exec(line)
    if (listItem && key) {
      if (!Array.isArray(data[key])) data[key] = data[key] ? [data[key]] : []
      data[key].push(unquote(listItem[1]))
      continue
    }

    const kv = /^([A-Za-z0-9_-]+)\s*:\s*(.*)$/.exec(line)
    if (!kv) continue
    key = kv[1]
    const value = kv[2].trim()
    if (value === '') {
      data[key] = ''
    } else if (value.startsWith('[') && value.endsWith(']')) {
      data[key] = value
        .slice(1, -1)
        .split(',')
        .map((s) => unquote(s))
        .filter(Boolean)
    } else {
      data[key] = unquote(value)
    }
  }
  return { data, body: normalized.slice(match[0].length) }
}

export function firstHeading(body) {
  const m = /^#[ \t]+(.+?)[ \t]*$/m.exec(body)
  return m ? m[1].trim() : ''
}

function asTags(value) {
  if (Array.isArray(value)) return value.map((t) => String(t).trim()).filter(Boolean)
  if (typeof value === 'string' && value.trim()) {
    return value
      .split(/[,，]/)
      .map((t) => t.trim())
      .filter(Boolean)
  }
  return []
}

export function asMinutes(value) {
  const n = Number(value)
  return Number.isFinite(n) && n > 0 ? Math.round(n) : 0
}

/** 扫描 notes/<年>/<月>/ 下的所有笔记并按日期升序返回 */
export async function listNotes() {
  const notes = []
  let years = []
  try {
    years = await readdir(NOTES_DIR, { withFileTypes: true })
  } catch {
    return notes
  }

  for (const year of years) {
    if (!year.isDirectory() || !/^\d{4}$/.test(year.name)) continue
    const yearDir = path.join(NOTES_DIR, year.name)
    const months = await readdir(yearDir, { withFileTypes: true })
    for (const month of months) {
      if (!month.isDirectory() || !/^\d{2}$/.test(month.name)) continue
      const monthDir = path.join(yearDir, month.name)
      const files = await readdir(monthDir, { withFileTypes: true })
      for (const file of files) {
        if (!file.isFile() || !file.name.endsWith('.md')) continue
        const abs = path.join(monthDir, file.name)
        const text = await readFile(abs, 'utf8')
        const { data, body } = parseFrontmatter(text)
        const iso = parseISO(data.date) ? String(data.date).trim() : file.name.replace(/\.md$/, '')
        if (!parseISO(iso)) continue
        const title = firstHeading(body) || iso
        notes.push({
          iso,
          file: abs,
          rel: relFromRoot(abs),
          title,
          tags: asTags(data.tags),
          minutes: asMinutes(data.study_minutes),
          mood: typeof data.mood === 'string' ? data.mood.trim() : '',
          weekday: weekdayCN(iso),
          year: iso.slice(0, 4),
          month: iso.slice(5, 7),
          body,
        })
      }
    }
  }

  notes.sort((a, b) => (a.iso < b.iso ? -1 : a.iso > b.iso ? 1 : 0))
  return notes
}

/** 把 YYYY-MM-DD 映射成「天序号」，用 UTC 计算，避免夏令时导致的偏差 */
function dayNumber(iso) {
  const date = parseISO(iso)
  if (!date) return NaN
  return Date.UTC(date.getFullYear(), date.getMonth(), date.getDate()) / 86400000
}

/** 连续打卡：当前连续（截至今天或昨天）与历史最长 */
export function computeStreaks(isoDates, today = todayISO()) {
  const days = [...new Set(isoDates)].sort()
  if (days.length === 0) return { current: 0, longest: 0 }

  const at = dayNumber

  let longest = 1
  let run = 1
  for (let i = 1; i < days.length; i += 1) {
    run = at(days[i]) - at(days[i - 1]) === 1 ? run + 1 : 1
    if (run > longest) longest = run
  }

  const last = days[days.length - 1]
  const gapToToday = at(today) - at(last)
  let current = 0
  if (gapToToday === 0 || gapToToday === 1) {
    current = 1
    for (let i = days.length - 1; i > 0; i -= 1) {
      if (at(days[i]) - at(days[i - 1]) !== 1) break
      current += 1
    }
  }
  return { current, longest }
}

export function formatMinutes(total) {
  const hours = Math.floor(total / 60)
  const mins = total % 60
  if (total === 0) return '0 分钟'
  if (hours === 0) return `${mins} 分钟`
  if (mins === 0) return `${hours} 小时`
  return `${hours} 小时 ${mins} 分钟`
}

export function formatNow(now = new Date()) {
  const offset = -now.getTimezoneOffset()
  const sign = offset >= 0 ? '+' : '-'
  const abs = Math.abs(offset)
  return `${toISODate(now)} ${pad2(now.getHours())}:${pad2(now.getMinutes())} UTC${sign}${pad2(Math.floor(abs / 60))}:${pad2(abs % 60)}`
}
