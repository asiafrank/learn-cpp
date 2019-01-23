package crls;

import org.junit.jupiter.api.Test;

/**
 * @author zhangxf created at 12/5/2018.
 */
public class DPTest {

    @Test
    public void matrixChainOrderTest() {
        int[] p = {5, 10, 3, 12, 5, 50, 6};
        DP.Pair pair = DP.matrixChainOrder(p);
        DP.printOptimalParens(pair.s, 1, pair.s.length);
        printArray(pair.m);
        System.out.println();
        printArray(pair.s);
    }

    @Test
    public void lcsLengthTest() {
        int[] x = {'A', 'B', 'C', 'B', 'D', 'A', 'B'};
        int[] y = {'B', 'D', 'C', 'A', 'B', 'A'};
        DP.LCSPair pair = DP.lcsLength(x, y);
        printArray(pair.c);
        printArray(pair.b);
        DP.printLCS(pair.b, x, x.length - 1, y.length - 1);
        System.out.println();
    }

    private void printArray(int[][] a) {
        for (int i = 0; i < a.length; i++) {
            for (int j = 0; j < a[i].length; j++) {
                System.out.print(a[i][j] + " ");
            }
            System.out.println();
        }
    }

    private void printArray(char[][] a) {
        for (int i = 0; i < a.length; i++) {
            for (int j = 0; j < a[i].length; j++) {
                System.out.print(a[i][j] + " ");
            }
            System.out.println();
        }
    }
}
