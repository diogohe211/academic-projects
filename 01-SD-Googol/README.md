# Googol

Distributed web search engine built with Java RMI. Downloaders collect pages, storage barrels index their content, and a gateway coordinates search. The web application adds search results, backlinks, statistics, Hacker News integration and an AI endpoint.

## Requirements

JDK, Maven and the bundled jsoup JAR. The backend uses RMI and the web application uses Spring Boot and Thymeleaf.

## Run

Compile the backend from meta1 in PowerShell.

```powershell
New-Item -ItemType Directory -Force bin
javac -cp "lib/jsoup-1.21.2.jar;src" -d bin (Get-ChildItem src -Recurse -Filter *.java).FullName
```

Review config/config.properties, then start the registry, storage barrels, gateway, downloaders and CLI client. The component commands are documented in run_all.ps1. With the backend running, start the web application from meta2.

```sh
mvn spring-boot:run
```

## Project notes

The supplied compiler.ps1 ends with a machine-specific directory change. run_all.ps1 contains a fixed JDK path and stops existing Java processes. Review these scripts before using them. Configure external service credentials locally. Team project;
