# Escopo do fork WYD-Go

Esta árvore produz exclusivamente o client WYD 7.48 deste repositório. A source
Global/KR 7.59/7.69 que originou o TMProject é somente proveniência e referência
semântica; não é um segundo alvo suportado.

- lógica, UI, renderer, protocolo e lifecycle devem ser adaptados nesta source;
- conteúdo visual e tabelas devem ser adaptados nos assets 7.48;
- não manter slots, packets, controles, loaders ou branches exclusivos de
  versões mais novas;
- não executar, editar, verificar ou reaplicar os `.ps1` e binários em
  `client748/wyd.exe nativo+patches/`;
- compilar com `Build-Client.ps1`, que instala e confere automaticamente o
  resultado como `client748/project.exe` em todo build bem-sucedido.

Antes de editar, seguir `AGENTS.md`, `client-source/AGENTS.md` e a skill
`wyd-go-feature`. Mudanças de client exigem o gate Ghidra 7.48; referências
mais novas podem fornecer apenas comportamento a ser traduzido para o formato
comprovado do 7.48.

# Proveniência do TMProject

Este projeto é baseado em uma descompilação do cliente do jogo With Your Destiny. 
Nós não reindividicamos nenhum direito autoral do projeto. Este projeto foi criado apenas para fins de estudo sendo todos os direitos reservados exclusivamente e inteiramente a Hanbitsoft.

Este código deriva de uma descompilação do upstream. Nesta cópia, qualquer
alteração deve respeitar os contratos 7.48 e as validações definidos pelo
WYD-Go; completude do upstream não prova compatibilidade com o client alvo.

## Compilação

Usar `Build-Client.ps1`, que seleciona a toolchain suportada, produz o artefato
transitório em `build/<Configuration>/` e instala automaticamente
`client748/project.exe`, falhando se a cópia ou a conferência de hash falhar. O
produto atual é Win32; migrar para x64 mudaria ABI, packing e dependências e não
faz parte deste client 7.48.

## Dependências

### Visual Studio
Para compilar o projeto você precisará ter o Visual Studio 2022 com os seguintes componentes instalados:
- Desenvolvimento para desktop com C++
- ATL do C++ para a versão da toolchain que está sendo utilizada

A única dependência externa do projeto é o DirectX porém o mesmo está incluida no repositório e devidamente configurada.

## Bugs

Este projeto deriva de descompilação e pode conter problemas. Toda correção
deve ser confirmada contra o fluxo nativo 7.48 e validada no `project.exe`.

### Estilo herdado
Para manter o código mais organizado, o padrão de código que consta atualmente deverá ser seguido, como por exemplo:
- Variáveis com estilo hungarian, e.g: pPointer, nInt, bBoolean, sz para sized string e etc.
- Nome de classes em PascalCase
- entre outros...

Como não temos um documento que dite todas as regras, caso surja uma dúvida, procure no código algum exemplo do que você está tentando fazer.

## Contribuidores
Este projeto foi desenvolvido por:
- Eric Santos (SKEWED)
- Wed Souza (FREEDOM)
- Kevin Kouketsu (shepher)

# General Public License:

The code is under the [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html), so you can be feel free to study and contribute.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
