# Segurança de assets do Client Go

## Objetivo e limite

Os assets distribuídos não devem ser tratados como segredo absoluto. O usuário
controla a máquina e pode observar um processo que possui a chave durante a
execução. A proteção serve para dificultar edição casual, extração e substituição
silenciosa; não substitui a autoridade do servidor.

## Pipeline de distribuição

```text
arquivo original 7.48
        -> conversão Go determinística
        -> compressão
        -> pacote com chunks
        -> AES-256-GCM ou XChaCha20-Poly1305
        -> manifesto com SHA-256
        -> assinatura Ed25519
```

O arquivo original nunca é sobrescrito. A chave privada de assinatura permanece
fora do client e do repositório. A chave pública embutida no client só verifica
autenticidade; ela não permite fabricar um pacote válido.

## Regras do pacote

- versão do formato e versão dos assets são obrigatórias;
- cada chunk possui tamanho, tipo, nonce e tag de autenticação;
- o manifesto é verificado antes de qualquer dado ser enviado ao renderer;
- hashes servem para integridade/cache, e a assinatura serve para autenticidade;
- chaves de conteúdo devem ser aleatórias e rotacionáveis;
- em modo online, a entrega da chave pode ser vinculada à sessão autenticada;
- em modo desenvolvimento, o pacote pode permanecer legível para diagnóstico,
  mas a validação estrutural continua ativa.

## Limite de confiança

Texturas, modelos, UI e efeitos locais não podem decidir dano, skill aprendida,
item, teleporte, recompensa ou qualquer outra regra de gameplay. O servidor
deve rejeitar intenções inválidas independentemente do conteúdo carregado pelo
client.
