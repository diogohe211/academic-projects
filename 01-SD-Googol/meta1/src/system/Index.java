package system;


import java.util.*;


public class Index {
private final Map<String, Set<String>> invertedIndex = new HashMap<>();
private final Map<String, Set<String>> incomingLinks = new HashMap<>();


public void add(String word, String url) {
invertedIndex.computeIfAbsent(word, k -> new HashSet<>()).add(url);
}


public void addIncomingLink(String to, String from) {
incomingLinks.computeIfAbsent(to, k -> new HashSet<>()).add(from);
}


public Set<String> search(String word) {
return invertedIndex.getOrDefault(word, Collections.emptySet());
}


public Set<String> getIncomingLinks(String url) {
return incomingLinks.getOrDefault(url, Collections.emptySet());
}
}