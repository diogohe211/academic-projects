package rmi;


import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;
import java.util.Map;


import system.DocumentData;
import system.Search_result;


public interface BarrelInterface extends Remote {
// ---- Ingestão vinda dos Downloaders ----
void upsertDocument(DocumentData doc) throws RemoteException; // cria/atualiza doc e índice
void registerOutLinks(String fromUrl, List<String> toUrls) throws RemoteException; // atualiza backlinks


// ---- Consultas vindas da Gateway ----
Search_result search(List<String> terms, int page, int pageSize) throws RemoteException; // ordenado por nº de backlinks
List<String> getBacklinks(String url) throws RemoteException; // páginas que apontam para url


// ---- Estatísticas ----
int getIndexSize() throws RemoteException; // nº de documentos
Map<String, Object> getBarrelStats() throws RemoteException; // livre, para Gateway agregar
}