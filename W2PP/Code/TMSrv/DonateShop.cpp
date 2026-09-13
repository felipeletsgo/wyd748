#pragma once

#include "ProcessClientMessage.h"
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <ctime>
#include "SendFunc.h"
#include "Functions.h"
#include "wMySQL.h"

bool StoreAgroup(int itemId)
{
	int List[] = {
		3314, 4140, 3312, 3343, 3336, 3310, 3311, 3407, 3408, 3409, 3410, 3411, 3412, 3413, 3414, 3415, 3416, 3417, 2426, 3330, 777
	};

	for (int i = 0; i < (sizeof(List) / 4); i++) {
		if (itemId == List[i])
			return true;
	}
	return false;
}

void ReqDonateShop(int conn, char* pMsg)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	MSG_ReqShopDonate* m = (MSG_ReqShopDonate*)pMsg;

	int Size = m->Size;

	if (Size > sizeof(MSG_ReqShopDonate)) //CONTROLE DE SIZE
	{
		SendClientMessage(conn, "Imposs�vel executar a��o, tente mais tarde.");
		return;
	}

	if (pUser[conn].DonateShopDelay != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].DonateShopDelay; // ATRASO PACOTE

		if (isTime < 3000)
		{
			SendClientMessage(conn, "Aguarde 3 segundo para uma nova Tentativa.");
			return;
		}

	}
	pUser[conn].DonateShopDelay = GetTickCount64();

	int nShop = m->Store;
	int nPage = m->Page;
	int nItemPos = m->ItemPos;
	int nQnt = m->Qnt;

	if (nShop < 0 || nShop > 9)
		return;

	if (nPage < 0 || nPage > 4)
		return;

	if (nItemPos < 0 || nItemPos > 14)
		return;

	if (nQnt <= 0 || nQnt > 120)
		return;

	int ItemIndex = cStoreItens[nShop][nPage][nItemPos][0];
	int Price = (cStoreItens[nShop][nPage][nItemPos][1] * nQnt);

	if (ItemIndex <= 0 || ItemIndex >= MAX_ITEMLIST)
		return;

	auto& pc = cSQL::instance();

	int Donate = 0;

	// SQL Injection fix - escapar AccountName
	char escapedAccount[64 * 2 + 1];
	cSQL::EscapeString(escapedAccount, pUser[conn].AccountName, sizeof(escapedAccount));

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `accounts` WHERE `username` = '%s'", escapedAccount);
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Donate = atoi(row[6]);
	}
	mysql_free_result(result);
	if (Donate < Price)
	{
		SendClientMessage(conn, "Saldo de Rubis Insuficiente");
		return;
	}

	int FreeSlot = 0;
	int slotsInv = 30;
	int SlotReq = 1;

	bool Agroup = false;

	if (StoreAgroup(ItemIndex) == true)
		Agroup = true;

	if (Agroup == false)
		SlotReq = nQnt;

	if (pMob[conn].MOB.Carry[60].sIndex == 3467)
		slotsInv += 15;
	if (pMob[conn].MOB.Carry[61].sIndex == 3467)
		slotsInv += 15;

	for (int i = 0; i < slotsInv; i++) {
		if (pMob[conn].MOB.Carry[i].sIndex == 0)
			FreeSlot += 1;
	}
	if (FreeSlot < SlotReq) {
		SendClientMessage(conn, "N�o h� espa�o dispon�vel no Invent�rio");
		return;
	}

	STRUCT_ITEM item;
	memset(&item, 0, sizeof(STRUCT_ITEM));
	item.sIndex = ItemIndex;
	item.stEffect[0].cEffect = Agroup ? 61 : 0;
	item.stEffect[0].cValue = Agroup ? nQnt : 0;

	int tQnt = nQnt;

	if (Agroup == false)
	{
		while (nQnt >= 1) {
			PutItem(conn, &item);
			nQnt -= 1;
		}
	}
	else {
		PutItem(conn, &item);
	}

	SNPRINTF_SAFE(xQuery, sizeof(xQuery), "UPDATE accounts SET donate = '%d' WHERE username = '%s' ", (Donate - Price), escapedAccount);
	pc.wQuery(xQuery);

	SendClientMessage(conn, strFmt("Comprou [x%d] %s por [%d] Rubis", tQnt, g_pItemList[ItemIndex].Name, Price));
	ItemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, strFmt("Comprou [x%d] %s por [%d] Rubis | Restantes: %d", tQnt, g_pItemList[ItemIndex].Name, Price, Donate - Price));
	UpdateDonate(conn);
}

