package leetcode;

/**
 * 198. 打家劫舍
 * https://leetcode-cn.com/problems/house-robber
 * 你是一个专业的小偷，计划偷窃沿街的房屋。每间房内都藏有一定的现金，
 * 影响你偷窃的唯一制约因素就是相邻的房屋装有相互连通的防盗系统，
 * 如果两间相邻的房屋在同一晚上被小偷闯入，系统会自动报警。
 *
 * 给定一个代表每个房屋存放金额的非负整数数组，计算你 不触动警报装置的情况下 ，
 * 一夜之内能够偷窃到的最高金额。
 *
 * @author zhangxiaofan 2021/03/03-10:25
 */
public class Q198HouseRobber {

    /**
     * index 当前需要判断 抢与不抢 的下标
     */
    public static int houseRobber0(int[] arr, int index) {
        if (arr == null)
            return 0;
        if (index >= arr.length)
            return 0;

        int rs = Math.max(
                houseRobber0(arr, index + 1), // 不抢
                arr[index] + houseRobber0(arr, index + 2)  // 抢
        );
        return rs;
    }

    /**
     * 主函数
     */
    public static int houseRobber(int[] arr) {
        return houseRobber0(arr, 0);
    }

    /**
     * 动态规划求打家劫舍
     *
     * dp[i] = max(dp[i + 1], arr[i] + dp[i + 2])
     */
    public static int houseRobberDP(int[] arr) {
        int[] dp = new int[arr.length + 2];
        // base case
        dp[arr.length] = 0;
        for (int i = arr.length - 1; i >= 0; i--) {
            dp[i] = Math.max(dp[i + 1], arr[i] + dp[i + 2]);
        }
        return dp[0];
    }

    public static void main(String[] args) {
        int[] arr = new int[]{1,2,3,1};
        int i = houseRobber(arr);
        System.out.println(i);

        int i1 = houseRobberDP(arr);
        System.out.println(i1);
    }

}
