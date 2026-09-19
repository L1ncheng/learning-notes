// 单调栈（Monotonic Stack）—— 六个模板 + 暴力对拍，可直接编译运行
//
// 编译（装了 MinGW / g++ 后）：
//     g++ -std=c++17 -Wall -Wextra -O2 monotonic_stack.cpp -o monotonic_stack
//     ./monotonic_stack
//
// 收录的模板：
//   ① 739  每日温度        —— 下一个更大（递减栈，存下标，要距离）
//   ② 496  下一个更大元素 I —— 下一个更大（递减栈，只存值，不要距离）
//   ③ 503  下一个更大元素 II —— 循环数组（走两遍，取模）
//   ④ 84   柱状图中最大矩形 —— 递增栈 + 两端哨兵（算左右边界）
//   ⑤ 42   接雨水          —— 递减栈，弹出时横向按层累加
//   ⑥ 907  子数组最小值之和 —— 贡献法：左边"严格小于"、右边"小于等于"
//   对比 239 滑动窗口最大值 —— 单调「队列」（多一个队头出队）
//
// 所有实现都和 O(n^2) 暴力版做了随机对拍。

#include <algorithm>
#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// ============================================================
// ① LC 739 每日温度：对每个 i，找右边第一个比它大的位置 j，答案是 j - i
//    栈：单调递减（栈底到栈顶值递减），存【下标】（因为要算距离）
// ============================================================
vector<int> dailyTemperatures(vector<int>& T) {
    int n = (int)T.size();
    vector<int> ans(n, 0), st;
    for (int i = 0; i < n; ++i) {
        // 当前温度比栈顶高 -> 栈顶那个位置"等到了它的答案"
        while (!st.empty() && T[st.back()] < T[i]) {
            int j = st.back();
            st.pop_back();
            ans[j] = i - j;
        }
        st.push_back(i);
    }
    return ans;     // 还留在栈里的，右边没有更高的温度，保持 0
}

// ============================================================
// ② LC 496 下一个更大元素 I
//    栈里存【值】就够了 —— 因为只要"下一个更大是谁"，不要距离
// ============================================================
vector<int> nextGreaterElement(vector<int>& nums1, vector<int>& nums2) {
    unordered_map<int, int> nxt;        // 值 -> 它右边第一个更大的值
    vector<int> st;                     // 单调递减栈（存值）
    for (int x : nums2) {
        while (!st.empty() && st.back() < x) {
            nxt[st.back()] = x;
            st.pop_back();
        }
        st.push_back(x);
    }
    vector<int> ans;
    ans.reserve(nums1.size());
    for (int x : nums1) {
        auto it = nxt.find(x);
        ans.push_back(it == nxt.end() ? -1 : it->second);
    }
    return ans;
}

// ============================================================
// ③ LC 503 下一个更大元素 II：循环数组
//    技巧：走两圈（2n），只有第一圈的下标入栈，第二圈纯粹用来"当别人的后继"
// ============================================================
vector<int> nextGreaterElementsII(vector<int>& nums) {
    int n = (int)nums.size();
    vector<int> ans(n, -1), st;         // 存下标（停在 [0, n) 内）
    for (int i = 0; i < 2 * n; ++i) {
        int cur = nums[i % n];
        while (!st.empty() && nums[st.back()] < cur) {
            ans[st.back()] = cur;
            st.pop_back();
        }
        if (i < n) st.push_back(i);     // 第二圈不再入栈
    }
    return ans;
}

