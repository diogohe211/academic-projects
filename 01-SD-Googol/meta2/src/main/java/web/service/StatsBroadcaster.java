package web.service;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.rmi.RemoteException;
import java.util.Map;

@Component
public class StatsBroadcaster {

    private final GatewayClientService gatewayClient;
    private final SimpMessagingTemplate messagingTemplate;

    public StatsBroadcaster(GatewayClientService gatewayClient,
                            SimpMessagingTemplate messagingTemplate) {
        this.gatewayClient = gatewayClient;
        this.messagingTemplate = messagingTemplate;
    }

    @Scheduled(fixedDelay = 2000) // a cada 2 segundos
    public void pushStats() {
        try {
            Map<String, Object> stats = gatewayClient.stats();
            if (stats != null) {
                messagingTemplate.convertAndSend("/topic/stats", stats);
            }
        } catch (RemoteException e) {
            // não rebenta o scheduler
        }
    }
}
