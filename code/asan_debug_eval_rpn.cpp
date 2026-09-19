# 复现 & 定位 ASan heap-buffer-overflow
#
# 需要 g++（或 clang++）。你机器上目前没装，装好 MinGW-w64 后可用。
#
# ===== 编译并运行（带 AddressSanitizer）=====
#   编译：
#     g++ -std=c++17 -g -O0 -fsanitize=address -fno-omit-frame-pointer \
#         -Wall -Wextra asan_debug_eval_rpn.cpp -o asan_debug
#
#   运行：
#     ./asan_debug
#
# Windows + MinGW 若报缺少 libasan，改用：
#     -fsanitize=address -static-libasan
#
# ===== 重点 =====
# 下面 test_* 函数按"可能是元凶"的顺序排列。
# 第 1 组会故意触发空栈崩溃，用来确认你遇到的报错形态是否一致。
# 第 2 组全是合法输入，用来确认正确解不会崩。

#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

// ============================================================
// 解法 A：你提交的版本（原样复制）
// ============================================================
int evalRPN_user(vector<string>& tokens) {
    vector<int> a;
    int n = tokens.size();
    for (int i = 0; i < n; i++) {
        char x = tokens[i][0];          // <-- 用首字符判断
        switch (x) {
            case '+': {
                int t1 = a.back(); a.pop_back();
                int t2 = a.back(); a.pop_back();
                a.push_back(t1 + t2);
                break;
            }
            case '-': {
                int t1 = a.back(); a.pop_back();
                int t2 = a.back(); a.pop_back();
                a.push_back(t2 - t1);
                break;
            }
            case '*': {
                int t1 = a.back(); a.pop_back();
                int t2 = a.back(); a.pop_back();
                a.push_back(t1 * t2);
                break;
            }
            case '/': {
                int t1 = a.back(); a.pop_back();
                int t2 = a.back(); a.pop_back();
                a.push_back(t2 / t1);
                break;
            }
            default:
                a.push_back(stoi(tokens[i]));
        }
    }
    return a.back();
}

// ============================================================
// 解法 B：安全版（带防御，不可能越界）
// ============================================================
int evalRPN_safe(vector<string>& tokens) {
    if (tokens.empty()) return 0;

    vector<int> a;
    for (const string& t : tokens) {
        if (t.empty()) continue;

        char x = t.back();                       // 末字符判断
        bool isOp = (t.size() == 1 &&
                     (x == '+' || x == '-' || x == '*' || x == '/'));

        if (isOp) {
            if (a.size() < 2) {                  // 防御：操作数不足
                cerr << "  [guard] 运算符 '" << x << "' 时栈不足 2 个元素\n";
                return 0;
            }
            int right = a.back(); a.pop_back();
            int left  = a.back(); a.pop_back();
            if      (x == '+') a.push_back(left + right);
            else if (x == '-') a.push_back(left - right);
            else if (x == '*') a.push_back(left * right);
            else               a.push_back(left / right);
        } else {
            a.push_back(stoi(t));
        }
    }
    return a.empty() ? 0 : a.back();
}

// ============================================================
// 测试框架
// ============================================================
static int failures = 0;

// 只跑安全版，比对期望值
void expect(const string& name, vector<string> tokens, int want) {
    int got = evalRPN_safe(tokens);
    bool ok = (got == want);
    if (!ok) failures++;
    cout << (ok ? "[PASS] " : "[FAIL] ") << name
         << "  期望=" << want << " 实得=" << got << "\n";
}

// 跑用户版，看会不会崩
void probe_user(const string& name, vector<string> tokens) {
    cout << "[probe] " << name << "  ->  ";
    cout.flush();
    int got = evalRPN_user(tokens);
    cout << "返回 " << got << " (没崩)\n";
}

int main() {
    cout << "===== 第 2 组：合法输入，验证安全版 =====" << endl;
    expect("基本加法",     {"3", "4", "+"}, 7);
    expect("两数相乘",     {"2", "1", "+", "3", "*"}, 9);
    expect("除加组合",     {"4", "13", "5", "/", "+"}, 6);
    expect("负数除",       {"-7", "2", "/"}, -3);
    expect("减法顺序",     {"3", "4", "-"}, -1);
    expect("LC 长用例",    {"10","6","9","3","+","-11","*","/","*","17","+","5","+"}, 22);
    expect("单个数字",     {"18"}, 18);
    expect("负号开头数字", {"-11"}, -11);
    expect("多位数",       {"100", "25", "/"}, 4);

    cout << "\n===== 第 1 组：故意触发空栈 =====" << endl;
    cout << "说明：合法表达式不会走到这里；下面这些是【不合法】输入，\n";
    cout << "      用来复现 ASan 的 heap-buffer-overflow 报错形态。\n\n";

    // 1) 空输入：tokens 为空 -> 直接 return a.back()，栈空
    {
        cout << "--- 用例 1: 空输入 {} ---\n";
        vector<string> t = {};
        cout << "  safe 版: " << evalRPN_safe(t) << " (有防护)\n";
        // 取消下面两行注释即可复现崩溃：
        // cout << "  user 版: ";
        // cout << evalRPN_user(t) << "\n";
    }

    // 2) 运算符开头：栈里没操作数就弹
    {
        cout << "--- 用例 2: 运算符开头 {\"+\"} ---\n";
        probe_user("user 版 {\"+\"}", {"+"});
    }

    // 3) 操作数不足
    {
        cout << "--- 用例 3: 操作数不足 {\"1\",\"+\"} ---\n";
        probe_user("user 版 {\"1\",\"+\"}", {"1", "+"});
    }

    cout << "\n";
    if (failures == 0) cout << "安全版全部通过 ✓" << endl;
    else               cout << failures << " 个合法用例失败 ✗" << endl;

    cout << "\n提示：如果你想确认判题机崩溃的确切输入，\n";
    cout << "      把上面 probe_user 换成判题机给的 tokens 即可复现。\n";
    return failures == 0 ? 0 : 1;
}
