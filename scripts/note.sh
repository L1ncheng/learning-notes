#!/usr/bin/env sh
# 学习日志快捷入口（macOS / Linux / Git Bash）
#
#   ./scripts/note.sh                  创建今天的笔记
#   ./scripts/note.sh 2026-09-15       补写指定日期
#   ./scripts/note.sh yesterday        补写昨天
#   ./scripts/note.sh --force          覆盖已存在的笔记
#   ./scripts/note.sh --index          只重建索引

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname -- "$SCRIPT_DIR")

if ! command -v node >/dev/null 2>&1; then
    echo "✗ 找不到 node。请先安装 Node.js 18+：https://nodejs.org" >&2
    exit 127
fi

MODE="new"
ARGS=""

for arg in "$@"; do
    case "$arg" in
        --index|--index-only)
            MODE="index"
            ;;
        -h|--help)
            cat <<'EOF'
用法: ./scripts/note.sh [日期] [--force] [--index]

  日期        YYYY-MM-DD、today（默认）或 yesterday
  --force     已存在时覆盖
  --index     只重建 INDEX.md 与 README 统计

需要 Node.js 18+（https://nodejs.org）。
EOF
            exit 0
            ;;
        *)
            ARGS="$ARGS $arg"
            ;;
    esac
done

if [ "$MODE" = "index" ]; then
    # shellcheck disable=SC2086
    exec node "$SCRIPT_DIR/build-index.mjs" $ARGS
fi

# shellcheck disable=SC2086
exec node "$SCRIPT_DIR/new-note.mjs" $ARGS
