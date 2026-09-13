#include "main.h"
#include "UISlot.h"
#define DROPLIST 3

#define RUNEACTIVE 46
#define ENERGYMAX 47
#define ENERGYPOS 48

void CClientInfo::Init()
{
	this->Info.Cash = 0;
	this->Server.Double = 0;
	this->Server.DropRate = 0;
	this->Server.ExpRate = 0;
	this->Ranking.State = 0;

	for (int i = 0; i < 50; i++) {
		this->Info.Keys[i];
		this->Ranking.RankLevel[i] = 0;
		this->Ranking.RankEvolution[i] = 1;
		this->Ranking.RankClasse[i] = 0;
		this->Ranking.PvP[i] = 0;
		strncpy(this->Ranking.RankName[i], "", sizeof(this->Ranking.RankName[i]));
	}
	for (int i = 0; i < 10; i++) {
		strncpy(this->DropList.MobName[i], "", sizeof(this->DropList.MobName[i]));
	}
	for (int i = 0; i < 64; i++) {
		this->DropList.Item[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		for (int x = 0; x < 3; x++) {
			for (int z = 0; z < 15; z++) {
				for (int c = 0; c < 3; c++) {
					this->Info.cStoreItens[i][x][z][c] = 0;
				}
			}
		}
	}
	strncpy(this->Info.Pix, "", sizeof(this->Info.Pix));
}

void CClientInfo::ReceiveInfo(char* Packet)
{
	auto player = GetChar();

	auto sm = reinterpret_cast<MSG_STANDARD*>(Packet);
	switch (sm->Type)
	{
	case _MSG_UpdateJoias: {
		auto p = reinterpret_cast<MSG_UpdateJoias*>(Packet);

		auto NewEquip = g_pInterface->Instance()->getGuiFromHandle<UISlot>(1655571);
		*(int*)((int)NewEquip + 0x400) = 0x5;
		NewEquip->deleteItemGui(0, 0);

		auto NewEquip2 = g_pInterface->Instance()->getGuiFromHandle<UISlot>(1655581);
		*(int*)((int)NewEquip2 + 0x400) = 0x5;
		NewEquip2->deleteItemGui(0, 0);

		STRUCT_ITEM item[2]{};
		memcpy(&item[0], &p->Joias[0], sizeof(STRUCT_ITEM));
		memcpy(&item[1], &p->Joias[1], sizeof(STRUCT_ITEM));

		if (item[0].sIndex != 0)
			NewEquip->setGui((int)NewEquip->New(item[0], 0, 0), 0, 0);

		if (item[1].sIndex != 0)
			NewEquip2->setGui((int)NewEquip2->New(item[1], 0, 0), 0, 0);
	}break;
	case _MSG_UpdateKeys: {
		auto p = reinterpret_cast<MSG_UpdateKeys*>(Packet);

		for (int i = 0; i < 50; i++) {
			this->Info.Keys[i] = p->Keys[i];

			if (this->Info.Keys[i] < 0)
				this->Info.Keys[i] = 0;
		}
	}break;
	case _MSG_UpdateItemLevel: {
		auto p = reinterpret_cast<MSG_UpdateItemLevel*>(Packet);

		auto player = GetChar();

		if (g_pClientInfo->Info.chave1 == 0 && g_pClientInfo->Info.Keys[0] >= 0) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Escolha um dos itens para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM item[5];
			int Index[5] = { 0, 0, 0, 0, 0 };

			if (player.Class == 0) //TK
			{
				Index[0] = 938;
				Index[1] = 941;
				Index[2] = 942;
				Index[3] = 944;
				Index[4] = 945;
			}
			if (player.Class == 1) //FM
			{
				Index[0] = 940;
				Index[1] = 943;
				Index[2] = 0;
				Index[3] = 0;
				Index[4] = 0;
			}
			if (player.Class == 2) //BM
			{
				Index[0] = 938;
				Index[1] = 941;
				Index[2] = 942;
				Index[3] = 939;
				Index[4] = 0;
			}
			if (player.Class == 3) //HT
			{
				Index[0] = 939;
				Index[1] = 938;
				Index[2] = 943;
				Index[3] = 0;
				Index[4] = 0;
			}

			for (int i = 0; i < 5; i++) {
				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				memset(&item[i], 0, sizeof(STRUCT_ITEM));
				item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (Index[i] == 938 || Index[i] == 939)
					ConfigR::ShopItemQnt = 2;

				if (item[i].sIndex != 0)
					JephiGrid->setGui((int)JephiGrid->New(item[i], 0, 0), 0, 0);
			}
			break;
		}
		if (g_pClientInfo->Info.chave1 == 1 && g_pClientInfo->Info.Keys[0] >= 0) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Clique no item para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM item[5];
			int Index[5] = { 3980, 0, 0, 0, 0 };

			for (int i = 0; i < 5; i++) {
				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				memset(&item[i], 0, sizeof(STRUCT_ITEM));
				item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (item[i].sIndex != 0)
					JephiGrid->setGui((int)JephiGrid->New(item[i], 0, 0), 0, 0);
			}
			break;
		}
		if (g_pClientInfo->Info.chave1 == 2 && g_pClientInfo->Info.Keys[0] >= 113) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Escolha um dos itens para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM item[5];
			int Index[5] = { 2373, 2368, 0, 0, 0 };

			for (int i = 0; i < 5; i++) {

				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				memset(&item[i], 0, sizeof(STRUCT_ITEM));
				item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (item[i].sIndex != 0) {
					item[i].stEffect[0].cEffect = 10;
					item[i].stEffect[0].cValue = 10;
					item[i].stEffect[1].cEffect = 10;
					item[i].stEffect[1].cValue = 10;
					item[i].stEffect[2].cEffect = 100;
					item[i].stEffect[2].cValue = 0;

					JephiGrid->setGui((int)JephiGrid->New(item[i], 0, 0), 0, 0);
				}
			}
			break;
		}
		if (g_pClientInfo->Info.chave1 == 3 && g_pClientInfo->Info.Keys[0] >= 124) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Clique em um dos itens para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM Item[5];

			int Index[5] = { 0, 0, 0, 0, 0 };

			if (player.Class == 0) //TK
			{
				Index[0] = 1216;
				Index[1] = 1219;
				Index[2] = 1207;
				Index[3] = 1213;
				Index[4] = 1210;
			}
			if (player.Class == 1) //FM
			{
				Index[0] = 1351;
				Index[1] = 1354;
				Index[2] = 1342;
				Index[3] = 1348;
				Index[4] = 1345;
			}
			if (player.Class == 2) //BM
			{
				Index[0] = 1501;
				Index[1] = 1504;
				Index[2] = 1492;
				Index[3] = 1498;
				Index[4] = 1495;
			}
			if (player.Class == 3) //HT
			{
				Index[0] = 1651;
				Index[1] = 1654;
				Index[2] = 1642;
				Index[3] = 1648;
				Index[4] = 1645;
			}

			for (int i = 0; i < 5; i++) {
				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				memset(&Item[i], 0, sizeof(STRUCT_ITEM));
				Item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (Item[i].sIndex != 0) {
					Item[i].stEffect[0].cEffect = 43;
					Item[i].stEffect[0].cValue = 3;
					Item[i].stEffect[1].cEffect = 0;
					Item[i].stEffect[1].cValue = 0;
					Item[i].stEffect[2].cEffect = 0;
					Item[i].stEffect[2].cValue = 0;
					JephiGrid->setGui((int)JephiGrid->New(Item[i], 0, 0), 0, 0);
				}
			}
			break;
		}
		if (g_pClientInfo->Info.chave1 == 4 && g_pClientInfo->Info.Keys[0] >= 143) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Escolha um dos itens para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM Item[5];

			for (int i = 0; i < 5; i++)
				memset(&Item[i], 0, sizeof(STRUCT_ITEM));

			int Index[5] = { 0, 0, 0, 0, 0 };

			if (player.Class == 0) //TK
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 854;
					Item[0].sIndex = 854;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 809;
					Index[1] = 809;
					Item[0].sIndex = 809;
					Item[1].sIndex = 809;

					for (int i = 0; i < 2; i++) {
						Item[i].stEffect[0].cEffect = 43;
						Item[i].stEffect[0].cValue = 5;
						Item[i].stEffect[1].cEffect = 2;
						Item[i].stEffect[1].cValue = 36;
						Item[i].stEffect[2].cEffect = 0;
						Item[i].stEffect[2].cValue = 0;
					}
				}
			}
			if (player.Class == 1) //FM
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 2127;
					Item[0].sIndex = 2127;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 2122;
					Item[0].sIndex = 2122;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 2;
					Item[0].stEffect[1].cValue = 36;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
			}
			if (player.Class == 2) //BM
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 854;
					Item[0].sIndex = 854;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 809;
					Index[1] = 809;
					Item[0].sIndex = 809;
					Item[1].sIndex = 809;

					for (int i = 0; i < 2; i++) {
						Item[i].stEffect[0].cEffect = 43;
						Item[i].stEffect[0].cValue = 5;
						Item[i].stEffect[1].cEffect = 2;
						Item[i].stEffect[1].cValue = 36;
						Item[i].stEffect[2].cEffect = 0;
						Item[i].stEffect[2].cValue = 0;
					}
				}
			}
			if (player.Class == 3) //HT
			{
				Index[0] = 2122;
				Item[0].sIndex = 2122;
				Item[0].stEffect[0].cEffect = 43;
				Item[0].stEffect[0].cValue = 5;
				Item[0].stEffect[1].cEffect = 2;
				Item[0].stEffect[1].cValue = 36;
				Item[0].stEffect[2].cEffect = 0;
				Item[0].stEffect[2].cValue = 0;
			}

			for (int i = 0; i < 5; i++) {
				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				Item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (Item[i].sIndex != 0) {
					JephiGrid->setGui((int)JephiGrid->New(Item[i], 0, 0), 0, 0);
				}
			}
			break;
		}
		if (g_pClientInfo->Info.chave1 == 5 && g_pClientInfo->Info.Keys[0] >= 255) {
			ConfigR::WindowControl = 9;

			ConfigR::ShopItemQnt = 1;

			auto PainelJephi = g_pInterface->Instance()->getGuiFromHandle<UIControl>(117900);
			PainelJephi->IsVisible = true;

			auto Text = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(117930);
			Text->setConstString("Escolha um dos itens para receber");
			Text->setTextColor(TNColor::CornBlue);

			STRUCT_ITEM Item[5];

			for (int i = 0; i < 5; i++)
				memset(&Item[i], 0, sizeof(STRUCT_ITEM));

			int Index[5] = { 0, 0, 0, 0, 0 };

			if (player.Class == 0) //TK
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3566;
					Item[0].sIndex = 3566;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3551;
					Index[1] = 3551;
					Item[0].sIndex = 3551;
					Item[1].sIndex = 3551;

					for (int i = 0; i < 2; i++) {
						Item[i].stEffect[0].cEffect = 43;
						Item[i].stEffect[0].cValue = 5;
						Item[i].stEffect[1].cEffect = 2;
						Item[i].stEffect[1].cValue = 36;
						Item[i].stEffect[2].cEffect = 0;
						Item[i].stEffect[2].cValue = 0;
					}
				}
			}
			if (player.Class == 1) //FM
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3582;
					Item[0].sIndex = 3582;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3556;
					Item[0].sIndex = 3556;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 2;
					Item[0].stEffect[1].cValue = 36;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
			}
			if (player.Class == 2) //BM
			{
				//MAGO
				if (player.BaseScore.Int > (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3566;
					Item[0].sIndex = 3566;
					Item[0].stEffect[0].cEffect = 43;
					Item[0].stEffect[0].cValue = 5;
					Item[0].stEffect[1].cEffect = 60;
					Item[0].stEffect[1].cValue = 16;
					Item[0].stEffect[2].cEffect = 0;
					Item[0].stEffect[2].cValue = 0;
				}
				//DANO
				if (player.BaseScore.Int < (player.BaseScore.Dex + player.BaseScore.Str)) {
					Index[0] = 3551;
					Index[1] = 3551;
					Item[0].sIndex = 3551;
					Item[1].sIndex = 3551;

					for (int i = 0; i < 2; i++) {
						Item[i].stEffect[0].cEffect = 43;
						Item[i].stEffect[0].cValue = 5;
						Item[i].stEffect[1].cEffect = 2;
						Item[i].stEffect[1].cValue = 36;
						Item[i].stEffect[2].cEffect = 0;
						Item[i].stEffect[2].cValue = 0;
					}
				}
			}
			if (player.Class == 3) //HT
			{
				Index[0] = 3556;
				Item[0].sIndex = 3556;
				Item[0].stEffect[0].cEffect = 43;
				Item[0].stEffect[0].cValue = 5;
				Item[0].stEffect[1].cEffect = 2;
				Item[0].stEffect[1].cValue = 36;
				Item[0].stEffect[2].cEffect = 0;
				Item[0].stEffect[2].cValue = 0;
			}

			for (int i = 0; i < 5; i++) {
				auto JephiGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(117901 + i);
				*(int*)((int)JephiGrid + 0x400) = 0x5;
				JephiGrid->deleteItemGui(0, 0);

				Item[i].sIndex = Index[i];
				ConfigR::ItemLevel[i] = Index[i];

				if (Item[i].sIndex != 0) {
					JephiGrid->setGui((int)JephiGrid->New(Item[i], 0, 0), 0, 0);
				}
			}
			break;
		}
	}break;
	case _MSG_SendTraje: {
		auto p = reinterpret_cast<MSG_SendTraje*>(Packet);
		
		if (p->Tipo == 1) {
			auto PainelTrajes = g_pInterface->Instance()->getGuiFromHandle<UIControl>(1882000);
			PainelTrajes->IsVisible = true;

			STRUCT_ITEM item[15];
			int TrajeIndex = 4500 - 22;
			for (int i = 0; i < 15; i++) {
				auto TrajeGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(1882001 + i);
				*(int*)((int)TrajeGrid + 0x400) = 0x5;
				TrajeGrid->deleteItemGui(0, 0);

				memset(&item[i], 0, sizeof(STRUCT_ITEM));
				item[i].sIndex = g_pClientInfo->Info.Keys[1 + i] + TrajeIndex;

				if (item[i].sIndex != 0 && (item[i].sIndex >= 4500 && item[i].sIndex <= 4549))
					TrajeGrid->setGui((int)TrajeGrid->New(item[i], 0, 0), 0, 0);
			}
		}	

	}break;		
	case _MSG_UpdateDonateStore:
	{
		auto p = reinterpret_cast<MSG_UpdateDonateStore*>(Packet);

		for (int i = 0; i < 3; i++) {
			for (int x = 0; x < 3; x++) {
				for (int z = 0; z < 15; z++) {
					for (int c = 0; c < 3; c++) {
						this->Info.cStoreItens[i][x][z][c] = p->cStoreItens[i][x][z][c];
					}
				}
			}
		}

	} break;
	case _MSG_Recaptcha: {
		auto p = reinterpret_cast<MSG_Recaptcha*>(Packet);

		char Word[7];
		strncpy(Word, "", 7);

		if (this->Info.Keys[45] == 0) {
			char Keywords[30][7] = {
			"BIKE", "AVIÃO", "BIKE", "CARRO", "NAVIO",
			"CARRO", "BIKE", "AVIÃO", "MOTO", "MOTO",
			"BIKE", "BIKE", "AVIÃO", "TREM", "BIKE",
			"AVIÃO", "BIKE", "CARRO", "NAVIO", "NAVIO",
			"NAVIO", "MOTO", "TREM", "TREM", "AVIÃO",
			"TREM", "NAVIO", "AVIÃO", "NAVIO", "MOTO"
			};

			strncpy(Word, Keywords[p->word], 7);
		}
		if (this->Info.Keys[45] == 1) {
			char Keywords1[30][7] = {
			"TREM", "BIKE", "CARRO", "BIKE", "AVIÃO",
			"TREM", "BIKE", "NAVIO", "MOTO", "BIKE",
			"CARRO", "MOTO", "NAVIO", "AVIÃO", "BIKE",
			"AVIÃO", "CARRO", "MOTO", "TREM", "MOTO",
			"BIKE", "AVIÃO", "BIKE", "NAVIO", "NAVIO",
			"AVIÃO", "NAVIO", "AVIÃO", "NAVIO", "NAVIO"
			};
			strncpy(Word, Keywords1[p->word], 7);
		}
		if (this->Info.Keys[45] == 2) {
			char Keywords2[30][7] = {
			"BIKE", "MOTO", "AVIÃO", "BIKE", "AVIÃO",
			"AVIÃO", "MOTO", "TREM", "NAVIO", "MOTO",
			"CARRO", "TREM", "CARRO", "BIKE", "CARRO",
			"NAVIO", "AVIÃO", "AVIÃO", "BIKE", "BIKE",
			"MOTO", "BIKE", "BIKE", "NAVIO", "MOTO",
			"NAVIO", "NAVIO", "AVIÃO", "TREM", "AVIÃO"
			};
			strncpy(Word, Keywords2[p->word], 7);
		}
		auto PainelCaptcha = g_pInterface->Instance()->getGuiFromHandle<UIControl>(1319001);
		PainelCaptcha->IsVisible = true;

		auto Desc = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1319002);
		Desc->setConstString("Clique na imagem que represente um(a): [%s]", Word);

		auto Desc2 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1319003);
		Desc2->setConstString("[ATENÇÃO] A próxima tentativa será somente em 2 minutos");

		auto PainelInv = g_pInterface->Instance()->getGuiFromHandle<UIControl>(589832);
		PainelInv->IsVisible = false;

	}break;
	case _MSG_SendRanking:
	{
		auto p = reinterpret_cast<MSG_SendRanking*>(Packet);
		
		for (int i = 0; i < 50; i++) {
			strncpy(this->Ranking.RankName[i], p->RankName[i], sizeof(this->Ranking.RankName[i]));
			this->Ranking.RankLevel[i] = p->RankLevel[i];
			this->Ranking.RankClasse[i] = p->RankClasse[i];
			this->Ranking.RankEvolution[i] = p->RankEvolution[i];
			this->Ranking.PvP[i] = p->PvP[i];
		}
		this->Ranking.State = p->State;

		auto PainelRanking = g_pInterface->Instance()->getGuiFromHandle<UIControl>(1919400);

		char Evolution[5][15] = { "Mortal", "Arch", "Celestial", "CelestialCS", "SubCelestial" };
		char Classe[4][4] = { "TK", "FM", "BM", "HT" };

		if (p->State == 0) {
			auto Pos = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919401);
			Pos->setConstString("Pos");
			auto Name = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919402);
			Name->setConstString("Nick");
			auto Level = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919403);
			Level->setConstString("Level");
			auto Evo = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919404);
			Evo->setConstString("Evolução");
			auto Class = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919404);
			Class->setConstString("Classe");

			auto Pos1 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919410);
			auto Pos2 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919411);
			auto Pos3 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919412);
			auto Pos4 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919413);
			auto Pos5 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919414);
			auto Pos6 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919415);
			auto Pos7 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919416);
			auto Pos8 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919417);
			auto Pos9 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919418);
			auto Pos10 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919419);
			auto Page = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919420);

			ConfigR::RankingPage = 1;

			Page->setConstString("%d", ConfigR::RankingPage);
			Pos1->setConstString("1. [%s][%d][%s][%s]", this->Ranking.RankName[0], this->Ranking.RankLevel[0], Evolution[this->Ranking.RankEvolution[0] - 1], Classe[this->Ranking.RankClasse[0]]);
			Pos2->setConstString("2. [%s][%d][%s][%s]", this->Ranking.RankName[1], this->Ranking.RankLevel[1], Evolution[this->Ranking.RankEvolution[1] - 1], Classe[this->Ranking.RankClasse[1]]);
			Pos3->setConstString("3. [%s][%d][%s][%s]", this->Ranking.RankName[2], this->Ranking.RankLevel[2], Evolution[this->Ranking.RankEvolution[2] - 1], Classe[this->Ranking.RankClasse[2]]);
			Pos4->setConstString("4. [%s][%d][%s][%s]", this->Ranking.RankName[3], this->Ranking.RankLevel[3], Evolution[this->Ranking.RankEvolution[3] - 1], Classe[this->Ranking.RankClasse[3]]);
			Pos5->setConstString("5. [%s][%d][%s][%s]", this->Ranking.RankName[4], this->Ranking.RankLevel[4], Evolution[this->Ranking.RankEvolution[4] - 1], Classe[this->Ranking.RankClasse[4]]);
			Pos6->setConstString("6. [%s][%d][%s][%s]", this->Ranking.RankName[5], this->Ranking.RankLevel[5], Evolution[this->Ranking.RankEvolution[5] - 1], Classe[this->Ranking.RankClasse[5]]);
			Pos7->setConstString("7. [%s][%d][%s][%s]", this->Ranking.RankName[6], this->Ranking.RankLevel[6], Evolution[this->Ranking.RankEvolution[6] - 1], Classe[this->Ranking.RankClasse[6]]);
			Pos8->setConstString("8. [%s][%d][%s][%s]", this->Ranking.RankName[7], this->Ranking.RankLevel[7], Evolution[this->Ranking.RankEvolution[7] - 1], Classe[this->Ranking.RankClasse[7]]);
			Pos9->setConstString("9. [%s][%d][%s][%s]", this->Ranking.RankName[8], this->Ranking.RankLevel[8], Evolution[this->Ranking.RankEvolution[8] - 1], Classe[this->Ranking.RankClasse[8]]);
			Pos10->setConstString("10. [%s][%d][%s][%s]", this->Ranking.RankName[9], this->Ranking.RankLevel[9], Evolution[this->Ranking.RankEvolution[9] - 1], Classe[this->Ranking.RankClasse[9]]);			
		}
		if (p->State == 1) {
			auto Pos = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919401);
			Pos->setConstString("Pos");
			auto Name = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919402);
			Name->setConstString("Nick");
			auto PvP = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919403);
			PvP->setConstString("PvP");
			auto Evo = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919404);
			Evo->setConstString("Evolução");
			auto Class = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919404);
			Class->setConstString("Classe");

			auto Pos1 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919410);
			auto Pos2 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919411);
			auto Pos3 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919412);
			auto Pos4 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919413);
			auto Pos5 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919414);
			auto Pos6 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919415);
			auto Pos7 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919416);
			auto Pos8 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919417);
			auto Pos9 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919418);
			auto Pos10 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919419);
			auto Page = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(1919420);

			ConfigR::RankingPage = 1;

			Page->setConstString("%d", ConfigR::RankingPage);
			Pos1->setConstString("1. [%s][%d][%s][%s]", this->Ranking.RankName[0], this->Ranking.PvP[0], Evolution[this->Ranking.RankEvolution[0] - 1], Classe[this->Ranking.RankClasse[0]]);
			Pos2->setConstString("2. [%s][%d][%s][%s]", this->Ranking.RankName[1], this->Ranking.PvP[1], Evolution[this->Ranking.RankEvolution[1] - 1], Classe[this->Ranking.RankClasse[1]]);
			Pos3->setConstString("3. [%s][%d][%s][%s]", this->Ranking.RankName[2], this->Ranking.PvP[2], Evolution[this->Ranking.RankEvolution[2] - 1], Classe[this->Ranking.RankClasse[2]]);
			Pos4->setConstString("4. [%s][%d][%s][%s]", this->Ranking.RankName[3], this->Ranking.PvP[3], Evolution[this->Ranking.RankEvolution[3] - 1], Classe[this->Ranking.RankClasse[3]]);
			Pos5->setConstString("5. [%s][%d][%s][%s]", this->Ranking.RankName[4], this->Ranking.PvP[4], Evolution[this->Ranking.RankEvolution[4] - 1], Classe[this->Ranking.RankClasse[4]]);
			Pos6->setConstString("6. [%s][%d][%s][%s]", this->Ranking.RankName[5], this->Ranking.PvP[5], Evolution[this->Ranking.RankEvolution[5] - 1], Classe[this->Ranking.RankClasse[5]]);
			Pos7->setConstString("7. [%s][%d][%s][%s]", this->Ranking.RankName[6], this->Ranking.PvP[6], Evolution[this->Ranking.RankEvolution[6] - 1], Classe[this->Ranking.RankClasse[6]]);
			Pos8->setConstString("8. [%s][%d][%s][%s]", this->Ranking.RankName[7], this->Ranking.PvP[7], Evolution[this->Ranking.RankEvolution[7] - 1], Classe[this->Ranking.RankClasse[7]]);
			Pos9->setConstString("9. [%s][%d][%s][%s]", this->Ranking.RankName[8], this->Ranking.PvP[8], Evolution[this->Ranking.RankEvolution[8] - 1], Classe[this->Ranking.RankClasse[8]]);
			Pos10->setConstString("10. [%s][%d][%s][%s]", this->Ranking.RankName[9], this->Ranking.PvP[9], Evolution[this->Ranking.RankEvolution[9] - 1], Classe[this->Ranking.RankClasse[9]]);
		}
		
	}break;
	case _MSG_MailItem:
	{
		auto MailButton = g_pInterface->Instance()->getGuiFromHandle<UIControl>(1212300);
		MailButton->IsVisible = false;

		auto p = reinterpret_cast<MSG_MailItem*>(Packet);
		this->MailItem.Index = p->Index;
		this->MailItem.sEffect[0] = p->sEffect[0];
		this->MailItem.sValue[0] = p->sValue[0];
		this->MailItem.sEffect[1] = p->sEffect[1];
		this->MailItem.sValue[1] = p->sValue[1];
		this->MailItem.sEffect[2] = p->sEffect[2];
		this->MailItem.sValue[2] = p->sValue[2];

		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));
		item.sIndex = this->MailItem.Index;
		item.stEffect[0].cEffect = this->MailItem.sEffect[0];
		item.stEffect[0].cValue = this->MailItem.sValue[0];
		item.stEffect[1].cEffect = this->MailItem.sEffect[1];
		item.stEffect[1].cValue = this->MailItem.sValue[1];
		item.stEffect[2].cEffect = this->MailItem.sEffect[2];
		item.stEffect[2].cValue = this->MailItem.sValue[2];


		auto MailItem = g_pInterface->Instance()->getGuiFromHandle<UISlot>(121202);
		*(int*)((int)MailItem + 0x400) = 0x5;
		MailItem->deleteItemGui(0, 0);

		if (this->MailItem.Index != 0) {			
			MailButton->IsVisible = true;
			MailItem->setGui((int)MailItem->New(item, 0, 0), 0, 0);
		}
		

	}break;
	case _MSG_ServerInfos:
	{
		auto p = reinterpret_cast<MSG_ServerInfos*>(Packet);
		this->Server.Double = p->Double;
		this->Server.DropRate = p->DropRate;
		this->Server.ExpRate = p->ExpRate;

		auto Tx1 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(981403);
		if (this->Server.Double == 0) {
			Tx1->setConstString("DOUBLE DESATIVADO");
			Tx1->setTextColor(TNColor::Orange);
		}
		if (this->Server.Double == 1) {
			Tx1->setConstString("DOUBLE ATIVO");
			Tx1->setTextColor(TNColor::Speak);
		}

		auto Tx2 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(981404);
		Tx2->setConstString("Drop Bonus: %d", this->Server.DropRate);
		Tx2->setTextColor(TNColor::CornBlue);

		auto Tx3 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(981405);
		Tx3->setConstString("Exp Bonus: %d", this->Server.ExpRate);
		Tx3->setTextColor(TNColor::DeepPink);
	}break;
	case _MSG_SendDListNames:
	{
		auto p = reinterpret_cast<MSG_SendDListNames*>(Packet);

		ConfigR::DropListTpages = 1;
		int nMobs = 0;

		for (int i = 0; i < 30; i++)
			strncpy(this->DropList.MobName[i], p->MobName[i], sizeof(this->DropList.MobName[i]));

		for (int y = 0; y < 30; y++) {
			if (strcmp(p->MobName[y], "") == 0) {
				nMobs = y;
				break;
			}
		}

		auto NewWindowTX1 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179511);
		auto NewWindowTX2 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179512);
		auto NewWindowTX3 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179513);
		auto NewWindowTX4 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179514);
		auto NewWindowTX5 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179515);
		auto NewWindowTX6 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179516);
		auto NewWindowTX7 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179517);
		auto NewWindowTX8 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179518);
		auto NewWindowTX9 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179519);
		auto NewWindowTX10 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179520);

		int page = (ConfigR::DropListPage * 10);

		NewWindowTX1->setConstString("%s", g_pClientInfo->DropList.MobName[page + 0]);
		NewWindowTX2->setConstString("%s", g_pClientInfo->DropList.MobName[page + 1]);
		NewWindowTX3->setConstString("%s", g_pClientInfo->DropList.MobName[page + 2]);
		NewWindowTX4->setConstString("%s", g_pClientInfo->DropList.MobName[page + 3]);
		NewWindowTX5->setConstString("%s", g_pClientInfo->DropList.MobName[page + 4]);
		NewWindowTX6->setConstString("%s", g_pClientInfo->DropList.MobName[page + 5]);
		NewWindowTX7->setConstString("%s", g_pClientInfo->DropList.MobName[page + 6]);
		NewWindowTX8->setConstString("%s", g_pClientInfo->DropList.MobName[page + 7]);
		NewWindowTX9->setConstString("%s", g_pClientInfo->DropList.MobName[page + 8]);
		NewWindowTX10->setConstString("%s", g_pClientInfo->DropList.MobName[page + 9]);
		
		if (nMobs >= 11)
			ConfigR::DropListTpages = 2;

		if (nMobs >= 21)
			ConfigR::DropListTpages = 3;

		auto PageTx = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(179521);
		PageTx->setConstString("%d/%d", (ConfigR::DropListPage + 1), ConfigR::DropListTpages);
	}break;
	case _MSG_UpdateDropList: {
		auto p = reinterpret_cast<MSG_UpdateDropList*>(Packet);

		int Slot = 0;

		for (int i = 0; i < 64; i++) {
			this->DropList.Item[i] = 0;

			bool exist = false;

			if (p->Item[i] == 0)
				continue;

			for (int y = 0; y < 64; y++) {
				if (p->Item[i] == this->DropList.Item[y])
					exist = true;
			}

			if (exist)
				continue;

			this->DropList.Item[Slot] = p->Item[i];
			Slot++;
		}

		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		auto DropListGrid = g_pInterface->Instance()->getGuiFromHandle<UISlot>(179600);
		*(int*)((int)DropListGrid + 0x400) = 0x5;
		DropListGrid->deleteItems();

		for (int i = 0; i < 64; i++) {
			item.sIndex = this->DropList.Item[i];

			if (item.sIndex == 0)
				continue;

			DropListGrid->setGui((int)DropListGrid->New(item, 0, 0), i % 8, i / 8);
		}
	} break;
	case _MSG_UpdateDonate:
	{
		auto p = reinterpret_cast<MSG_UpdateDonate*>(Packet);

		this->Info.Cash = p->Cash;
		if (this->Info.Cash < 0)
			this->Info.Cash = 0;

		strncpy(this->Info.Pix, p->Pix, sizeof(this->Info.Pix));

		auto DonateInv = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(250099);
		auto PixTx4 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(188517);
				
		DonateInv->setConstString("Diamantes: %d", g_pClientInfo->Info.Cash);
		DonateInv->setTextColor(TNColor::CornBlue);

		PixTx4->setConstString("Diamantes: %d", g_pClientInfo->Info.Cash);
		PixTx4->setTextColor(TNColor::CornBlue);

		auto PixTx3 = g_pInterface->Instance()->getGuiFromHandle<UITextControl>(188516);
		PixTx3->setConstString("Sua Chave: %s", g_pClientInfo->Info.Pix);
		PixTx3->setTextColor(TNColor::CornBlue);
	}break;
	default:
		break;
	}
}