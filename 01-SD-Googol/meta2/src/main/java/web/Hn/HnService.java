package web.hn;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Service;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

@Service
public class HnService {
    private static final String BASE = "https://hacker-news.firebaseio.com/v0";
    private final ObjectMapper om = new ObjectMapper();

    public List<HnItem> top(int n) throws Exception {
        List<Long> ids = topIds();
        int limit = Math.min(n, ids.size());

        List<HnItem> out = new ArrayList<>();
        for (int i = 0; i < limit; i++) {
            try {
                HnItem item = item(ids.get(i));
                if (item != null) out.add(item);
            } catch (Exception ignored) {
                // se 1 item falhar não rebenta a pag toda
            }
        }
        return out;
    }

    private List<Long> topIds() throws Exception {
        String json = get(BASE + "/topstories.json");
        JsonNode node = om.readTree(json);

        List<Long> ids = new ArrayList<>();
        if (node.isArray()) {
            for (JsonNode x : node) ids.add(x.asLong());
        }
        return ids;
    }

    private HnItem item(long id) throws Exception {
        String json = get(BASE + "/item/" + id + ".json");
        JsonNode n = om.readTree(json);
        if (n == null || n.isNull()) return null;

        HnItem it = new HnItem();
        it.id = id;
        it.title = n.path("title").asText("");
        it.by = n.path("by").asText("");
        it.score = n.path("score").asInt(0);
        it.url = n.path("url").asText("https://news.ycombinator.com/item?id=" + id);
        it.time = n.path("time").asLong(0);
        return it;
    }

    private String get(String u) throws Exception {
        URL url = new URL(u);
        HttpURLConnection con = (HttpURLConnection) url.openConnection();
        con.setRequestMethod("GET");
        con.setConnectTimeout(5000);
        con.setReadTimeout(7000);

        int code = con.getResponseCode();
        if (code < 200 || code >= 300) {
            throw new RuntimeException("HTTP " + code + " em " + u);
        }

        try (BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream(), "UTF-8"))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line);
            return sb.toString();
        }
    }
}
