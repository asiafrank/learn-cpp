// two-sum.cpp : Defines the entry point for the console application.
// https://leetcode-cn.com/problems/two-sum/description/

/*
给定一个整数数组和一个目标值，找出数组中和为目标值的两个数。
你可以假设每个输入只对应一种答案，且同样的元素不能被重复利用。
示例:
给定 nums = [2, 7, 11, 15], target = 9
因为 nums[0] + nums[1] = 2 + 7 = 9
所以返回 [0, 1]
*/

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <map>

using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        // 默认从小到大顺序
        // key: diff, value: index
        vector<int> result;
        map<int, int> diff_map;
        int len = nums.size();
        int x;
        int diff;
        for (int i = 0; i < len; i++)
        {
            x = nums[i];
            auto it = diff_map.find(x);
            if (it == diff_map.end()) 
            {
                diff = target - x;
                diff_map[diff] = i;
            }
            else
            {
                return {it->second, i};
            }
        }
        return {};
    }
};

int main()
{
    cout << "Enter numbers: ";
    string line;
    getline(cin, line);
    istringstream this_line(line);
    istream_iterator<int> begin(this_line), end;
    vector<int> nums(begin, end);

    cout << "Enter target: ";
    int target;
    cin >> target;

    Solution s;
    vector<int> result = s.twoSum(nums, target);
    int len = result.size();
    cout << "Solution: ";
    for (int i = 0; i < len; i++)
    {
        cout << result[i] << " ";
    }
    cout << endl;
    return 0;
}

