package gateway;

import rmi.GatewayInterface;
import rmi.BarrelInterface;
import system.Search_result;
import system.Statistics;

import java.rmi.Naming;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.server.UnicastRemoteObject;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.io.FileInputStream;

public class Gateway extends UnicastRemoteObject implements GatewayInterface {

    private static final long serialVersionUID = 1L;

    // lista com os barrels ativos
    private final Map<String, BarrelInterface> barrels = new ConcurrentHashMap<>();
    private final List<String> barrelOrder = Collections.synchronizedList(new ArrayList<>());
    private int rr = 0;
    private final Statistics stats = new Statistics();

    //################### mapa p/ estados (alive/offline)
    private final Map<String, Boolean> barrelAlive = new ConcurrentHashMap<>();

    // fila simples (urls por indexar)
    private final Deque<String> urlQueue = new ArrayDeque<>();//metodo para FIFO

    //carregar config
    private final Properties config = new Properties();

    public Gateway() throws RemoteException {
        super();
        try {
            config.load(new FileInputStream("config/config.properties"));
            initBarrelsFromConfig();
            startHeartbeatMonitor(); //################### inicia verificação periódica
        } catch (Exception e) {
            System.err.println("Erro ao carregar configuração: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /** carrega barrels ja definidos e faz lookup do rmi**/
    private void initBarrelsFromConfig() {
        String host = config.getProperty("registry.host", "localhost");//FIXME: COLOCAR IP ESTATICO
        String port = config.getProperty("registry.port", "1099");
        String[] names = config.getProperty("barrel.names", "").split(",");

        for (String name : names) {
            name = name.trim();
            if (name.isEmpty()) continue;
            try {
                String url = "rmi://" + host + ":" + port + "/" + name;
                BarrelInterface b = (BarrelInterface) Naming.lookup(url);
                barrels.put(name, b);
                barrelOrder.add(name);
                barrelAlive.put(name, Boolean.TRUE); //################### marca barrel como ativo
                System.out.println("[Gateway] Ligado ao barrel " + name);
            } catch (Exception e) {
                barrelAlive.put(name, Boolean.FALSE); //################### inicializa como offline
                System.err.println("[Gateway] Falhou ligação ao barrel " + name + ": " + e.getMessage());
            }
        }
        if (barrels.isEmpty())
            System.err.println("[Gateway] Aviso: nenhum barrel disponível no arranque.");
    }

    public static void main(String[] args) {
        try {
            // carregar config
            Properties config = new Properties();
            config.load(new FileInputStream("config/config.properties"));
            String host = config.getProperty("registry.host", "localhost");
            int port = Integer.parseInt(config.getProperty("registry.port", "1099"));
            String gatewayName = config.getProperty("gateway.name", "Gateway");

            // garantir que o registry está ativo
            try {
                LocateRegistry.createRegistry(port);
                System.out.println("[Gateway] rmiregistry iniciado na porta " + port);
            } catch (RemoteException e) {
                System.out.println("[Gateway] rmiregistry já em execução.");
            }

            // criar e publicar objeto RMI
            Gateway g = new Gateway();
            String url = "rmi://" + host + ":" + port + "/" + gatewayName;
            Naming.rebind(url, g);
            System.out.println("[Gateway] Disponível em " + url);

        } catch (Exception e) {
            System.err.println("Erro ao iniciar Gateway: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // ========================================
    // Métodos remotos
    // ========================================

    @Override
    public synchronized void addBarrel(String name, String rmiUrl) throws RemoteException {
        try {
            BarrelInterface b = (BarrelInterface) Naming.lookup(rmiUrl);
            barrels.put(name, b);
            barrelOrder.add(name);
            barrelAlive.put(name, Boolean.TRUE); //################### marca como ativo
            System.out.println("[Gateway] Barrel " + name + " registado dinamicamente.");
        } catch (Exception e) {
            throw new RemoteException("Erro ao ligar barrel " + name, e);
        }
    }

    @Override
    public synchronized void removeBarrel(String name) throws RemoteException {
        barrels.remove(name);
        barrelOrder.remove(name);
        barrelAlive.remove(name); //################### remove estado também
        System.out.println("[Gateway] Barrel " + name + " removido.");
    }

    @Override
    public synchronized void submitUrl(String url) {
        urlQueue.addLast(url);
        System.out.println("[Gateway] Novo URL recebido: " + url);
    }

     @Override
    public Map<String, Object> stats() {
        return stats.snapshot();
    }

    @Override // método auxiliar para Downloaders
    public synchronized String pollUrl() throws RemoteException{
        return urlQueue.pollFirst();
    }

    //################### Método de escolha de barrel com round-robin + verificação de estado
    private synchronized BarrelInterface pickBarrel(StringBuilder chosenNameOut) throws RemoteException {
        if (barrelOrder.isEmpty())
            throw new RemoteException("Nenhum barrel disponível.");

        int attempts = 0;
        while (attempts < barrelOrder.size()) {
            String name = barrelOrder.get(rr % barrelOrder.size());
            rr++;
            attempts++;

            Boolean alive = barrelAlive.get(name);
            if (alive != null && alive.booleanValue()) {
                chosenNameOut.append(name);
                return barrels.get(name);
            }
        }
        throw new RemoteException("Todos os barrels estão offline.");
    }

    @Override
    public Search_result search(String query, int page) throws RemoteException {
        //DIVIDE A QUERY EM TERMOS
        List<String> terms = Arrays.asList(query.toLowerCase().split("\\s+"));
        List<String> clean = new ArrayList<String>();
        for (String s : terms) {
            if (!s.trim().isEmpty()) clean.add(s);  
        }
        stats.addQuery(String.join(" ", clean));

        StringBuilder chosen = new StringBuilder();
        BarrelInterface b = pickBarrel(chosen);
        long t0 = System.currentTimeMillis();

        try {
            // Pedido normal ao barrel escolhido
            Search_result r = b.search(clean, page, 10);
            long dt = System.currentTimeMillis() - t0;
            stats.addLatency(chosen.toString(), dt);

            // update de stats de tamanho dos indices
            for (Map.Entry<String, BarrelInterface> e : barrels.entrySet()) {
                try {
                    stats.setBarrelSize(e.getKey(), e.getValue().getIndexSize());
                } catch (Exception ignored) {}
            }
            return r;

        } catch (RemoteException e) {
            //################### barrel inativo = failover
            barrelAlive.put(chosen.toString(), Boolean.FALSE);
            System.err.println("[Gateway] Falha no barrel " + chosen + " -> failover para outro.");

            for (int i = 0; i < barrelOrder.size(); i++) {
                StringBuilder alt = new StringBuilder();
                BarrelInterface bb = pickBarrel(alt);
                if (alt.toString().equals(chosen.toString())) continue;
                try {
                    Search_result r = bb.search(clean, page, 10);
                    long dt = System.currentTimeMillis() - t0;
                    stats.addLatency(alt.toString(), dt);
                    return r;
                } catch (Exception ignore) {}
            }
            throw e;
        }
    }

    @Override
    public List<String> backlinks(String url) throws RemoteException {
        StringBuilder chosen = new StringBuilder();
        BarrelInterface b = pickBarrel(chosen);
        try {
            return b.getBacklinks(url);
        } catch (RemoteException e) {
            barrelAlive.put(chosen.toString(), Boolean.FALSE); //################### marca como offline
            for (String name : barrelOrder) {
                if (name.equals(chosen.toString())) continue;
                try {
                    return barrels.get(name).getBacklinks(url);
                } catch (Exception ignored) {}
            }
            throw e;
        }
    }


    //################### thread dos batimentos verifica se ha vivos no meio dos feridos
    private void startHeartbeatMonitor() {
        Thread t = new Thread(new Runnable() {
            public void run() {
                while (true) {
                    try {
                        for (String name : barrelOrder) {
                            BarrelInterface b = barrels.get(name);
                            if (b == null) continue;
                            try {
                                // chama atraves do getIndexSize
                                b.getIndexSize();
                                barrelAlive.put(name, Boolean.TRUE);
                            } catch (Exception ex) {
                                barrelAlive.put(name, Boolean.FALSE);
                            }
                        }
                        Thread.sleep(5000); // verifica a cada 5s
                    } catch (InterruptedException e) {
                        break;
                    }
                }
            }
        });
        t.setDaemon(true);
        t.start();
    }
}
