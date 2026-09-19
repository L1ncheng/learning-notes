// 函数的独占时间 —— 可直接编译运行的完整示例
//
// 编译（装了 MinGW / g++ 后）：
//     g++ -std=c++17 -Wall -Wextra -O2 exclusive_time.cpp -o exclusive_time
//     ./exclusive_time
//
// 对应 LeetCode 636: Exclusive Time of Functions
//
// 四种实现互为验证：
//   方案一  prev 记账       —— 只存 id，栈顶吃 [prev, t) 或 [prev, t]（推荐）
//   方案二  栈帧自带开始时刻 —— 栈里存 (id, startAt)，结束时间弹栈时结算
//   方案三  root 数组记账    —— 按函数 id 记"当前最内层栈帧的开始时刻"（递归也成立）
//   方案四  逐单位时间模拟   —— 暴力，逐秒判断谁在栈顶（只用于小时间戳对拍）

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// ---------- 解析 "id:start|end:timestamp" ----------
struct Log {
    int id;
    bool isStart;
    long long t;
};

Log parseLog(const string& log) {
    size_t p1 = log.find(':');              // id 后面的冒号
    size_t p2 = log.find(':', p1 + 1);      // start/end 后面的冒号
    Log e;
    e.id      = stoi(log.substr(0, p1));
    e.isStart = (log.compare(p1 + 1, 5, "start") == 0);
    e.t       = stoll(log.substr(p2 + 1));  // 时间戳最大 1e9，用 long long 稳
    return e;
}

// ---------- 方案一：prev 记账（推荐，最短） ----------
// 核心：不用管"函数是什么"，只维护「上一次时间轴发生变化的时刻 prev」，
//       时间片 [prev, ?] 永远归当前的栈顶。
vector<int> exclusiveTime(int n, vector<string>& logs) {
    vector<int> res(n, 0);
    vector<int> st;             // 栈里只存函数 id
    long long prev = 0;         // 上一次"状态切换"的时刻

    for (const string& log : logs) {
        Log e = parseLog(log);
        if (e.isStart) {
            // 新函数在 t 才开始，所以 [prev, t-1] 这 t-prev 个单位归"老栈顶"
            if (!st.empty()) res[st.back()] += (int)(e.t - prev);
            st.push_back(e.id);
            prev = e.t;                                  // 新函数从 t 开始执行
        } else {
            // end 表示"时间戳 t 的末尾结束"，所以 t 这一秒仍属于它 -> 闭区间要 +1
            res[st.back()] += (int)(e.t - prev + 1);
            st.pop_back();
            prev = e.t + 1;                              // 父函数从 t+1 恢复执行
        }
    }
    return res;
}

// ---------- 方案二：每个栈帧自带"本次开始执行的时刻" ----------
vector<int> exclusiveTime2(int n, vector<string>& logs) {
    vector<int> res(n, 0);
    vector<int> st;
    vector<long long> startAt;   // 与 st 同步：st[i] 这次是从 startAt[i] 开始跑的

    for (const string& log : logs) {
        Log e = parseLog(log);
        if (e.isStart) {
            // 父函数跑到 t-1 为止，结算掉
            if (!st.empty()) res[st.back()] += (int)(e.t - startAt.back());
            st.push_back(e.id);
            startAt.push_back(e.t);
        } else {
            res[e.id] += (int)(e.t - startAt.back() + 1);   // 含 t 这一秒
            st.pop_back();
            startAt.pop_back();
            if (!st.empty()) startAt.back() = e.t + 1;      // 父函数从 t+1 继续，重新计时
        }
    }
    return res;
}

// ---------- 方案三：root 数组记账（按函数 id 记"当前开始时刻"） ----------
// 栈里只存 id，"这一层是从哪一刻开始跑的"记在 root[id] 里。
// 递归同一个 id 也不会乱：内层 start 覆盖 root[id]，内层 end 又把 root[id] 改成 t+1
// 还给外层——因为"函数 id 当前最内层的那一帧"恰好就是我们唯一需要的信息。
vector<int> exclusiveTime3(int n, vector<string>& logs) {
    vector<int> res(n, 0);
    vector<int> root(n, 0);     // root[id] = id 当前最内层栈帧的开始时刻
    vector<int> st;

    for (const string& log : logs) {
        Log e = parseLog(log);
        if (e.isStart) {
            // 父函数跑到 t-1 为止：区间 [root[父], t-1]，长度 t - root[父]（没有 +1！）
            if (!st.empty()) res[st.back()] += (int)(e.t - root[st.back()]);
            st.push_back(e.id);
            root[e.id] = (int)e.t;
        } else {
            res[e.id] += (int)(e.t - root[e.id] + 1);   // 含 t 这一秒
            st.pop_back();
            if (!st.empty()) root[st.back()] = (int)e.t + 1;   // 父函数从 t+1 恢复
        }
    }
    return res;
}

// ---------- 方案四：逐单位时间模拟（暴力对拍用） ----------
// 对每个整数时刻 t，判断此刻"正在执行的函数"（= 栈顶），给它 +1。
// 时间戳可能到 1e9，所以只在 maxT 很小时启用；跳过时返回空数组。
vector<int> exclusiveTimeBrute(int n, vector<string>& logs) {
    vector<Log> ev;
    for (const string& s : logs) ev.push_back(parseLog(s));
    if (ev.empty()) return {};

    long long maxT = ev.back().t;
    if (maxT > 200000) return {};               // 太大，跳过（空数组 = 不打分）

    vector<int> res(n, 0), st;
    size_t j = 0;
    for (long long t = 0; t <= maxT; ++t) {
        // 同一时刻的 start 先处理：它在 t 这一秒开始执行
        while (j < ev.size() && ev[j].t == t && ev[j].isStart) {
            st.push_back(ev[j].id);
            ++j;
        }
        if (!st.empty()) res[st.back()] += 1;   // t 这一秒归栈顶

        // 同一时刻的 end 后处理：它在 t 这一秒的末尾才结束
        while (j < ev.size() && ev[j].t == t && !ev[j].isStart) {
            if (!st.empty()) st.pop_back();
            ++j;
        }
    }
    return res;
}

