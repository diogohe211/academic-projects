Projeto SD - Meta 1
Sistema Distribuído de Indexação e Pesquisa (Googol)
1. Descrição Geral

Este projeto implementa um sistema distribuído de indexação e pesquisa de páginas web, inspirado em motores de busca.
A comunicação entre componentes é feita através de Java RMI (Remote Method Invocation).
O sistema permite:

Submeter URLs para indexação;
Descarregar páginas e extrair texto, termos e links;
Efetuar pesquisas por palavras-chave;
Consultar backlinks e estatísticas de utilização em tempo real.

O sistema é composto pelos seguintes módulos:

Gateway – ponto central de coordenação e balanceamento.
Storage_barel – armazena e indexa os documentos processados.
Downloader – descarrega e processa as páginas web.
GoogolClient – cliente interativo de linha de comandos.

2. Estrutura de Diretórios

META1/
 ├── src/
 │   ├── barrel/          → Implementação do Storage_barel
 │   ├── client/          → Cliente (GoogolClient)
 │   ├── downloader/      → Lógica de download e indexação
 │   ├── gateway/         → Coordenação e registo RMI
 │   ├── rmi/             → Interfaces remotas
 │   └── system/          → Classes auxiliares (DocumentData, Statistics, etc.)
 ├── lib/jsoup-1.21.2.jar → Biblioteca usada para processar HTML
 ├── config/config.properties → Configurações de rede e nomes RMI
 ├── run_all.ps1          → Script PowerShell que executa todo o sistema
 ├──README.md            → Este ficheiro
 └──relatorio.md

3. Dependências

Java 8 (JDK 1.8 ou superior)
Biblioteca externa: jsoup-1.21.2.jar
Sistema Operativo recomendado: Windows 10 ou 11 (testado com PowerShell)

4. Compilação

Para compilar o projeto manualmente:
javac -cp "lib/jsoup-1.21.2.jar;src" -d bin (Get-ChildItem -Recurse -Filter *.java | ForEach-Object { $_.FullName })

5. Execução Automática

A forma mais simples de executar o sistema é através do script PowerShell incluído:
.\run_all.ps1

O script executa automaticamente:

-->O rmiregistry (porta 1099);
-->Dois Storage_barel (Barrel1, Barrel2);
-->A Gateway;
-->Um ou mais Downloaders;
-->O Cliente interativo (GoogolClient).

Cada componente é aberto numa janela separada.

6. Execução Manual

Caso pretenda executar manualmente (por exemplo, para debug):

1.Iniciar o RMI Registry
cd bin
start rmiregistry 1099

2.Lançar os Barrels
java -cp "bin;lib/jsoup-1.21.2.jar" barrel.Storage_barel Barrel1
java -cp "bin;lib/jsoup-1.21.2.jar" barrel.Storage_barel Barrel2
 
3.Lançar a Gateway
java -cp "bin;lib/jsoup-1.21.2.jar" gateway.Gateway

4.Lançar o Downloader
java -cp "bin;lib/jsoup-1.21.2.jar" downloader.Downloader

5.Lançar o Cliente
java -cp "bin;lib/jsoup-1.21.2.jar" client.GoogolClient

7. Funcionalidades

Indexação automática e recursiva de páginas web;
Pesquisa por palavras-chave (com paginação);
Consulta de backlinks;
Estatísticas em tempo real (top queries, latências e tamanho dos índices);
Failover automático entre barrels;
Execução paralela de múltiplos downloaders.

8. Limitações

A replicação entre barrels é best-effort (sem confirmação ACK).
O Reliable Multicast será implementado na Meta 2.
O balanceamento entre múltiplos downloaders é simples e não adaptativo.

9. Autores

Diogo Alves 2020214197
Diogo Henriques 2019222685

10. Notas Finais

Este sistema cumpre os objetivos da Meta 1 de Sistemas Distribuídos, apresentando uma solução funcional, modular e tolerante a falhas.
Foi testado com Java 8 em ambiente Windows, com múltiplos processos RMI e downloaders em execução paralela.