// ============================================================
// ④ LC 84 柱状图中最大的矩形
//    思路：以每根柱子为"最矮的那根"向两边扩。递增栈里存下标；
//          当 h[i] 比栈顶矮时，栈顶就是"以它为高的矩形"的右边界。
//    技巧：两端补哨兵 0 —— 省掉判空，也省掉遍历结束后的收尾。
// ============================================================
int largestRectangleArea(vector<int>& heights) {
    vector<int> h;
    h.reserve(heights.size() + 2);
    h.push_back(0);                              // 左哨兵
    for (int x : heights) h.push_back(x);
    h.push_back(0);                              // 右哨兵

    vector<int> st;                              // 单调递增栈（存下标）
    int best = 0;
    for (int i = 0; i < (int)h.size(); ++i) {
        while (!st.empty() && h[st.back()] > h[i]) {
            int height = h[st.back()];
            st.pop_back();
            int left = st.empty() ? -1 : st.back();     // 左边界（不含）
            best = max(best, height * (i - left - 1));  // 宽度 = i - left - 1
        }
        st.push_back(i);
    }
    return best;
}

// ============================================================
// ⑤ LC 42 接雨水（单调栈版）
//    栈：单调递减。当 h[i] 比栈顶高时，栈顶是"坑底"，
//        它的左边界是新栈顶，右边界是 i —— 这一层的水是横着加的。
// ============================================================
int trap(vector<int>& height) {
    vector<int> st;
    int water = 0;
    for (int i = 0; i < (int)height.size(); ++i) {
        while (!st.empty() && height[st.back()] < height[i]) {
            int bottom = st.back();
            st.pop_back();
            if (st.empty()) break;               // 左边没有更高的边 -> 这一层接不住水
            int left = st.back();
            int w  = i - left - 1;               // 宽度
            int hh = min(height[left], height[i]) - height[bottom];   // 水高
            water += w * hh;
        }
        st.push_back(i);
    }
    return water;
}

// ============================================================
// ⑥ 对比：LC 1475 商品折扣后的最终价格（"下一个更小或相等"）
// ============================================================
vector<int> finalPrices(vector<int>& prices) {
    vector<int> ans = prices, st;
    for (int i = 0; i < (int)prices.size(); ++i) {
        while (!st.empty() && prices[st.back()] >= prices[i]) {   // 注意 >=：相等也打折
            ans[st.back()] = prices[st.back()] - prices[i];
            st.pop_back();
        }
        st.push_back(i);
    }
    return ans;
}

// ============================================================
// ⑦ LC 907 子数组的最小值之和（贡献法）
//    对每个 i，数出"有多少个子数组的最小值恰好是 arr[i]"，再乘 arr[i]。
//    左边界用【严格小于】、右边界用【小于等于】—— 这样相等元素只被数一次。
// ============================================================
int sumSubarrayMins(vector<int>& arr) {
    const long long MOD = 1000000007LL;
    int n = (int)arr.size();
    vector<int> left(n, 0), right(n, 0), st;

    for (int i = 0; i < n; ++i) {                       // 左边第一个"严格小于"
        while (!st.empty() && arr[st.back()] >= arr[i]) st.pop_back();
        left[i] = st.empty() ? i + 1 : i - st.back();
        st.push_back(i);
    }
    st.clear();
    for (int i = n - 1; i >= 0; --i) {                  // 右边第一个"小于等于"
        while (!st.empty() && arr[st.back()] > arr[i]) st.pop_back();
        right[i] = st.empty() ? n - i : st.back() - i;
        st.push_back(i);
    }

    long long ans = 0;
    for (int i = 0; i < n; ++i) {
        ans = (ans + (long long)arr[i] * left[i] * right[i]) % MOD;
    }
    return (int)ans;
}

// ============================================================
// ⑧ 对比：LC 239 滑动窗口最大值 —— 这是单调【队列】
//    和单调栈唯一的区别：窗口左边界离开时，要从队头弹出去。
// ============================================================
vector<int> maxSlidingWindow(vector<int>& nums, int k) {
    deque<int> dq;                      // 存下标，值单调递减
    vector<int> ans;
    for (int i = 0; i < (int)nums.size(); ++i) {
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();        // 队头滑出窗口
        if (i >= k - 1) ans.push_back(nums[dq.front()]);
    }
    return ans;
}

