package system;

import java.io.Serializable;
import java.util.*;

public class Statistics implements Serializable {
    private static final long serialVersionUID = 1L;

    // top queries
    private final Map<String, Integer> queryFreq = new HashMap<String, Integer>();
    // media de respostas por barrel
    private final Map<String, List<Long>> barrelTimes = new HashMap<String, List<Long>>();
    // mapa barrel -> tamanho índice
    private final Map<String, Integer> barrelSizes = new HashMap<String, Integer>();

    public synchronized void addQuery(String q) {
        Integer count = queryFreq.get(q);
        if (count == null) count = 0;
        queryFreq.put(q, count + 1);
    }

    public synchronized void addLatency(String barrel, long millis) {
        List<Long> l = barrelTimes.get(barrel);
        if (l == null) {
            l = new ArrayList<Long>();
            barrelTimes.put(barrel, l);
        }
        l.add(millis);
    }

    public synchronized void setBarrelSize(String barrel, int size) {
        barrelSizes.put(barrel, size);
    }

    public synchronized Map<String, Object> snapshot() {
        Map<String, Object> s = new LinkedHashMap<String, Object>();

        // top 10 queries
        List<Map.Entry<String, Integer>> top = new ArrayList<Map.Entry<String, Integer>>(queryFreq.entrySet());
        Collections.sort(top, new Comparator<Map.Entry<String, Integer>>() {
            public int compare(Map.Entry<String, Integer> a, Map.Entry<String, Integer> b) {
                return Integer.compare(b.getValue(), a.getValue());
            }
        });
        List<Map.Entry<String, Integer>> top10 = top.size() > 10 ? top.subList(0, 10) : top;

        List<Map<String, Object>> topList = new ArrayList<Map<String, Object>>();
        for (Map.Entry<String, Integer> e : top10) {
            Map<String, Object> m = new LinkedHashMap<String, Object>();
            m.put("query", e.getKey());
            m.put("count", e.getValue());
            topList.add(m);
        }
        s.put("topQueries", topList);

        // medias por barrel
        Map<String, Double> avg = new LinkedHashMap<String, Double>();
        for (Map.Entry<String, List<Long>> e : barrelTimes.entrySet()) {
            List<Long> l = e.getValue();
            double m = 0.0;
            if (!l.isEmpty()) {
                long sum = 0;
                for (Long val : l) sum += val;
                m = ((double) sum) / l.size();
            }
            // decimas de segundo
            avg.put(e.getKey(), m / 100.0);
        }

        s.put("avgDeciSecondsByBarrel", avg);
        s.put("barrelSizes", new LinkedHashMap<String, Integer>(barrelSizes));
        return s;
    }
}
