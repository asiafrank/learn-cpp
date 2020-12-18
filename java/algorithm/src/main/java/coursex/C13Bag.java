package coursex;

/**
 * 从左往右尝试模型
 * <p>
 * 01背包问题
 * 给定两个长度都为 N 的数组 weights 和 values，
 * weights[i] 和 values[i] 分别代表 i 号物品的重量和价值。
 * 给定一个正数 bag，表示一个载重bag的袋子，
 * 你装的物品不能超过这个重量。
 * 返回你能装下最多的价值是多少?
 */
public class C13Bag {

    /**
     * 暴力递归处理 01 背包
     *
     * @param weight    重量数组
     * @param value     价值数组
     * @param bagWeight 背包容量
     * @return 背包所能装下的最大价值
     */
    public static int bag1(int[] weight, int[] value, int bagWeight) {
        return recursive(weight, value, 0, bagWeight);
    }

    /**
     * 暴力递归版本，自顶向下
     * <p>
     * 1.选择当前物品时，计算 rest - weight[i] 的最大价值
     * 2.不选择当前物品时，计算 rest 的最大价值
     * 3.上面两者取最大值
     *
     * @param weight 重量数组
     * @param value  价值数组
     * @param i      当前物品编号。数组下标
     * @param rest   背包剩下多少重量可以装
     * @return rest 重量 所能装的价值
     */
    public static int recursive(int[] weight, int[] value, int i, int rest) {
        if (i >= weight.length)
            return 0;
        if (rest <= 0)
            return 0;

        int delta = rest - weight[i];
        if (delta < 0) { // 不能选择的情况
            return recursive(weight, value, i + 1, rest);
        } else { // 可以选择；选择与不选择，比大小
            int choose = value[i] + recursive(weight, value, i + 1, delta);
            int noChoose = recursive(weight, value, i + 1, rest);
            return Math.max(choose, noChoose);
        }
    }

    //.....省略记忆法....

    /**
     * 动态规划，自底向上
     * @param weight    重量数组
     * @param value     价值数组
     * @param bagWeight 背包容量
     * @return 背包所能装下的最大价值
     */
    public static int bag2(int[] weight, int[] value, int bagWeight) {
        if (bagWeight <= 0)
            return 0;

        // dp[i][X] = V 含义 以 i 为止的物品放入 X 重量的背包，最大能装 V 价值
        int[][] dp = new int[weight.length][bagWeight + 1];
        for (int i = 0; i < weight.length; i++) {
            dp[i][0] = 0;
        }

        for (int i = 1; i < weight.length; i++) {
            for (int bw = 1; bw <= bagWeight; bw++) {
                int w = weight[i];
                int rest = bw - w;
                if (rest < 0) { // 物品重量大于背包重量，则无法选择
                    dp[i][bw] = dp[i - 1][bw];
                } else {
                    // 不选择: dp[i - 1][bw]
                    // 选择：dp[i][bw - w] + value[i]
                    // 哪个大取哪个
                    dp[i][bw] = Math.max(dp[i - 1][bw], dp[i][bw - w] + value[i]);
                }
            }
        }
        return dp[weight.length - 1][bagWeight];
    }

    public static void main(String[] args) {
        int[] weight = new int[]{1, 2, 3, 4, 5};
        int[] value = new int[]{1, 2, 3, 4, 5};
        int bag = 6; // [2,4] 是最大
        int v = bag1(weight, value, bag);
        int v2 = bag2(weight, value, bag);
        System.out.println(v);
        System.out.println(v2);
    }
}