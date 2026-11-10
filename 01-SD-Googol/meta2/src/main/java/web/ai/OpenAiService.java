package web.ai;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Service;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;

@Service
public class OpenAiService {

    private final ObjectMapper om = new ObjectMapper();

    public String ask(String userText) throws Exception {
        String apiKey = System.getenv("OPENAI_API_KEY");
        if (apiKey == null || apiKey.trim().isEmpty()) {
            return "OPENAI_API_KEY não está definida no ambiente.";
        }

        // Responses API :contentReference[oaicite:2]{index=2}
        URL url = new URL("https://api.openai.com/v1/responses");
        HttpURLConnection con = (HttpURLConnection) url.openConnection();
        con.setRequestMethod("POST");

        // Bearer auth :contentReference[oaicite:3]{index=3}
        con.setRequestProperty("Authorization", "Bearer " + apiKey);
        con.setRequestProperty("Content-Type", "application/json");
        con.setDoOutput(true);

        
        String body = "{"
                + "\"model\":\"gpt-4o-mini\","
                + "\"input\":[{"
                +   "\"role\":\"user\","
                +   "\"content\":[{"
                +     "\"type\":\"input_text\","
                +     "\"text\":" + om.writeValueAsString(userText)
                +   "}]"
                + "}]"
                + "}";

        try (OutputStream os = con.getOutputStream()) {
            os.write(body.getBytes("UTF-8"));
        }

        int code = con.getResponseCode();
        InputStream is = (code >= 200 && code < 300) ? con.getInputStream() : con.getErrorStream();
        String resp = readAll(is);

        if (code < 200 || code >= 300) {
            // devolve erro 
            return "Erro OpenAI (" + code + "):\n" + resp;
        }

        // extrai o texto do output
        JsonNode root = om.readTree(resp);
        JsonNode out = root.path("output");
        if (out.isArray()) {
            for (JsonNode o : out) {
                JsonNode content = o.path("content");
                if (content.isArray()) {
                    for (JsonNode c : content) {
                        if ("output_text".equals(c.path("type").asText())) {
                            return c.path("text").asText();
                        }
                    }
                }
            }
        }

        return resp; // fallback
    }

    private String readAll(InputStream is) throws Exception {
        if (is == null) return "";
        try (BufferedReader br = new BufferedReader(new InputStreamReader(is, "UTF-8"))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line).append("\n");
            return sb.toString();
        }
    }
}
