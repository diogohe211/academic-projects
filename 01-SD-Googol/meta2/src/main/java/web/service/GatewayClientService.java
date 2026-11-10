package web.service;

import org.springframework.stereotype.Service;
import rmi.GatewayInterface;
import system.Search_result;

import java.rmi.RemoteException;
import java.util.List;
import java.util.Map;

@Service
public class GatewayClientService {

    private final GatewayInterface gateway;

    public GatewayClientService(GatewayInterface gateway) {
        this.gateway = gateway;
    }

    public Search_result search(String query, int page) throws RemoteException {
        return gateway.search(query, page);
    }

    public void submitUrl(String url) throws RemoteException {
        gateway.submitUrl(url);
    }

    public List<String> backlinks(String url) throws RemoteException {
        return gateway.backlinks(url);
    }

    public Map<String, Object> stats() throws RemoteException {
        return gateway.stats();
    }
}
