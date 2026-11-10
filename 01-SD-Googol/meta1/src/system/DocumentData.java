package system;


import java.io.Serializable;
import java.util.List;
import java.util.Set;


public class DocumentData implements Serializable {
private static final long serialVersionUID = 1L;
public final String url;
public final String title;
public final String snippet; // pequena citaçao
public final Set<String> terms; // termos normalizados
public final List<String> outLinks; // links encontrados na pag


public DocumentData(String url, String title, String snippet, Set<String> terms, List<String> outLinks) {
this.url = url;
this.title = title;
this.snippet = snippet;
this.terms = terms;
this.outLinks = outLinks;
}
}