param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Terceiro elo da cadeia de patches do client 7.48 (bypass de dados):
#
#   WYD.original.exe            B545EA10...619C
#     -> Patch-WYD748.ps1                 (camera/mouse, cores de dano)
#   WYD.pre-extended-stats.exe  2AA1773A...21EE
#     -> Patch-WYD748-ExtendedStats.ps1   (stats 32-bit, secao .xstat)
#   (intermediario)             4724B76F...45D6
#     -> ESTE SCRIPT                      (4 bypasses)
#   WYD.exe                     E704DA0A...E20D
#
# POR QUE EXISTE: os quatro saltos abaixo sao necessarios para liberar a
# versao, o SkillData e as duas validacoes do ItemList. A cadeia anterior tinha
# apenas tres; o checksum inicial do ItemList ficou fora do script e fazia o
# client rejeitar qualquer binario regenerado. Mantemos todos em um elo unico,
# com guarda de bytes e SHA, para que nenhuma edicao manual volte a ser necessaria.
#
# COMO FUNCIONAM: nos sitios de salto o codigo original usa JZ para
# pular o `33 C0` (xor eax,eax) que zera o resultado. Trocar por `EB 04`
# (JMP +4), ou `EB 07` no primeiro checksum do ItemList (JMP +7), torna o
# salto incondicional, entao o caminho de falha nunca roda -- a comparacao
# passa a ser irrelevante. O salto de +7 precisa ser separado porque o bloco
# de falha dessa rotina tem cinco bytes a mais que os outros.

$entradaHash = '4724B76F19702645D6052A9D'   # prefixo: saida do patch wide
$saidaHash   = 'E704DA0A185892EBE394324ED3A1001FC0DFB832334546186E61948DED15E20D'

$sitios = @(
    @{
        Offset = 0x07E44A; De = [byte[]](0x74,0x04); Para = [byte[]](0xEB,0x04)
        Nome   = 'versao do client'
        # cmp dword [ebp-0E], 0x276 / 0x278 -- compara a versao declarada
        # contra duas constantes. Sem o bypass, versao diferente derruba o login.
    },
    @{
        Offset = 0x14F9E8; De = [byte[]](0x74,0x04); Para = [byte[]](0xEB,0x04)
        Nome   = 'checksum de arquivo de dados #1'
        # push 0x0092FEF8 / call / cmp eax, 0x000A152F -- o retorno da funcao
        # de checksum e comparado com uma constante cravada no executavel.
        # Sem o bypass, qualquer edicao no arquivo impede o carregamento.
    },
    @{
        Offset = 0x14FAD0; De = [byte[]](0x74,0x07); Para = [byte[]](0xEB,0x07)
        Nome   = 'checksum inicial do ItemList'
        # cmp eax, 0x035489AD; o client 7.48 mantem o checksum original do
        # ItemList compilado. Sem este salto, um ItemList.bin regenerado com
        # campos/volatiles novos ainda falha antes de chegar ao segundo ramo.
    },
    @{
        Offset = 0x14FBFC; De = [byte[]](0x74,0x04); Para = [byte[]](0xEB,0x04)
        Nome   = 'checksum final do ItemList/ExtraItem'
        # push 0x00D44950 / call / cmp eax, 0x035489AD -- mesmo padrao, outro
        # arquivo. E este par que permite editar o ItemList livremente.
    }
)

$data = [IO.File]::ReadAllBytes($Executable)
$antes = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash

if ($saidaHash -and $antes.StartsWith($saidaHash)) {
    Write-Host 'Bypasses ja aplicados; nada a fazer.'
    return
}
if (-not $antes.StartsWith($entradaHash)) {
    throw "Entrada inesperada (SHA-256: $antes). Rode este script sobre a saida do Patch-WYD748-ExtendedStats.ps1."
}

foreach ($s in $sitios) {
    for ($i = 0; $i -lt $s.De.Length; $i++) {
        $atual = $data[$s.Offset + $i]
        if ($atual -ne $s.De[$i]) {
            throw ('{0} (0x{1:X6}): esperado {2:X2}, encontrado {3:X2}. Verifique a cadeia antes de continuar.' -f
                $s.Nome, ($s.Offset + $i), $s.De[$i], $atual)
        }
    }
    [Array]::Copy($s.Para, 0, $data, $s.Offset, $s.Para.Length)
}

[IO.File]::WriteAllBytes($Executable, $data)

$depois = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
if (-not $depois.StartsWith($saidaHash)) {
    throw "Resultado inesperado (SHA-256: $depois)."
}
foreach ($s in $sitios) { Write-Host ('  bypass aplicado: {0} (0x{1:X6})' -f $s.Nome, $s.Offset) }
Write-Host ('SHA-256: {0}' -f $depois)
