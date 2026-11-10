package web.controller;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.*;
import web.ai.OpenAiService;

@Controller
public class AiController {

    private final OpenAiService ai;

    public AiController(OpenAiService ai) {
        this.ai = ai;
    }

    @GetMapping("/ai")
    public String page() {
        return "ai";
    }

    @PostMapping("/ai")
    public String ask(@RequestParam("q") String q, Model model) throws Exception {
        model.addAttribute("q", q);
        model.addAttribute("answer", ai.ask(q));
        return "ai";
    }
}
