param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Ultimo elo da cadeia de patches do client 7.48:
#
#   WYD.original.exe            B545EA10...619C
#     -> Patch-WYD748.ps1                 (camera/mouse, cores de dano)
#   WYD.pre-extended-stats.exe  2AA1773A...21EE
#     -> Patch-WYD748-ExtendedStats.ps1   (stats 32-bit, secao .xstat)
#   (intermediario)             4724B76F...45D6
#     -> ESTE SCRIPT                      (3 bypasses)
#   WYD.exe                     F2B8CDEB...0C5B
#
# POR QUE EXISTE: estes tres bytes estavam no WYD.exe em uso mas em NENHUM
# script -- foram editados a mao e nunca documentados. Quem reaplicasse o patch
# wide os perderia, e o client pararia de carregar o ItemList e de conectar,
# sem pista nenhuma do motivo. Descobertos ao reconstruir a linha-base:
# eram a UNICA diferenca entre o binario em uso e a saida do patch wide.
#
# COMO FUNCIONAM: nos tres sitios o codigo original usa `74 04` (JZ +4) para
# pular o `33 C0` (xor eax,eax) que zera o resultado. Trocar por `EB 04`
# (JMP +4) torna o salto incondicional, entao o caminho de falha nunca roda --
# a comparacao passa a ser irrelevante.

$entradaHash = '4724B76F19702645D6052A9D'   # prefixo: saida do patch wide
$saidaHash   = 'F2B8CDEB9863192A5CDA4FCB'   # prefixo: client em uso

$sitios = @(
    @{
        Offset = 0x07E44A; De = 0x74; Para = 0xEB
        Nome   = 'versao do client'
        # cmp dword [ebp-0E], 0x276 / 0x278 -- compara a versao declarada
        # contra duas constantes. Sem o bypass, versao diferente derruba o login.
    },
    @{
        Offset = 0x14F9E8; De = 0x74; Para = 0xEB
        Nome   = 'checksum de arquivo de dados #1'
        # push 0x0092FEF8 / call / cmp eax, 0x000A152F -- o retorno da funcao
        # de checksum e comparado com uma constante cravada no executavel.
        # Sem o bypass, qualquer edicao no arquivo impede o carregamento.
    },
    @{
        Offset = 0x14FBFC; De = 0x74; Para = 0xEB
        Nome   = 'checksum de arquivo de dados #2'
        # push 0x00D44950 / call / cmp eax, 0x035489AD -- mesmo padrao, outro
        # arquivo. E este par que permite editar o ItemList livremente.
    }
)

$data = [IO.File]::ReadAllBytes($Executable)
$antes = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash

if ($antes.StartsWith($saidaHash)) {
    Write-Host 'Bypasses ja aplicados; nada a fazer.'
    return
}
if (-not $antes.StartsWith($entradaHash)) {
    throw "Entrada inesperada (SHA-256: $antes). Rode este script sobre a saida do Patch-WYD748-ExtendedStats.ps1."
}

foreach ($s in $sitios) {
    $atual = $data[$s.Offset]
    if ($atual -ne $s.De) {
        throw ('{0} (0x{1:X6}): esperado {2:X2}, encontrado {3:X2}. Verifique a cadeia antes de continuar.' -f
            $s.Nome, $s.Offset, $s.De, $atual)
    }
    $data[$s.Offset] = $s.Para
}

[IO.File]::WriteAllBytes($Executable, $data)

$depois = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
if (-not $depois.StartsWith($saidaHash)) {
    throw "Resultado inesperado (SHA-256: $depois)."
}
foreach ($s in $sitios) { Write-Host ('  bypass aplicado: {0} (0x{1:X6})' -f $s.Nome, $s.Offset) }
Write-Host ('SHA-256: {0}' -f $depois)
