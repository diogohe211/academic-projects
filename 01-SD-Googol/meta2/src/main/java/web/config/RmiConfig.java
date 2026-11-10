package web.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import rmi.GatewayInterface;

import java.io.InputStream;
import java.rmi.Naming;
import java.util.Properties;

@Configuration
public class RmiConfig {

    @Bean
    public GatewayInterface gatewayInterface() throws Exception {
        // le config/config.properties do classpath
        Properties config = new Properties();
        try (InputStream in = getClass().getClassLoader()
                .getResourceAsStream("config/config.properties")) {
            if (in == null) {
                throw new IllegalStateException("config/config.properties não encontrado no classpath");
            }
            config.load(in);
        }

        String host = config.getProperty("registry.host", "localhost");
        String port = config.getProperty("registry.port", "1099");
        String gatewayName = config.getProperty("gateway.name", "Gateway");

        String url = "rmi://" + host + ":" + port + "/" + gatewayName;
        System.out.println("[WEB] A ligar à Gateway em " + url);

        return (GatewayInterface) Naming.lookup(url);
    }
}