// ============================================================
// 测试框架
// ============================================================
static int failed = 0;

string show(const vector<int>& v) {
    if (v.empty()) return "(跳过)";
    string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ",";
        s += to_string(v[i]);
    }
    return s + "]";
}

void check(const string& name, int n, const vector<string>& logsRef,
           const vector<int>& expected) {
    vector<string> logs = logsRef;
    vector<int> r1 = exclusiveTime(n, logs);        // 方案一
    vector<int> r2 = exclusiveTime2(n, logs);       // 方案二
    vector<int> r3 = exclusiveTime3(n, logs);       // 方案三（root 数组）
    vector<int> r4 = exclusiveTimeBrute(n, logs);   // 方案四（可能跳过）

    bool ok = (r1 == expected) && (r2 == expected) && (r3 == expected)
              && (r4.empty() || r4 == expected);
    if (!ok) failed++;

    cout << (ok ? "[PASS] " : "[FAIL] ") << name
         << "  期望 " << show(expected)
         << " | 方案一 " << show(r1)
         << " | 方案二 " << show(r2)
         << " | 方案三 " << show(r3);
    if (!r4.empty()) cout << " | 暴力 " << show(r4);
    cout << "\n";
}

// ---------- 随机对拍：随机生成合法日志，四种实现互相比对 ----------
static unsigned long long rngState = 88172645463325252ULL;

unsigned long long rnd() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}

void randomCrossCheck(int rounds) {
    int bad = 0;
    for (int r = 0; r < rounds; ++r) {
        int n = 1 + (int)(rnd() % 4);          // 1..4 个函数
        vector<string> logs;
        vector<int> st;
        long long t = 0;

        int events = 2 * (1 + (int)(rnd() % 8));
        for (int i = 0; i < events; ++i) {
            bool doStart = st.empty() || (st.size() < 5 && rnd() % 100 < 55);
            if (doStart) {
                int id = (int)(rnd() % n);
                logs.push_back(to_string(id) + ":start:" + to_string(t));
                st.push_back(id);
            } else {
                int id = st.back();
                st.pop_back();
                logs.push_back(to_string(id) + ":end:" + to_string(t));
            }
            t += 1 + (long long)(rnd() % 3);    // 时间戳严格递增，满足题目约束
        }
        while (!st.empty()) {                   // 收尾：把没结束的调用都结束掉
            int id = st.back();
            st.pop_back();
            logs.push_back(to_string(id) + ":end:" + to_string(t));
            t += 1 + (long long)(rnd() % 3);
        }

        vector<int> a = exclusiveTime(n, logs);
        vector<int> b = exclusiveTime2(n, logs);
        vector<int> c = exclusiveTime3(n, logs);
        vector<int> d = exclusiveTimeBrute(n, logs);

        if (a != b || a != c || (!d.empty() && a != d)) {
            bad++;
            if (bad <= 3) {
                cout << "[DIFF] n=" << n << "  logs = { ";
                for (const auto& s : logs) cout << '"' << s << "\" ";
                cout << "}\n        方案一 " << show(a)
                     << " 方案二 " << show(b)
                     << " 方案三 " << show(c)
                     << " 暴力 " << show(d) << "\n";
            }
        }
    }
    if (bad) failed++;
    cout << (bad == 0 ? "[PASS] " : "[FAIL] ")
         << "随机对拍 " << rounds << " 组（四种实现结果一致）\n";
}

// ============================================================
int main() {
    cout << "=== 函数的独占时间 (LeetCode 636) 测试 ===\n\n";

    // ---- 题目给的三个示例 ----
    check("示例 1", 2, {"0:start:0", "1:start:2", "1:end:5", "0:end:6"}, {3, 4});
    check("示例 2 (自递归)", 1,
          {"0:start:0", "0:start:2", "0:end:5", "0:start:6", "0:end:6", "0:end:7"},
          {8});
    check("示例 3", 2,
          {"0:start:0", "0:start:2", "0:end:5", "1:start:6", "1:end:6", "0:end:7"},
          {7, 1});

    // ---- 边界 ----
    check("只占 1 个单位", 1, {"0:start:0", "0:end:0"}, {1});
    check("先后两个函数", 2,
          {"0:start:0", "0:end:1", "1:start:2", "1:end:2"}, {2, 1});
    check("三层嵌套各 2 秒", 3,
          {"0:start:0", "1:start:1", "2:start:2", "2:end:3", "1:end:4", "0:end:5"},
          {2, 2, 2});
    check("时间戳不从 0 开始", 2,
          {"0:start:5", "0:end:9", "1:start:10", "1:end:11"}, {5, 2});
    check("最大时间戳 (1e9)", 1, {"0:start:0", "0:end:1000000000"}, {1000000001});

    // ---- 随机对拍 ----
    cout << "\n";
    randomCrossCheck(3000);

    cout << "\n";
    if (failed == 0) cout << "全部通过 ✓\n";
    else             cout << failed << " 项失败 ✗\n";
    return failed == 0 ? 0 : 1;
}
