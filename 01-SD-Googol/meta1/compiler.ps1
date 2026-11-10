# =======================================
# PowerShell Script: compiler.ps1
# Descrição: Compila todo o projeto Java para a pasta bin
# =======================================

$ErrorActionPreference = "Stop"

function Section($msg) {
	Write-Host "`n==== $msg ====" -ForegroundColor Cyan
}

function Fail($msg) {
	Write-Host "[erro] $msg" -ForegroundColor Red
	exit 1
}

# Caminhos base
$projectPath = $PSScriptRoot
$srcPath = Join-Path $projectPath "src"
$binPath = Join-Path $projectPath "bin"
$libPath = Join-Path (Join-Path $projectPath "lib") "jsoup-1.21.2.jar"

Section "verificações de pré-requisitos"

# Verificar JDK/javac
$javac = Get-Command javac -ErrorAction SilentlyContinue
if (-not $javac -and $env:JAVA_HOME) {
	$javacCandidate = Join-Path $env:JAVA_HOME "bin\javac.exe"
	if (Test-Path $javacCandidate) {
		$javac = @{ Path = $javacCandidate }
	}
}
if (-not $javac) {
	Fail "javac não encontrado. Instale JDK e/ou defina JAVA_HOME/Path."
}
Write-Host "[ok] javac encontrado em: $($javac.Path)" -ForegroundColor Green

# Verificar jsoup jar (auto-download se em falta)
if (-not (Test-Path $libPath)) {
	Section "a obter jsoup (1.21.2)"
	$libDir = Split-Path $libPath -Parent
	New-Item -ItemType Directory -Force -Path $libDir | Out-Null
	$url = "https://repo1.maven.org/maven2/org/jsoup/jsoup/1.21.2/jsoup-1.21.2.jar"
	try {
		Write-Host "[download] $url -> $libPath" -ForegroundColor Yellow
		Invoke-WebRequest -Uri $url -OutFile $libPath -UseBasicParsing
	}
	catch {
		Fail "falha ao descarregar jsoup: $($_.Exception.Message). Faça download manual e coloque em '$libPath'"
	}
}
Write-Host "[ok] jsoup jar: $libPath" -ForegroundColor Green

# Verificar src
if (-not (Test-Path $srcPath)) { Fail "pasta src não existe em '$srcPath'" }

# Criar bin
Section "preparar pasta bin"
New-Item -ItemType Directory -Force -Path $binPath | Out-Null
Write-Host "[ok] pasta bin pronta: $binPath" -ForegroundColor Green

Section "compilar fontes"

# Obter todos os .java
$javaFiles = Get-ChildItem -Path $srcPath -Recurse -Filter *.java | Select-Object -ExpandProperty FullName
if (-not $javaFiles -or $javaFiles.Count -eq 0) { Fail "não foram encontrados ficheiros .java em '$srcPath'" }

# Construir classpath (Windows usa ;)
$classpath = "lib\jsoup-1.21.2.jar;src"

# Compilar (mostrar comando de forma legível)
Write-Host "[cmd] $($javac.Path) -cp $classpath -d $binPath <$(($javaFiles | Measure-Object).Count)> ficheiros>" -ForegroundColor Yellow

# Executar compilação
& $javac.Path -cp $classpath -d $binPath $javaFiles

if ($LASTEXITCODE -ne 0) {
	Fail "compilação falhou (exit code: $LASTEXITCODE)"
}

Section "compilação concluída"
Write-Host "[ok] classes geradas em: $binPath" -ForegroundColor Green
Write-Host "Pode agora executar: .\\run_all.ps1" -ForegroundColor Cyan
Set-Location "C:\Users\Legion\Desktop\PROJSD\meta1"

# Optional: set JAVA_HOME for this session
# $env:JAVA_HOME = "C:\Program Files\Java\jdk-21"
# $env:Path = "$env:JAVA_HOME\bin;$env:Path"

# Ensure output folder exists
New-Item -ItemType Directory -Force -Path .\bin | Out-Null

# Compile all .java files, using jsoup + src on the classpath, output to bin
javac -cp "lib\jsoup-1.21.2.jar;src" -d bin (Get-ChildItem -Recurse -Filter *.java | ForEach-Object { $_.FullName })