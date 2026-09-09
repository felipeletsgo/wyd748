#pragma once

#include "MessageHeader.h"

// Contrato existente de transferencia/renomeacao na selecao de personagem.
// O emissor nativo 004A32DD envia 52 bytes; 004A626E consome Result e Slot.
// Extraido de Basedef sem trocar tipos, packing ou ordem dos campos.
constexpr auto MSG_ReqTransper_Opcode = 0xFAA;
struct MSG_ReqTransper
{
    MSG_STANDARD Header;
    // Zero solicita/confirma sucesso; demais valores sao resultados do servidor.
    int Result;
    // Indice signed: a cena valida [0,4) antes de consultar seus arrays.
    int Slot;
    // Campos wire fixos; receber 16 bytes nao garante terminacao nula.
    char OldName[16];
    char NewName[16];
};

static_assert(sizeof(int) == 4, "Transfer requires signed 32-bit fields");
static_assert(sizeof(MSG_ReqTransper) == 52, "Transfer packet size changed");
static_assert(offsetof(MSG_ReqTransper, Header) == 0, "Transfer header moved");
static_assert(offsetof(MSG_ReqTransper, Result) == 12, "Transfer result moved");
static_assert(offsetof(MSG_ReqTransper, Slot) == 16, "Transfer slot moved");
static_assert(offsetof(MSG_ReqTransper, OldName) == 20, "Transfer old name moved");
static_assert(offsetof(MSG_ReqTransper, NewName) == 36, "Transfer new name moved");