// ============================================================
// 暴力版（对拍用，全部 O(n^2) 或更慢）
// ============================================================
vector<int> dailyTemperaturesBrute(vector<int>& T) {
    int n = (int)T.size();
    vector<int> ans(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (T[j] > T[i]) { ans[i] = j - i; break; }
    return ans;
}

vector<int> nextGreaterElementsIIBrute(vector<int>& nums) {
    int n = (int)nums.size();
    vector<int> ans(n, -1);
    for (int i = 0; i < n; ++i)
        for (int d = 1; d < n; ++d)
            if (nums[(i + d) % n] > nums[i]) { ans[i] = nums[(i + d) % n]; break; }
    return ans;
}

int largestRectangleAreaBrute(vector<int>& heights) {
    int n = (int)heights.size(), best = 0;
    for (int i = 0; i < n; ++i) {
        int lo = i, hi = i;
        while (lo - 1 >= 0 && heights[lo - 1] >= heights[i]) --lo;
        while (hi + 1 < n && heights[hi + 1] >= heights[i]) ++hi;
        best = max(best, heights[i] * (hi - lo + 1));
    }
    return best;
}

int trapBrute(vector<int>& height) {
    int n = (int)height.size(), water = 0;
    for (int i = 1; i + 1 < n; ++i) {
        int l = 0, r = 0;
        for (int j = 0; j <= i; ++j) l = max(l, height[j]);
        for (int j = i; j < n; ++j) r = max(r, height[j]);
        water += min(l, r) - height[i];
    }
    return water;
}

vector<int> finalPricesBrute(vector<int>& prices) {
    int n = (int)prices.size();
    vector<int> ans = prices;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (prices[j] <= prices[i]) { ans[i] = prices[i] - prices[j]; break; }
    return ans;
}

int sumSubarrayMinsBrute(vector<int>& arr) {
    const long long MOD = 1000000007LL;
    int n = (int)arr.size();
    long long ans = 0;
    for (int i = 0; i < n; ++i) {
        int mn = arr[i];
        for (int j = i; j < n; ++j) {
            mn = min(mn, arr[j]);
            ans = (ans + mn) % MOD;
        }
    }
    return (int)ans;
}

vector<int> maxSlidingWindowBrute(vector<int>& nums, int k) {
    vector<int> ans;
    for (int i = 0; i + k <= (int)nums.size(); ++i) {
        int mx = nums[i];
        for (int j = i; j < i + k; ++j) mx = max(mx, nums[j]);
        ans.push_back(mx);
    }
    return ans;
}

// ============================================================
// 测试框架
// ============================================================
static int failed = 0;

string show(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); ++i) { if (i) s += ","; s += to_string(v[i]); }
    return s + "]";
}

void checkVec(const string& name, const vector<int>& got, const vector<int>& want) {
    bool ok = (got == want);
    if (!ok) failed++;
    cout << (ok ? "[PASS] " : "[FAIL] ") << name
         << "  期望 " << show(want) << " 实得 " << show(got) << "\n";
}

void checkInt(const string& name, int got, int want) {
    bool ok = (got == want);
    if (!ok) failed++;
    cout << (ok ? "[PASS] " : "[FAIL] ") << name
         << "  期望 " << want << " 实得 " << got << "\n";
}

// ============================================================
// 随机对拍
// ============================================================
static unsigned long long rngState = 88172645463325252ULL;
unsigned long long rnd() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}
int rndInt(int lo, int hi) { return lo + (int)(rnd() % (unsigned long long)(hi - lo + 1)); }

