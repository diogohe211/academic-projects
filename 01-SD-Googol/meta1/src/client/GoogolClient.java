package client;

import java.rmi.Naming;
import java.rmi.RemoteException;
import java.util.*;
import java.io.FileInputStream;
import java.io.IOException;
import java.rmi.NotBoundException;

import rmi.GatewayInterface;
import system.Search_result;

public class GoogolClient {

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        try {
            // ler config
            Properties config = new Properties();
            config.load(new FileInputStream("config/config.properties"));

            String host = config.getProperty("registry.host", "localhost");
            String port = config.getProperty("registry.port", "1099");
            String gatewayName = config.getProperty("gateway.name", "Gateway");

            // contruir url rmi
            String url = "rmi://" + host + ":" + port + "/" + gatewayName;

            // ligar a gateway
            GatewayInterface gateway = (GatewayInterface) Naming.lookup(url);
            System.out.println("Ligado à Gateway em " + url);

            String ultimaquery = null;
            int pagina = 0;
            
             while (true) {
                System.out.println("\n======= MENU GOOGL CLIENT =======");
                System.out.println("1. Submeter novo Url");
                System.out.println("2. Fazer pesquisa");
                System.out.println("3. Ver backlinks");
                System.out.println("4. Ver estatisticas");
                System.out.println("5. SAIR");
                System.out.print("Escolha uma opcao: ");
                
                String opcao = sc.nextLine().trim();

                if (opcao.equals("1")) {
                    System.out.print("Introduza o Url: ");
                    String urltosubmit = sc.nextLine().trim();
                    if (!urltosubmit.isEmpty()) {
                        gateway.submitUrl(urltosubmit);
                        System.out.println("[OK] Url submetido com sucesso!");
                    } else {
                        System.out.println("[!] Url vazio — ignorado.");
                    }

                } else if (opcao.equals("2")) {
                    System.out.print("Introduza a pesquisa: ");
                    String query = sc.nextLine().trim();
                    ultimaquery = query;
                    pagina = 0;

                    while (true) {
                        Search_result r = gateway.search(query, pagina);
                        System.out.println("\n=== Resultados (Pagina " + (pagina+1) + ") ===");
                        if (r.items.isEmpty()) {
                            System.out.println("nenhum resultado encontrado.");
                        } else {
                            for (int i = 0; i < r.items.size(); i++) {
                                System.out.println((i + 1) + ". " + r.items.get(i).title + " → " + r.items.get(i).url);
                                System.out.println("   " + r.items.get(i).snippet + "\n");
                            }
                        }
                        System.out.print("\n[n] Proxima pagina | [b] Voltar menu: ");
                        String next = sc.nextLine().trim().toLowerCase();
                        if (next.equals("n")) pagina++;
                        else break;
                    }

                } else if (opcao.equals("3")) {
                    System.out.print("Introduza o Url: ");
                    String target = sc.nextLine().trim();
                    List<String> bks = gateway.backlinks(target);
                    System.out.println("\n=== Backlinks Encontrados ===");
                    if (bks.isEmpty()) System.out.println("(nenhum)");
                    else bks.forEach(l -> System.out.println(" - " + l));

                } else if (opcao.equals("4")) {
                    Map<String, Object> stats = gateway.stats();
                    System.out.println("\n=== Estatisticas do Sistema ===");
                    for (Map.Entry<String, Object> e : stats.entrySet()) {
                        System.out.println("• " + e.getKey() + ": " + e.getValue());
                    }

                } else if (opcao.equals("5")) {
                    System.out.println("A terminar Cliente...");
                    break;

                } else {
                    System.out.println("[!] Opcão invalida. Tentar novamente.");
                }
            }

        } catch (NotBoundException e) {
            System.err.println("[Erro] Gateway nao encontrada no registry: " + e.getMessage());
        } catch (RemoteException e) {
            System.err.println("[Erro RMI] " + e.getMessage());
        } catch (IOException e) {
            System.err.println("[Erro IO] Falha ao carregar config.properties");
        } catch (Exception e) {
            e.printStackTrace();
        }

        System.out.println("Cliente terminado.");
    }
}
