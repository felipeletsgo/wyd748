# Admissão de rede

## Camada server-side

O servidor usa exclusivamente o endereço remoto observado no socket TCP. Ele
não confia em IP, MAC, HWID ou nome de máquina enviados pelo client.

A proteção possui duas fronteiras independentes:

1. `max_connections_per_ip` limita sockets, inclusive antes do login;
2. `max_authenticated_clients_per_ip` limita janelas depois da senha correta.

O padrão de produção é quatro clients autenticados por IP público. Tanto a
fronteira de sockets pré-auth quanto a de clients autenticados agrupam IPv6 por
`/64`, para que endereços temporários da mesma rede não multipliquem o limite.

## Política de VPS, VPN e datacenter

`data/network_admission.json` contém regras CIDR locais. Nenhuma consulta HTTP,
DNS ou API externa acontece durante o login. Isso evita transformar a
disponibilidade de um serviço de reputação em dependência do World.

Exemplo:

```json
{
  "version": 1,
  "rules": [
    {
      "cidr": "203.0.113.0/24",
      "action": "deny",
      "source": "hosting-provider",
      "reason": "hosting provider"
    },
    {
      "cidr": "203.0.113.8/32",
      "action": "allow",
      "reason": "trusted tester"
    },
    {
      "cidr": "198.51.100.0/24",
      "action": "limit",
      "maxClients": 1,
      "reason": "anonymous proxy range"
    }
  ]
}
```

Regras:

- o prefixo mais específico vence;
- `deny` recusa antes de executar PBKDF2;
- `allow` cria uma exceção dentro de uma faixa ampla, mas não remove o limite
  global de quatro clients;
- `limit` cria um teto agregado para todo o CIDR que fez match, simultâneo ao
  limite normal por IP/`/64`; seu valor efetivo nunca supera o teto global;
- CIDRs duplicados, não canônicos ou ações desconhecidas derrubam o boot;
- `reason` aparece apenas no log interno; o client recebe uma mensagem genérica;
- alterações entram em vigor depois de reiniciar o servidor.

## Compilador offline de ranges

O comando `cmd/network-admission` transforma feeds baixados pelo operador no
arquivo estrito usado pelo servidor. Ele não baixa conteúdo e não acessa a rede.
Aceita texto com um CIDR por linha ou JSON nos formatos comuns que expõem
`ip_prefix`, `ipv6_prefix`, `ipv4Prefix`, `ipv6Prefix`, `cidr` ou
`addressPrefixes`.

Exemplo de validação sem escrita:

```powershell
go run ./cmd/network-admission `
  -base data/network_admission.json `
  -input C:\feeds\provider-ranges.json `
  -source provider-name `
  -action deny `
  -reason "hosting provider" `
  -dry-run
```

Depois da revisão, remova `-dry-run` para substituir as regras de
`source=provider-name`. Reexecutar a importação remove os ranges antigos dessa
mesma fonte, inclui os novos e preserva regras manuais ou de outras fontes.

Para reduzir a uma janela em vez de bloquear:

```powershell
go run ./cmd/network-admission `
  -input C:\feeds\proxy-ranges.txt `
  -source proxy-feed `
  -action limit `
  -max-clients 1 `
  -reason "anonymous proxy"
```

O compilador:

- mascara host bits e remove duplicatas;
- ordena a saída deterministicamente;
- recusa conflito exato entre fontes com decisões diferentes;
- identifica cada lote por `source` minúsculo e estável;
- grava por arquivo temporário, `fsync`, fechamento e `rename` no mesmo
  diretório. A substituição é atômica somente onde o sistema operacional e o
  filesystem oferecem essa garantia; isso também não equivale a garantir
  durabilidade após queda de energia;
- nunca altera a política quando a validação falha.

O arquivo versionado começa vazio para não bloquear provedores legítimos com
uma lista inventada ou desatualizada. Faixas de hosting/VPN devem vir de uma
fonte de reputação escolhida e revisada pelo operador. Um atualizador futuro
pode baixar feeds fora do processo; o compilador acima continua sendo a
fronteira de validação antes de substituir o JSON autoritativo.

## Limite desta etapa

O servidor não consegue determinar sozinho se o sistema remoto é físico ou uma
VM. MAC também não atravessa roteadores. Uma futura etapa do client deverá usar
launcher/DLL, desafio do servidor e identificação autenticada. Mesmo assim,
esse sinal será uma camada de risco; o servidor continuará aplicando IP, CIDR,
limites de sessão e análise comportamental.
