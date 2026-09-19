// 逆波兰表达式求值 —— 可直接编译运行的完整示例
//
// 编译（装了 MinGW / g++ 后）：
//     g++ -std=c++17 -Wall -O2 eval_rpn.cpp -o eval_rpn
//     ./eval_rpn
//
// 对应 LeetCode 150: Evaluate Reverse Polish Notation

#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

// ---------- 解法一：用 std::stack（推荐） ----------
int evalRPN_stack(vector<string>& tokens) {
    stack<int> st;
    for (const string& t : tokens) {
        // 运算符一定是单字符 token；用 back() 判断可避免把 "-11" 误判成减号
        char x = t.back();
        if (t.size() == 1 && (x == '+' || x == '-' || x == '*' || x == '/')) {
            int b = st.top(); st.pop();   // 先弹出 -> 右操作数
            int a = st.top(); st.pop();   // 后弹出 -> 左操作数
            switch (x) {
                case '+': st.push(a + b); break;
                case '-': st.push(a - b); break;
                case '*': st.push(a * b); break;
                case '/': st.push(a / b); break;   // C++ 整数除法天然向零截断
            }
        } else {
            st.push(stoi(t));             // 数字："23"、"-11" 都能正确解析
        }
    }
    return st.top();
}

// ---------- 解法二：用 vector 当栈（看栈内变化方便） ----------
int evalRPN_vector(vector<string>& tokens) {
    vector<int> a;
    for (const string& t : tokens) {
        char x = t.back();
        if (t.size() == 1 && (x == '+' || x == '-' || x == '*' || x == '/')) {
            int right = a.back(); a.pop_back();   // 别漏掉这一句！
            int left  = a.back(); a.pop_back();   // 别漏掉这一句！
            switch (x) {
                case '+': a.push_back(left + right); break;
                case '-': a.push_back(left - right); break;
                case '*': a.push_back(left * right); break;
                case '/': a.push_back(left / right); break;
            }
        } else {
            a.push_back(stoi(t));
        }
    }
    return a.back();
}

// ---------- 测试 ----------
static int failed = 0;

void check(const vector<string>& tokens, int expected) {
    vector<string> t1 = tokens, t2 = tokens;
    int r1 = evalRPN_stack(t1);
    int r2 = evalRPN_vector(t2);
    bool ok = (r1 == expected && r2 == expected);
    if (!ok) failed++;

    cout << (ok ? "[PASS] " : "[FAIL] ");
    cout << "输入 { ";
    for (const auto& s : tokens) cout << '"' << s << "\" ";
    cout << "} 期望 " << expected
         << " | stack=" << r1 << " vector=" << r2 << "\n";
}

int main() {
    cout << "=== 逆波兰表达式求值 测试 ===\n\n";

    check({"2", "1", "+", "3", "*"}, 9);                       // (2+1)*3
    check({"4", "13", "5", "/", "+"}, 6);                      // 4 + 13/5  <- 经典漏 pop 反例
    check({"10", "6", "9", "3", "+", "-11", "*", "/", "*", "17", "+", "5", "+"}, 22);
    check({"3", "4", "+"}, 7);                                 // 简单加法
    check({"7", "2", "/"}, 3);                                 // 向零截断: 3
    check({"-7", "2", "/"}, -3);                               // 向零截断: -3 (不是 -4)
    check({"4", "3", "-"}, 1);                                 // 减法顺序: 4-3=1
    check({"3", "4", "-"}, -1);                                // 减法顺序: 3-4=-1
    check({"18"}, 18);                                         // 单个数字
    check({"1", "2", "+", "3", "4", "-", "*"}, -3);            // (1+2)*(3-4)

    cout << "\n";
    if (failed == 0) cout << "全部通过 ✓\n";
    else             cout << failed << " 个用例失败 ✗\n";
    return failed == 0 ? 0 : 1;
}
