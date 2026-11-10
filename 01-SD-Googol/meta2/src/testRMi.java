import rmi.GatewayInterface;
import system.Search_result;

import java.rmi.Naming;
import java.util.Properties;
import java.io.FileInputStream;

public class testRMi {

    public static void main(String[] args) throws Exception {
        //le config?
        //TODO:AJUSTAR PATH
        Properties config = new Properties();
        config.load(new FileInputStream("source/config/config.properties"));

        String host = config.getProperty("registry.host", "localhost");
        String port = config.getProperty("registry.port", "1099");
        String gatewayName = config.getProperty("gateway.name", "Gateway");

        String url = "rmi://" + host + ":" + port + "/" + gatewayName;
        System.out.println("[META2] A ligar à Gateway em " + url);

        //lookup RMI
        GatewayInterface gateway = (GatewayInterface) Naming.lookup(url);

        //faz pesquisa
        Search_result r = gateway.search("universidade", 0);

        System.out.println("Resultados: " + r.items.size());
        r.items.forEach(item -> {
            System.out.println("- " + item.title + " -> " + item.url);
        });
    }
}
