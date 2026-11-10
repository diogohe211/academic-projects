package barrel;

import rmi.BarrelInterface;
import system.DocumentData;
import system.Search_result;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class Storage_barel extends UnicastRemoteObject implements BarrelInterface {

    private static final long serialVersionUID = 1L;

    // indice invertido: termo -> conjunto de URLs
    private final Map<String, Set<String>> index = new ConcurrentHashMap<String, Set<String>>();
    // dados do documento
    private final Map<String, DocumentData> docs = new ConcurrentHashMap<String, DocumentData>();
    // backlinks: url -> páginas que apontam para ela
    private final Map<String, Set<String>> backlinks = new ConcurrentHashMap<String, Set<String>>();

    //################### construtor padrão
    public Storage_barel() throws RemoteException {
        super();
        System.out.println("[Storage_barel] Barrel iniciado e pronto para receber dados.");
    }

    //################### sincronizado para eivtar concorrencia
    @Override
    public synchronized void upsertDocument(DocumentData doc) throws RemoteException {
        if (doc == null || doc.url == null) return;

        // DEBUG: verifica documento recebido
        System.out.println("[Barrel] Documento recebido: " + doc.url + 
                           " | termos=" + (doc.terms != null ? doc.terms.size() : 0));

        // -rm entradas antifas
        if (docs.containsKey(doc.url)) {
            DocumentData old = docs.get(doc.url);
            if (old != null && old.terms != null) {
                for (String t : old.terms) {
                    Set<String> s = index.get(t);
                    if (s != null) {
                        s.remove(doc.url);
                        if (s.isEmpty()) index.remove(t);
                    }
                }
            }
        }

        docs.put(doc.url, doc);

        // add ao indice
        if (doc.terms != null) {
            for (String t : doc.terms) {
                // normalizar o termo (lower case)
                t = t.toLowerCase();
                Set<String> urls = index.get(t);
                if (urls == null) {
                    urls = Collections.newSetFromMap(new ConcurrentHashMap<String, Boolean>());
                    index.put(t, urls);
                }
                urls.add(doc.url);
            }
        }

        System.out.println("[Barrel] Total de documentos indexados: " + docs.size());
    }

    //################### registerOutLinks implementei sincronizaçãp para evitar erros
    @Override
    public synchronized void registerOutLinks(String fromUrl, List<String> toUrls) throws RemoteException {
        if (fromUrl == null || toUrls == null) return;
        for (String to : toUrls) {
            if (to == null || to.trim().isEmpty()) continue;
            Set<String> set = backlinks.get(to);
            if (set == null) {
                set = Collections.newSetFromMap(new ConcurrentHashMap<String, Boolean>());
                backlinks.put(to, set);
            }
            set.add(fromUrl);
        }
    }

    //################### pesquisa atraves de termos
    @Override
    public synchronized Search_result search(List<String> terms, int page, int pageSize) throws RemoteException {
        if (terms == null || terms.isEmpty()) {
            return new Search_result(page, pageSize, 0, new ArrayList<Search_result.Item>());
        }

        // DEBUG: log do pedido de pesquisa
        System.out.println("[Barrel] Pesquisa recebida: " + terms);
        System.out.println("[Barrel] Tamanho do índice: " + index.size() + ", docs: " + docs.size());

        // normalizar termos de pesquisa
        terms = terms.stream()
                     .filter(t -> t != null && !t.trim().isEmpty())
                     .map(String::toLowerCase)
                     .collect(Collectors.toList());

        // calcular uniao (em vez de interseçao) -> procura menos restritiva 
        Set<String> candidate = new HashSet<String>();
        for (String term : terms) {
            Set<String> s = index.get(term);
            if (s != null) candidate.addAll(s);
        }

        // DEBUG: total de candidatos
        System.out.println("[Barrel] URLs candidatos encontrados: " + candidate.size());

        // ranking por nº de backlinks
        List<String> ordered = new ArrayList<String>(candidate);
        Collections.sort(ordered, new Comparator<String>() {
            public int compare(String a, String b) {
                int sizeA = backlinks.containsKey(a) ? backlinks.get(a).size() : 0;
                int sizeB = backlinks.containsKey(b) ? backlinks.get(b).size() : 0;
                return Integer.compare(sizeB, sizeA);
            }
        });

        int total = ordered.size();
        int from = Math.max(0, Math.min(page * pageSize, total));
        int to = Math.max(from, Math.min(from + pageSize, total));

        List<Search_result.Item> items = new ArrayList<Search_result.Item>();
        for (int i = from; i < to; i++) {
            String url = ordered.get(i);
            DocumentData d = docs.get(url);
            int in = backlinks.containsKey(url) ? backlinks.get(url).size() : 0;
            String title = (d == null || d.title == null) ? url : d.title;
            String snippet = (d == null || d.snippet == null) ? "" : d.snippet;
            items.add(new Search_result.Item(url, title, snippet, in));
        }

        // DEBUG:num final de resultados
        System.out.println("[Barrel] Pesquisa concluída: " + items.size() + " resultados devolvidos.");

        return new Search_result(page, pageSize, total, items);
    }

    @Override
    public synchronized List<String> getBacklinks(String url) throws RemoteException {
        if (url == null) return new ArrayList<String>();
        Set<String> s = backlinks.get(url);
        if (s == null) return new ArrayList<String>();
        return new ArrayList<String>(s);
    }

    //################### heartbeat usado pela gateway
    @Override
    public synchronized int getIndexSize() throws RemoteException {
        return docs.size();
    }

    //################### stats gerais
    @Override
    public synchronized Map<String, Object> getBarrelStats() throws RemoteException {
        Map<String, Object> m = new LinkedHashMap<String, Object>();
        m.put("documents", docs.size());
        m.put("terms", index.size());
        m.put("backlinks", backlinks.size());
        return m;
    }

    // ##########################
    // main para iniciar o barrel sozinho
    // ##########################
    public static void main(String[] args) {
        try {
            // nome do barrel if null==barrel1
            String name = (args.length > 0 && args[0] != null && !args[0].trim().isEmpty())
                            ? args[0].trim()
                            : "Barrel1";

            // config host e porta
            String host = "localhost";
            int port = 1099;
            String url = "rmi://" + host + ":" + port + "/" + name;

            // cria o novo barrel
            Storage_barel barrel = new Storage_barel();
            java.rmi.Naming.rebind(url, barrel);

            // info
            System.out.println("=======================================");
            System.out.println("[Storage_barel] " + name + " iniciado e pronto para receber dados.");
            System.out.println("[Storage_barel] Registado em " + url);
            System.out.println("[Storage_barel] Pressiona CTRL+C para terminar.");
            System.out.println("=======================================");

        } catch (java.rmi.ConnectException e) {
            System.err.println("[Storage_barel] Falha ao ligar ao registry RMI — está em execução?");
            e.printStackTrace();
        } catch (java.rmi.RemoteException e) {
            System.err.println("[Storage_barel] Erro remoto: " + e.getMessage());
            e.printStackTrace();
        } catch (Exception e) {
            System.err.println("[Storage_barel] Erro inesperado: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
