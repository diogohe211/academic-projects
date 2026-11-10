package system;


import java.io.Serializable;
import java.util.List;


public class Search_result implements Serializable {
private static final long serialVersionUID = 1L;
public static class Item implements Serializable {
private static final long serialVersionUID = 1L;
public final String url;
public final String title;
public final String snippet;
public final int inLinks;
public Item(String url, String title, String snippet, int inLinks) {
this.url = url;
this.title = title;
this.snippet = snippet;
this.inLinks = inLinks;
}
}
public final int page;
public final int pageSize;
public final int total;
public final List<Item> items;
public Search_result(int page, int pageSize, int total, List<Item> items) {
this.page = page;
this.pageSize = pageSize;
this.total = total;
this.items = items;
}
}