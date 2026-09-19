// 从 "id:start|end:timestamp" 里提取信息 —— 四种解析方式对照 + 常见坑演示
//
// 编译（装了 MinGW / g++ 后）：
//     g++ -std=c++17 -Wall -Wextra -O2 parse_log.cpp -o parse_log
//     ./parse_log
//
// 结论先放这里：字符串解析的通用套路只有三步 ——
//   ① 定位（find / rfind）  ② 切分（substr / getline）  ③ 转换（stoi / stoll）
// 永远不要写死下标，永远不要 (int) 一个字符。

#include <cctype>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

struct Log {
    int       id;
    bool      isStart;
    long long t;
};

bool same(const Log& a, const Log& b) {
    return a.id == b.id && a.isStart == b.isStart && a.t == b.t;
}

string show(const Log& e) {
    return "id=" + to_string(e.id) + " " + (e.isStart ? "start" : "end")
           + " t=" + to_string(e.t);
}

// ============================================================
// 四种解析写法
// ============================================================

// ---- 写法 1：find + substr + stoi/stoll（通用，推荐）----
Log parse1(const string& log) {
    size_t p1 = log.find(':');              // 第一个冒号：id 到此为止
    size_t p2 = log.find(':', p1 + 1);      // 第二个冒号：类型到此为止
    Log e;
    e.id      = stoi(log.substr(0, p1));       // [0, p1)
    e.isStart = (log.compare(p1 + 1, 5, "start") == 0);
    e.t       = stoll(log.substr(p2 + 1));     // p2+1 到末尾
    return e;
}

// ---- 写法 2：stringstream + getline(ss, token, ':')（切分最省心）----
Log parse2(const string& log) {
    stringstream ss(log);
    string sId, sType, sTime;
    getline(ss, sId,   ':');                // 读到第一个 ':' 为止
    getline(ss, sType, ':');                // 读到第二个 ':' 为止
    getline(ss, sTime);                     // 剩下一整段

    Log e;
    e.id      = stoi(sId);
    e.isStart = (sType == "start");
    e.t       = stoll(sTime);
    return e;
}

// ---- 写法 3：sscanf（C 风格，最短；MSVC 下要加 _CRT_SECURE_NO_WARNINGS）----
Log parse3(const string& log) {
    int  id = 0;
    char type[16] = {0};
    long long t = 0;
    sscanf(log.c_str(), "%d:%15[^:]:%lld", &id, type, &t);
    Log e;
    e.id      = id;
    e.isStart = (string(type) == "start");
    e.t       = t;
    return e;
}

// ---- 写法 4：手写扫描（最快、零分配，面试可以讲原理）----
Log parse4(const string& log) {
    size_t i = 0;
    int id = 0;
    while (i < log.size() && isdigit((unsigned char)log[i]))
        id = id * 10 + (log[i++] - '0');    // 关键：字符 - '0' 才是数值

    ++i;                                     // 跳过第一个 ':'
    bool isStart = (log[i] == 's');          // "start" / "end" 首字母就够区分
    while (i < log.size() && log[i] != ':') ++i;
    ++i;                                     // 跳过第二个 ':'

    long long t = 0;
    while (i < log.size() && isdigit((unsigned char)log[i]))
        t = t * 10 + (log[i++] - '0');

    Log e;
    e.id = id;
    e.isStart = isStart;
    e.t = t;
    return e;
}

// ============================================================
// 测试框架
// ============================================================
static int failed = 0;

void ok(bool cond, const string& name, const string& extra = "") {
    if (!cond) failed++;
    cout << (cond ? "[PASS] " : "[FAIL] ") << name;
    if (!extra.empty()) cout << "   " << extra;
    cout << "\n";
}

// ---- 1. 四种写法必须给出完全相同的结果 ----
void testParsers() {
    cout << "=== 1. 四种解析写法对照 ===\n";
    vector<string> samples = {
        "0:start:0", "1:end:5", "12:start:345", "99:end:1000000000"
    };
    for (const string& s : samples) {
        Log a = parse1(s), b = parse2(s), c = parse3(s), d = parse4(s);
        bool good = same(a, b) && same(a, c) && same(a, d);
        if (!good) failed++;
        cout << (good ? "[PASS] " : "[FAIL] ") << '"' << s << '"'
             << "  ->  " << show(a)
             << " | 写法2 " << show(b)
             << " | 写法3 " << show(c)
             << " | 写法4 " << show(d) << "\n";
    }
    cout << "\n";
}

// ---- 2. 字符 ≠ 数字 ----
void testCharVsDigit() {
    cout << "=== 2. 字符和数字：唯一的桥是 c - '0' ===\n";
    char c = '7';
    ok(((int)c == 55), "(int)'7' 得到的是 ASCII 码", "= 55");
    ok((c - '0' == 7), "'7' - '0' 才是数值", "= 7");
    ok(('0' == 48 && '9' == 57), "'0'..'9' 在 ASCII 里连续", "48..57");
    ok((':' == 58), "':' 紧挨在 '9' 后面", "58 —— 所以 c <= '9' 能识别数字");

    // 多位数不能只取一个字符
    Log e = parse4("12:start:345");
    ok(e.id == 12 && e.t == 345, "两位数编号 / 三位数时间戳", show(e));
    cout << "\n";
}

