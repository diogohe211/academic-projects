# =======================================
# PowerShell Script: run_all.ps1
# Creditos: Feito com o auxilio do Chatgpt
# Descrição:
#   arranca todo o sistema RMI (registry, barrel, gateway, downloader, client)
#   com esperas e verificações simples
# =======================================

# caminhos base (usar diretório do script)
$projectPath = $PSScriptRoot
$binPath     = Join-Path $projectPath "bin"
$libPath     = Join-Path (Join-Path $projectPath "lib") "jsoup-1.21.2.jar"

function Section($msg) {
    Write-Host "`n==== $msg ====" -ForegroundColor Cyan
}

# Caminho fixo para rmiregistry (ajuste se necessário)
$rmiExePath = "C:\Program Files\Java\jdk-21\bin\rmiregistry.exe"

# ==========================
# 1️⃣ limpar ambiente
# ==========================
Section "limpar processos antigos"
Get-Job | Stop-Job -ErrorAction SilentlyContinue
Remove-Job * -ErrorAction SilentlyContinue
Get-Process java -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

# ==========================
# 2️⃣ iniciar rmiregistry
# ==========================
Section "a iniciar rmiregistry (porta 1099)"
if (-not (Test-Path -Path $binPath)) {
    Write-Host "[aviso] pasta 'bin' não existe em $binPath. O rmiregistry será iniciado no diretório do projeto." -ForegroundColor Yellow
}
if (-not (Test-Path $rmiExePath)) {
    Write-Host "[erro] rmiregistry não encontrado em '$rmiExePath'. Atualize a variável $rmiExePath no topo do script para o caminho correto do seu JDK." -ForegroundColor Red
    exit 1
}
Start-Process -FilePath $rmiExePath -ArgumentList "1099" -WorkingDirectory ($binPath | ForEach-Object { if (Test-Path $_) { $_ } else { $projectPath } })
Start-Sleep -Seconds 6

$registryOK = $false
for ($i=0; $i -lt 10; $i++) {
    $check = netstat -ano | findstr /R "LISTENING\s*1099|:1099"
    if ($check) { $registryOK = $true; break }
    Start-Sleep -Milliseconds 500
}

$registryOK = $false
for ($i=0; $i -lt 12; $i++) {
    $check = netstat -ano | findstr /R "LISTENING\s*1099|:1099"
    if ($check) { $registryOK = $true; break }
    # fallback: processo rmiregistry ativo?
    $proc = Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match "rmiregistry|java" }
    if ($proc) {
        # se processo ativo, assumimos OK
        $registryOK = $true; break
    }
    Start-Sleep -Milliseconds 500
}

if (-not $registryOK) {
    Write-Host "[aviso] não foi possível confirmar a porta 1099 via netstat, mas se a janela do rmiregistry está aberta, pode prosseguir." -ForegroundColor Yellow
}
Write-Host "[ok] rmiregistry ativo." -ForegroundColor Green

# ==========================
# 3️⃣ Lançar Storage Barrels (Barrel1 e Barrel2)
# ==========================
Section "A INICIAR STORAGE_BARELS"

# Classpath único
$cp = "bin;$libPath"

# Lança o primeiro barrel (Barrel1) em nova consola CMD
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" barrel.Storage_barel Barrel1"

# Lança o segundo barrel (Barrel2) em nova consola CMD
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" barrel.Storage_barel Barrel2"

Start-Sleep -Seconds 3

# ==========================
# 4️⃣ lançar gateway
# ==========================
Section "a iniciar gateway"
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" gateway.Gateway"
Start-Sleep -Seconds 3

# ==========================
# 5️⃣ lançar downloader
# ==========================
Section "A INICIAR DOWNLOADERS (2 INSTÂNCIAS)"
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" downloader.Downloader"
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" downloader.Downloader"
Start-Sleep -Seconds 2

# ==========================
# 6️⃣ lançar cliente
# ==========================
Section "a iniciar cliente interativo"
Start-Process cmd -ArgumentList '/k', "cd /d `"$projectPath`" && java -cp `"$cp`" client.GoogolClient"

# ==========================
# fim
# ==========================
Write-Host "`n===== todos os componentes foram iniciados =====" -ForegroundColor Green
Write-Host "janela 1: rmiregistry (bin)"
Write-Host "janela 2: barrel registado"
Write-Host "janela 3: gateway ligada"
Write-Host "janela 4: downloader ativo"
Write-Host "janela 5: cliente interativo"
Write-Host "============================================" -ForegroundColor Yellow
