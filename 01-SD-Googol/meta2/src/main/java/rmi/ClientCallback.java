package rmi;


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Map;


public interface ClientCallback extends Remote {
void onStatsUpdate(Map<String, Object> snapshot) throws RemoteException;
}