void TradeDonate(int conn, char* pMsg)
{

	MSG_SendPix* m = (MSG_SendPix*)pMsg;

	int Size = m->Size;

	if (Size > 24) //CONTROLE DE SIZE
	{
		SendClientMessage(conn, "Imposs�vel executar a��o, tente mais tarde.");
		return;
	}

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	int value = m->qnt;

	if (value < 100)
		return;

	if (pUser[conn].Atraso != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].Atraso; // ATRASO PACOTE

		if (isTime < 3000)
		{
			SendClientMessage(conn, "Aguarde 3 segundo para uma nova Tentativa.");
			return;
		}

	}
	pUser[conn].Atraso = GetTickCount64();

	auto& pc = cSQL::instance();

	int Donate = 0;
	std::string mypix;

	// SQL Injection fix
	char escapedAccount[64 * 2 + 1];
	cSQL::EscapeString(escapedAccount, pUser[conn].AccountName, sizeof(escapedAccount));

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `accounts` WHERE `username` = '%s'", escapedAccount);
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		mypix = row[20];
		Donate = atoi(row[6]);
	}	
	mysql_free_result(result);

	if (Donate < value)
	{
		SendClientMessage(conn, "Saldo Insuficiente");
		return;
	}

	auto& pc2 = cSQL::instance();

	int Donate2 = 0;
	std::string pixrecive;
	std::string pixkey = { m->chave };

	// SQL Injection fix - escapar chave pix
	char escapedPix[256 * 2 + 1];
	cSQL::EscapeString(escapedPix, m->chave, sizeof(escapedPix));

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `accounts` WHERE `pix` = '%s'", escapedPix);
	MYSQL_ROW row2;
	MYSQL* wSQL2 = pc2.wStart();
	MYSQL_RES* result2 = pc2.wRes(wSQL2, hQuery);

	if (result2 == NULL)
		return;

	while ((row2 = mysql_fetch_row(result2)) != NULL)
	{
		pixrecive = row2[20];
		Donate2 = atoi(row2[6]);
	}
	mysql_free_result(result2);

	if (pixrecive != pixkey) {
		SendClientMessage(conn, "Jogador N�o Encontrado");
		return;
	}

	if (mypix == pixrecive) {
		SendClientMessage(conn, "N�o pode enviar para si mesmo");
		return;
	}

	int Value = value;

	// escapedPix já foi declarado e preenchido acima (linha 242)
	SNPRINTF_SAFE(xQuery, sizeof(xQuery), "UPDATE accounts SET donate = '%d' WHERE pix = '%s' ", (Donate2 + Value), escapedPix);
	pc2.wQuery(xQuery);

	auto& pc3 = cSQL::instance();
	SNPRINTF_SAFE(xQuery, sizeof(xQuery), "UPDATE accounts SET donate = '%d' WHERE username = '%s' ", (Donate - value), escapedAccount);
	pc3.wQuery(xQuery);

	SendClientMessage(conn, strFmt("[%d] Rubis enviados para a chave[%s]", value, m->chave));
	ItemLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, strFmt("[%d] Rubis enviados para a chave[%s] | Saldo: %d", value, m->chave, Donate - value));
	UpdateDonate(conn);
	return;
}

