package downloader;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import rmi.BarrelInterface;
import rmi.GatewayInterface;
import system.DocumentData;

import java.util.*;
import java.util.concurrent.TimeUnit;

public class Downloader implements Runnable {

    private final GatewayInterface gateway;              // comunicação com a gateway
    private final List<BarrelInterface> barrels;         // lista de barrels (para difundir resultados)

    // <Limite para nao fritar>
    private static final int MAX_URLS = 50;              // num maximo de urls por ciclo
    private static final int SLEEP_AFTER_LIMIT = 10;     // segundos de sonos

    public Downloader(GatewayInterface gateway, List<BarrelInterface> barrels) {
        this.gateway = gateway;
        this.barrels = barrels;
    }

    @Override
    public void run() {
        System.out.println("[Downloader] Thread iniciada. A processar URLs...");
        int processed = 0;

        while (true) {

            // se atingir o limite, descansa
            if (processed >= MAX_URLS) {
                System.out.println("[Downloader] Limite de " + MAX_URLS + " URLs atingido. A aguardar " + SLEEP_AFTER_LIMIT + "s...");
                try {
                    TimeUnit.SECONDS.sleep(SLEEP_AFTER_LIMIT);
                } catch (InterruptedException ignored) {}
                processed = 0;
            }

            String url = null;

            // obter url da gateway RMI
            try {
                url = gateway.pollUrl();
            } catch (java.rmi.RemoteException e) {
                System.err.println("[Downloader] Erro ao contactar a Gateway: " + e.getMessage());
                try {
                    TimeUnit.SECONDS.sleep(2);
                } catch (InterruptedException ignored) {}
                continue;
            }

            // caso nao haja dormir uma sesta
            if (url == null) {
                try {
                    TimeUnit.MILLISECONDS.sleep(200);
                } catch (InterruptedException ignored) {}
                continue;
            }

            processed++;
            System.out.println("[Downloader] (" + processed + "/" + MAX_URLS + ") A processar: " + url);

            try {
                // download do html
                Document doc = Jsoup.connect(url).get();
                String title = doc.title();
                String text = doc.text();

                // extrair conteudo
                Set<String> terms = new HashSet<String>();
                String[] tokens = text.toLowerCase().split("[^a-z0-9á-úç]+");
                for (String tok : tokens) {
                    if (!tok.trim().isEmpty()) terms.add(tok);
                }

                // cria snippet
                String snippet = text.length() > 220 ? text.substring(0, 220) : text;

                // extrair hiperlinks
                List<String> outLinks = new ArrayList<String>();
                Elements links = doc.select("a[href]");
                for (Element e : links) {
                    String abs = e.attr("abs:href");
                    if (!abs.trim().isEmpty()) outLinks.add(abs);
                }

                // TODO:FASE INICIAL DO MULTICAST, DEPOIS CRIAR UMA NOVA CLASSE OU MODULO DE COMMS???
                DocumentData d = new DocumentData(url, title, snippet, terms, outLinks);

                // ################### update :log e difusão para todos os barrels
                System.out.println("[Downloader] Enviando documento para " + barrels.size() + " barrels...");
                for (BarrelInterface b : barrels) {
                    try {
                        b.upsertDocument(d);
                        b.registerOutLinks(url, outLinks);
                    } catch (Exception ex) {
                        System.err.println("[Downloader] Falhou envio ao barrel: " + ex.getMessage());
                    }
                }

                // mando os url de volta
                //FIXME:loop infinito preciso implementar limnite
                for (String u : outLinks) {
                    try {
                        gateway.submitUrl(u);
                    } catch (Exception ex) {
                        System.err.println("[Downloader] Erro ao reenviar URL para Gateway: " + ex.getMessage());
                    }
                }

                // ################### delay para nao saturar a gateway
                try {
                    TimeUnit.MILLISECONDS.sleep(400);
                } catch (InterruptedException ignored) {}

            } catch (Exception e) {
                System.err.println("[Downloader] Erro ao processar " + url + ": " + e.getMessage());
            }
        }
    }

    //main para teste do downloader
    public static void main(String[] args) {
        try {
            System.out.println("[Downloader] A iniciar...");

            // localizar a Gateway via RMI
            String gatewayUrl = "rmi://localhost:1099/Gateway";
            GatewayInterface gatewayRMI = (GatewayInterface) java.rmi.Naming.lookup(gatewayUrl);
            System.out.println("[Downloader] Ligado à Gateway.");

            // localizar barrels
            List<BarrelInterface> barrels = new ArrayList<BarrelInterface>();

            // ################### Alteração 2: adicionar o segundo barrel
            String[] barrelNames = {"Barrel1", "Barrel2"}; // usar ambos os barrels registados
            for (String bname : barrelNames) {
                try {
                    String burl = "rmi://localhost:1099/" + bname;
                    BarrelInterface b = (BarrelInterface) java.rmi.Naming.lookup(burl);
                    barrels.add(b);
                    System.out.println("[Downloader] Ligado ao " + bname);
                } catch (Exception e) {
                    System.err.println("[Downloader] Falhou ligação a " + bname + ": " + e.getMessage());
                }
            }

            if (barrels.isEmpty()) {
                System.err.println("[Downloader] Nenhum barrel encontrado — a terminar.");
                return;
            }

            // ################### delay to make sure barrel e gateway existem
            System.out.println("[Downloader] A aguardar 3s antes de iniciar processamento...");
            TimeUnit.SECONDS.sleep(3);

            // iniciar a thread de download
            Downloader d = new Downloader(gatewayRMI, barrels);
            new Thread(d).start();

        } catch (Exception e) {
            System.err.println("[Downloader] Erro: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
