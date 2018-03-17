package com.java.network;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class PooledWebLog {
    private final static int NUM_THREADS = 4;

    public static void main(String[] args) throws IOException {
        ExecutorService executor = Executors.newFixedThreadPool(NUM_THREADS);
        Queue<LogEntry> results = new LinkedList<LogEntry>();
        try (BufferedReader in = new BufferedReader(
            new InputStreamReader(new FileInputStream(args[0]), "UTF-8"));) {
            String entry = in.readLine();
            for (;entry != null; entry = in.readLine()) {
                LookupTask task = new LookupTask(entry);
                Future<String> future = executor.submit(task);
                LogEntry result = new LogEntry(entry, future);
                results.add(result);
            }
        }

        // Start printing the results. This blocks each time a result isn't ready.
        // The output code can use a separate thread.
        for (LogEntry result : results) {
            try {
                System.out.println(result.future.get());
            } catch (InterruptedException | ExecutionException e) {
                System.out.println(result.original);
            }
        }

        executor.shutdown();
    }

    private static class LogEntry {
        String original;
        Future<String> future;

        public LogEntry(String original, Future<String> future) {
            this.original = original;
            this.future = future;
        }
    }
}