void UpdateShopDonate1() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore1` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[0][0][y][0] = Index[y];
		cStoreItens[0][0][y][1] = Value[y];
		cStoreItens[0][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[0][1][z - 15][0] = Index[z];
		cStoreItens[0][1][z - 15][1] = Value[z];
		cStoreItens[0][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[0][2][x - 30][0] = Index[x];
		cStoreItens[0][2][x - 30][1] = Value[x];
		cStoreItens[0][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[0][3][x - 45][0] = Index[x];
		cStoreItens[0][3][x - 45][1] = Value[x];
		cStoreItens[0][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[0][4][x - 60][0] = Index[x];
		cStoreItens[0][4][x - 60][1] = Value[x];
		cStoreItens[0][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate2() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore2` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[1][0][y][0] = Index[y];
		cStoreItens[1][0][y][1] = Value[y];
		cStoreItens[1][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[1][1][z - 15][0] = Index[z];
		cStoreItens[1][1][z - 15][1] = Value[z];
		cStoreItens[1][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[1][2][x - 30][0] = Index[x];
		cStoreItens[1][2][x - 30][1] = Value[x];
		cStoreItens[1][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[1][3][x - 45][0] = Index[x];
		cStoreItens[1][3][x - 45][1] = Value[x];
		cStoreItens[1][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[1][4][x - 60][0] = Index[x];
		cStoreItens[1][4][x - 60][1] = Value[x];
		cStoreItens[1][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate3() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore3` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[2][0][y][0] = Index[y];
		cStoreItens[2][0][y][1] = Value[y];
		cStoreItens[2][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[2][1][z - 15][0] = Index[z];
		cStoreItens[2][1][z - 15][1] = Value[z];
		cStoreItens[2][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[2][2][x - 30][0] = Index[x];
		cStoreItens[2][2][x - 30][1] = Value[x];
		cStoreItens[2][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[2][3][x - 45][0] = Index[x];
		cStoreItens[2][3][x - 45][1] = Value[x];
		cStoreItens[2][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[2][4][x - 60][0] = Index[x];
		cStoreItens[2][4][x - 60][1] = Value[x];
		cStoreItens[2][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate4() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore4` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[3][0][y][0] = Index[y];
		cStoreItens[3][0][y][1] = Value[y];
		cStoreItens[3][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[3][1][z - 15][0] = Index[z];
		cStoreItens[3][1][z - 15][1] = Value[z];
		cStoreItens[3][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[3][2][x - 30][0] = Index[x];
		cStoreItens[3][2][x - 30][1] = Value[x];
		cStoreItens[3][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[3][3][x - 45][0] = Index[x];
		cStoreItens[3][3][x - 45][1] = Value[x];
		cStoreItens[3][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[3][4][x - 60][0] = Index[x];
		cStoreItens[3][4][x - 60][1] = Value[x];
		cStoreItens[3][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate5() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore5` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[4][0][y][0] = Index[y];
		cStoreItens[4][0][y][1] = Value[y];
		cStoreItens[4][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[4][1][z - 15][0] = Index[z];
		cStoreItens[4][1][z - 15][1] = Value[z];
		cStoreItens[4][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[4][2][x - 30][0] = Index[x];
		cStoreItens[4][2][x - 30][1] = Value[x];
		cStoreItens[4][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[4][3][x - 45][0] = Index[x];
		cStoreItens[4][3][x - 45][1] = Value[x];
		cStoreItens[4][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[4][4][x - 60][0] = Index[x];
		cStoreItens[4][4][x - 60][1] = Value[x];
		cStoreItens[4][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate6() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore6` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[5][0][y][0] = Index[y];
		cStoreItens[5][0][y][1] = Value[y];
		cStoreItens[5][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[5][1][z - 15][0] = Index[z];
		cStoreItens[5][1][z - 15][1] = Value[z];
		cStoreItens[5][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[5][2][x - 30][0] = Index[x];
		cStoreItens[5][2][x - 30][1] = Value[x];
		cStoreItens[5][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[5][3][x - 45][0] = Index[x];
		cStoreItens[5][3][x - 45][1] = Value[x];
		cStoreItens[5][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[5][4][x - 60][0] = Index[x];
		cStoreItens[5][4][x - 60][1] = Value[x];
		cStoreItens[5][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate7() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore7` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[6][0][y][0] = Index[y];
		cStoreItens[6][0][y][1] = Value[y];
		cStoreItens[6][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[6][1][z - 15][0] = Index[z];
		cStoreItens[6][1][z - 15][1] = Value[z];
		cStoreItens[6][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[6][2][x - 30][0] = Index[x];
		cStoreItens[6][2][x - 30][1] = Value[x];
		cStoreItens[6][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[6][3][x - 45][0] = Index[x];
		cStoreItens[6][3][x - 45][1] = Value[x];
		cStoreItens[6][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[6][4][x - 60][0] = Index[x];
		cStoreItens[6][4][x - 60][1] = Value[x];
		cStoreItens[6][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate8() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore8` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[7][0][y][0] = Index[y];
		cStoreItens[7][0][y][1] = Value[y];
		cStoreItens[7][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[7][1][z - 15][0] = Index[z];
		cStoreItens[7][1][z - 15][1] = Value[z];
		cStoreItens[7][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[7][2][x - 30][0] = Index[x];
		cStoreItens[7][2][x - 30][1] = Value[x];
		cStoreItens[7][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[7][3][x - 45][0] = Index[x];
		cStoreItens[7][3][x - 45][1] = Value[x];
		cStoreItens[7][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[7][4][x - 60][0] = Index[x];
		cStoreItens[7][4][x - 60][1] = Value[x];
		cStoreItens[7][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate9() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore9` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[8][0][y][0] = Index[y];
		cStoreItens[8][0][y][1] = Value[y];
		cStoreItens[8][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[8][1][z - 15][0] = Index[z];
		cStoreItens[8][1][z - 15][1] = Value[z];
		cStoreItens[8][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[8][2][x - 30][0] = Index[x];
		cStoreItens[8][2][x - 30][1] = Value[x];
		cStoreItens[8][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[8][3][x - 45][0] = Index[x];
		cStoreItens[8][3][x - 45][1] = Value[x];
		cStoreItens[8][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[8][4][x - 60][0] = Index[x];
		cStoreItens[8][4][x - 60][1] = Value[x];
		cStoreItens[8][4][x - 60][2] = Estoque[x];
	}
}
void UpdateShopDonate10() {
	auto& pc = cSQL::instance();

	int Index[75] = {};
	int Value[75] = {};
	int Estoque[75] = {};
	int i = 0;

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `donatestore10` LIMIT 75");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		Index[i] = atoi(row[2]);
		Value[i] = atoi(row[3]);
		Estoque[i] = atoi(row[4]);
		i++;
	}
	mysql_free_result(result);
	for (int y = 0; y < 15; y++) {
		cStoreItens[9][0][y][0] = Index[y];
		cStoreItens[9][0][y][1] = Value[y];
		cStoreItens[9][0][y][2] = Estoque[y];
	}
	for (int z = 15; z < 30; z++) {
		cStoreItens[9][1][z - 15][0] = Index[z];
		cStoreItens[9][1][z - 15][1] = Value[z];
		cStoreItens[9][1][z - 15][2] = Estoque[z];
	}
	for (int x = 30; x < 45; x++) {
		cStoreItens[9][2][x - 30][0] = Index[x];
		cStoreItens[9][2][x - 30][1] = Value[x];
		cStoreItens[9][2][x - 30][2] = Estoque[x];
	}
	for (int x = 45; x < 60; x++) {
		cStoreItens[9][3][x - 45][0] = Index[x];
		cStoreItens[9][3][x - 45][1] = Value[x];
		cStoreItens[9][3][x - 45][2] = Estoque[x];
	}
	for (int x = 60; x < 75; x++) {
		cStoreItens[9][4][x - 60][0] = Index[x];
		cStoreItens[9][4][x - 60][1] = Value[x];
		cStoreItens[9][4][x - 60][2] = Estoque[x];
	}
}

void SendShopDonate(int conn) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	if (pUser[conn].RequestShopDelay != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].RequestShopDelay; // ATRASO PACOTE

		if (isTime < 15000)
		{
			return;
		}

	}
	pUser[conn].RequestShopDelay = GetTickCount64();


	MSG_UpdateDonateStore sm;
	memset(&sm, 0, sizeof(MSG_UpdateDonateStore));

	sm.Type = _MSG_UpdateDonateStore;
	sm.Size = sizeof(MSG_UpdateDonateStore);
	sm.ID = conn;

	for (int i = 0; i < 10; i++) {
		for (int x = 0; x < 5; x++) {
			for (int z = 0; z < 15; z++) {
				for (int c = 0; c < 3; c++) {
					sm.cStoreItens[i][x][z][c] = cStoreItens[i][x][z][c];
				}
			}
		}
	}

	int Size = sm.Size;

	if (Size > sizeof(MSG_UpdateDonateStore))
	{
		sm.Size = 0;
		return;
	}

	if (!pUser[conn].cSock.AddMessage((char*)&sm, sizeof(MSG_UpdateDonateStore)))
		CloseUser(conn);
}