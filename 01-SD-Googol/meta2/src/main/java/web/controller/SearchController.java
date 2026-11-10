package web.controller;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;
import system.Search_result;
import web.service.GatewayClientService;

import java.rmi.RemoteException;
import java.util.List;
import java.util.Map;

@Controller
public class SearchController {

    private final GatewayClientService gatewayClient;

    public SearchController(GatewayClientService gatewayClient) {
        this.gatewayClient = gatewayClient;
    }

    @GetMapping("/")
    public String home() {
        return "index";
    }

    @GetMapping("/search")
    public String search(@RequestParam("q") String q,
                         @RequestParam(value = "page", defaultValue = "0") int page,
                         Model model) throws RemoteException {

        Search_result result = gatewayClient.search(q, page);

        model.addAttribute("query", q);
        model.addAttribute("result", result);
        model.addAttribute("page", page);

        return "results";
    }

    @PostMapping("/submit-url")
    public String submitUrl(@RequestParam("url") String url,
                            RedirectAttributes ra) throws RemoteException {

        if (url != null && !url.trim().isEmpty()) {
            gatewayClient.submitUrl(url.trim());
            ra.addFlashAttribute("message", "URL submetido com sucesso!");
        }
        return "redirect:/";
    }

    @GetMapping("/backlinks")
    public String backlinks(@RequestParam("url") String url, Model model) throws RemoteException {

        List<String> links = gatewayClient.backlinks(url);

        model.addAttribute("url", url);
        model.addAttribute("backlinks", links);

        return "backlinks"; // templates/backlinks.html
    }

    @GetMapping("/stats")
    public String stats(Model model) throws RemoteException {

        Map<String, Object> stats = gatewayClient.stats();

        model.addAttribute("stats", stats);

        return "stats"; // templates/stats.html
    }
}