// ---- 3. 常见坑：substr / find / stoi ----
void testPitfalls() {
    cout << "=== 3. 常见坑 ===\n";
    string s = "0:start:123";

    // substr 第二个参数是"长度"，不是结束下标
    ok((s.substr(0, 1) == "0"), "substr(0, 1) 取 1 个字符");
    ok((s.substr(8) == "123"), "substr(8) 省略长度 = 到末尾");
    ok((s.substr(8, 2) == "12"), "substr(8, 2) 是长度 2，不是 [8,2)");

    // find 找不到返回 npos（不是 -1、不是 0）
    ok((s.find('@') == string::npos), "find 找不到返回 npos");
    ok((s.find(':') == 1), "第一个 ':' 在下标 1");
    ok((s.find(':', 2) == 7), "从下标 2 开始找第二个 ':'", "= 7");

    // stoi 只解析"能解析的前缀"，不报错（危险）
    size_t pos = 0;
    int v = stoi("123abc", &pos);
    ok((v == 123 && pos == 3), "stoi(\"123abc\") = 123，不报错",
       "pos 告诉你停在哪，可用它校验整串");

    // stoi 遇到完全不是数字的内容会抛异常
    bool threw = false;
    try { stoi("abc"); } catch (const invalid_argument&) { threw = true; }
    ok(threw, "stoi(\"abc\") 抛 invalid_argument");

    threw = false;
    try { stoi("99999999999"); } catch (const out_of_range&) { threw = true; }
    ok(threw, "stoi 超出 int 范围抛 out_of_range", "所以时间戳用 stoll");

    // 相反方向：数字 -> 字符串
    ok((to_string(345) == "345"), "to_string 反向转换");
    cout << "\n";
}

// ---- 4. 通用 split（按分隔符切分）----
vector<string> split(const string& s, char delim) {
    vector<string> out;
    string cur;
    for (char c : s) {
        if (c == delim) { out.push_back(cur); cur.clear(); }
        else            { cur += c; }
    }
    out.push_back(cur);              // 最后一段
    return out;
}

void testSplit() {
    cout << "=== 4. 通用 split ===\n";
    vector<string> v = split("12:start:345", ':');
    bool good = (v.size() == 3 && v[0] == "12" && v[1] == "start" && v[2] == "345");
    if (!good) failed++;
    cout << (good ? "[PASS] " : "[FAIL] ") << "split(\"12:start:345\", ':') = ";
    for (size_t i = 0; i < v.size(); ++i) cout << (i ? " | " : "") << '"' << v[i] << '"';
    cout << "   （注意 split 出来的是字符串，还要 stoi）\n\n";
}

// ============================================================
// 5. 用解析结果解 LeetCode 636（root 数组版，即批改后的代码）
// ============================================================
vector<int> exclusiveTime(int n, vector<string>& logs) {
    vector<int> res(n, 0);
    vector<int> root(n, 0);
    vector<int> st;
    for (const string& log : logs) {
        Log e = parse1(log);
        if (e.isStart) {
            if (!st.empty()) res[st.back()] += (int)(e.t - root[st.back()]);  // 半开，无 +1
            st.push_back(e.id);
            root[e.id] = (int)e.t;
        } else {
            res[e.id] += (int)(e.t - root[e.id] + 1);                        // 含 t
            st.pop_back();
            if (!st.empty()) root[st.back()] = (int)e.t + 1;                 // 父函数 t+1 恢复
        }
    }
    return res;
}

void testSolve() {
    cout << "=== 5. 解析 + 解题（LeetCode 636）===\n";
    struct Case { const char* name; int n; vector<string> logs; vector<int> want; };
    vector<Case> cases = {
        {"示例 1", 2, {"0:start:0", "1:start:2", "1:end:5", "0:end:6"}, {3, 4}},
        {"示例 2", 1, {"0:start:0", "0:start:2", "0:end:5",
                       "0:start:6", "0:end:6", "0:end:7"}, {8}},
        {"示例 3", 2, {"0:start:0", "0:start:2", "0:end:5",
                       "1:start:6", "1:end:6", "0:end:7"}, {7, 1}},
        {"两位数编号", 12, {"11:start:0", "11:end:7"}, {0,0,0,0,0,0,0,0,0,0,0,7}},
    };
    for (Case& c : cases) {
        vector<int> got = exclusiveTime(c.n, c.logs);
        bool good = (got == c.want);
        if (!good) failed++;
        cout << (good ? "[PASS] " : "[FAIL] ") << c.name << "  -> [";
        for (size_t i = 0; i < got.size(); ++i) cout << (i ? "," : "") << got[i];
        cout << "]\n";
    }
    cout << "\n";
}

int main() {
    cout << "=== 字符串解析：从 \"id:start|end:timestamp\" 提取信息 ===\n\n";
    testParsers();
    testCharVsDigit();
    testPitfalls();
    testSplit();
    testSolve();

    if (failed == 0) cout << "全部通过 ✓\n";
    else             cout << failed << " 项失败 ✗\n";
    return failed == 0 ? 0 : 1;
}