void randomCrossCheck(int rounds) {
    int bad = 0;
    for (int r = 0; r < rounds; ++r) {
        int n = rndInt(0, 12);
        vector<int> a(n);
        for (int& x : a) x = rndInt(0, 9);        // 值域故意很小，制造大量重复元素

        vector<int> t1 = dailyTemperatures(a), t2 = dailyTemperaturesBrute(a);
        vector<int> g1 = nextGreaterElementsII(a), g2 = nextGreaterElementsIIBrute(a);
        vector<int> p1 = finalPrices(a), p2 = finalPricesBrute(a);
        int r1 = largestRectangleArea(a), r2 = largestRectangleAreaBrute(a);
        int w1 = trap(a), w2 = trapBrute(a);
        int s1 = sumSubarrayMins(a), s2 = sumSubarrayMinsBrute(a);

        bool ok = (t1 == t2) && (g1 == g2) && (p1 == p2)
                  && (r1 == r2) && (w1 == w2) && (s1 == s2);

        if (!ok) {
            bad++;
            if (bad <= 3) {
                cout << "[DIFF] 输入 " << show(a) << "\n"
                     << "  每日温度  " << show(t1) << " vs " << show(t2) << "\n"
                     << "  循环下一个更大 " << show(g1) << " vs " << show(g2) << "\n"
                     << "  最大矩形 " << r1 << " vs " << r2 << "\n"
                     << "  接雨水 " << w1 << " vs " << w2 << "\n";
            }
        }
    }
    if (bad) failed++;
    cout << (bad == 0 ? "[PASS] " : "[FAIL] ")
         << "随机对拍 " << rounds << " 组（数据 0~9，专挑重复元素）\n";
}

// ============================================================
int main() {
    cout << "=== 单调栈 模板测试 ===\n\n";

    // ---- 题面示例 ----
    {
        vector<int> t = {73, 74, 75, 71, 69, 72, 76, 73};
        checkVec("739 每日温度", dailyTemperatures(t), {1, 1, 4, 2, 1, 1, 0, 0});
    }
    {
        vector<int> a = {4, 1, 2}, b = {1, 3, 4, 2};
        checkVec("496 下一个更大 I", nextGreaterElement(a, b), {-1, 3, -1});
    }
    {
        vector<int> a = {1, 2, 1};
        checkVec("503 循环下一个更大", nextGreaterElementsII(a), {2, -1, 2});
    }
    {
        vector<int> h = {2, 1, 5, 6, 2, 3};
        checkInt("84 最大矩形", largestRectangleArea(h), 10);
    }
    {
        vector<int> h = {0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1};
        checkInt("42 接雨水", trap(h), 6);
    }
    {
        vector<int> p = {8, 4, 6, 2, 3};
        checkVec("1475 打折后价格", finalPrices(p), {4, 2, 4, 2, 3});
    }
    {
        vector<int> a = {3, 1, 2, 4};
        checkInt("907 子数组最小值之和", sumSubarrayMins(a), 17);
    }
    {
        vector<int> a = {1, 3, -1, -3, 5, 3, 6, 7};
        checkVec("239 滑动窗口最大值", maxSlidingWindow(a, 3), {3, 3, 5, 5, 6, 7});
    }

    // ---- 边界 ----
    {
        vector<int> e = {};
        checkVec("空数组 每日温度", dailyTemperatures(e), {});
        checkInt("空数组 最大矩形", largestRectangleArea(e), 0);
        checkInt("空数组 接雨水", trap(e), 0);
    }
    {
        vector<int> h = {5};
        checkInt("单柱 最大矩形", largestRectangleArea(h), 5);
        checkInt("单柱 接雨水", trap(h), 0);
    }
    {
        vector<int> h = {2, 2};                 // 相等元素：最大矩形是 2*2 = 4
        checkInt("等高的两根柱子", largestRectangleArea(h), 4);
    }
    {
        vector<int> a = {5, 5, 5};
        checkVec("全相等 每日温度", dailyTemperatures(a), {0, 0, 0});
        checkInt("全相等 接雨水", trap(a), 0);
    }

    cout << "\n";
    randomCrossCheck(5000);

    cout << "\n";
    if (failed == 0) cout << "全部通过 ✓\n";
    else             cout << failed << " 项失败 ✗\n";
    return failed == 0 ? 0 : 1;
}
