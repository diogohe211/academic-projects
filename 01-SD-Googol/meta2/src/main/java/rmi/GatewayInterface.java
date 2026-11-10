package rmi;


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;
import java.util.Map;


import system.Search_result;


public interface GatewayInterface extends Remote {
// Cliente -> Gateway
void submitUrl(String url) throws RemoteException; // indexar url
Search_result search(String query, int page) throws RemoteException; // pesqeuisa + paginacao
List<String> backlinks(String url) throws RemoteException; // lista de paginas que apontam para url
Map<String, Object> stats() throws RemoteException; //stats irt


// para gestao dos barrels
void addBarrel(String name, String rmiUrl) throws RemoteException;
void removeBarrel(String name) throws RemoteException;

//downloader-->Gateway
String pollUrl() throws RemoteException;
}