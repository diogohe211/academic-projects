package web.controller;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.*;
import web.hn.HnService;

@Controller
public class HnController {

    private final HnService hn;

    public HnController(HnService hn) {
        this.hn = hn;
    }

    @GetMapping("/hn")
    public String hn(@RequestParam(value = "n", defaultValue = "10") int n, Model model) {
        try {
            model.addAttribute("items", hn.top(n));
        } catch (Exception e) {
            model.addAttribute("error", e.getClass().getSimpleName() + ": " + e.getMessage());
        }
        model.addAttribute("n", n);
        return "hn";
    }
}
