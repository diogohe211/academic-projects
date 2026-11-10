package system;


import java.util.concurrent.ConcurrentLinkedQueue;


public class UrlQueue {
private static final ConcurrentLinkedQueue<String> queue = new ConcurrentLinkedQueue<>();


public static void add(String url) {
queue.add(url);
}


public static String poll() {
return queue.poll();
}


public static boolean isEmpty() {
return queue.isEmpty();
}
}