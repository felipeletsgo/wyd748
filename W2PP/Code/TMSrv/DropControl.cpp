#include "..\Basedef.h"
#include "..\ItemEffect.h"
#include "Language.h"
#include "Server.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "CCastleZakum.h"
#include "Functions.h"


int ControleDropGold(int conn, int target, int DropedGold)
{
	int GoldFinal = DropedGold;





	return GoldFinal;
}

STRUCT_ITEM* ControleDropItem(int conn, int target, int bonus, int PosX, int PosY, STRUCT_ITEM* Item)
{
	STRUCT_ITEM* ItemDrop = Item;

	if (ItemDrop->sIndex != 0 && CCastleZakum::KeyDrop(target, conn, PosX, PosY, ItemDrop) == TRUE)
	{
		SetItemBonus(ItemDrop, pMob[target].MOB.CurrentScore.Level, 0, bonus);

		BlockAdd(ItemDrop, EF_DAMAGE, 63, 72);
		BlockAdd(ItemDrop, EF_MAGIC, 28, 32);

		BlockAdd(ItemDrop, EF_DAMAGEADD,  63, 72);
		BlockAdd(ItemDrop, EF_MAGICADD,  28, 32);

		//Filtro de Drop
		if (AutoDrop(conn, ItemDrop->sIndex) == false)
			return ItemDrop;

		if (DropItemControl(ItemDrop->sIndex) == false)
			return ItemDrop;

		//Retira o drop duplo da lan
		if ((pMob[conn].TargetX > 3604 && pMob[conn].TargetY > 3604 && pMob[conn].TargetX < 3691 && pMob[conn].TargetY < 3690) ||
			(pMob[conn].TargetX > 3732 && pMob[conn].TargetY > 3476 && pMob[conn].TargetX < 3819 && pMob[conn].TargetY < 3563) ||
			(pMob[conn].TargetX > 3860 && pMob[conn].TargetY > 3604 && pMob[conn].TargetX < 3946 && pMob[conn].TargetY < 3690)
			) {

			int List[]{
				419, 420, 4019, 2404, 2399, 4026, 2400, 2405, 4020, 2403, 2398, 5811
			};
			for (int i = 0; i < (sizeof(List) / 4); i++) {
				if (ItemDrop->sIndex == List[i])
					return ItemDrop;
			}
		}

		/*if (ItemDrop->sIndex == 1740 || ItemDrop->sIndex == 1741)
			SendNotice(strFmt("O jogador [%s] dropou uma [%s]", pMob[conn].MOB.MobName, g_pItemList[ItemDrop->sIndex].Name));*/

		//agrupador automatico teste
		if (AgroupList(conn, ItemDrop->sIndex) == true) {
			// Extrair quantidade do item se tiver efeito 61
			int dropAmount = 1;
			if (ItemDrop->stEffect[0].cEffect == 61)
				dropAmount = ItemDrop->stEffect[0].cValue;
			else if (ItemDrop->stEffect[1].cEffect == 61)
				dropAmount = ItemDrop->stEffect[1].cValue;
			else if (ItemDrop->stEffect[2].cEffect == 61)
				dropAmount = ItemDrop->stEffect[2].cValue;

			SendItemagruparEx(conn, ItemDrop->sIndex, dropAmount);

			if (LogList(ItemDrop->sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[ItemDrop->sIndex].Name, ItemDrop->sIndex, ItemDrop->stEffect[0].cEffect, ItemDrop->stEffect[0].cValue, ItemDrop->stEffect[1].cEffect, ItemDrop->stEffect[1].cValue, ItemDrop->stEffect[2].cEffect, ItemDrop->stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return ItemDrop;
		}

		if (PutItem(conn, ItemDrop))
		{
			if (LogList(ItemDrop->sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[ItemDrop->sIndex].Name, ItemDrop->sIndex, ItemDrop->stEffect[0].cEffect, ItemDrop->stEffect[0].cValue, ItemDrop->stEffect[1].cEffect, ItemDrop->stEffect[1].cValue, ItemDrop->stEffect[2].cEffect, ItemDrop->stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}
		}

		int PedacoEquilibrio = ItemDrop->sIndex == 4125;
		if (PedacoEquilibrio)
			DoTeleport(conn, 1970 + rand() % 2, 1580 + rand() % 2);
	}
	return ItemDrop;
}

bool AutoDrop(int conn, int itemId)
{
	if ((pMob[conn].MOB.Equip[13].sIndex == 3901 || pMob[conn].MOB.Equip[13].sIndex == 3902 || pMob[conn].MOB.Equip[13].sIndex == 3916) && pMob[conn].FiltroState == 1) {

		for (int i = 0; i < 50; i++)
		{
			if (pMob[conn].FiltroID[i] <= 0)
			{
				continue;
			}

			if (pMob[conn].FiltroID[i] == itemId)
				return true;
		}

		//SendMsgExp(conn, strFmt("Item [%s - ID [%d]] foi deletado pelo autodrop.", g_pItemList[itemId].Name, itemId), TNColor::GreenYellow, false);
		return false;
	}
	 else
		return true;
}

bool AgroupList(int conn, int itemId)
{
	//Box Quest
	if (itemId >= 4117 && itemId <= 4121)
		return true;

	//Olho Crescente
	if (itemId == 481)
		return true;

	if (pMob[conn].MOB.Equip[13].sIndex == 3901 || pMob[conn].MOB.Equip[13].sIndex == 3902 || pMob[conn].MOB.Equip[13].sIndex == 3916) {

		int List[] = {
			4026, //Moeda 1kk
			4027, //Moeda 5kk
			4028, //Moeda 10kk
			4019, //Classe_D
			4020, //Classe_E
			4018, //Classe_C
			4017, //Classe_B
			4016, //Classe_A
			419, //Resto_de_Oriharucon
			420, //Resto_de_Lactolerium
			412, //Poeira_de_Oriharucon
			413, //Poeira_de_Lactolerium
			578, //Pedra_de_Adamantita
			2441, //Diamante
			2442, //Esmeralda
			2443, //Coral
			2444, //Garnet
			1774, //Pedra_do_S�bio
			3140, //Pedra_da_Luz
			2396, //�mago_de_Cav_s/Sela_N
			2397, //�mago_de_Cav_Fantasm_N
			2398, //�mago_de_Cavalo_Leve_N
			2399, //�mago_de_Cavalo_Equip_N
			2400, //�mago_de_Andaluz_N
			2401, //�mago_de_Ca_s/Sela_B
			2402, //�mago_de_Cav_Fantasm_B
			2403, //�mago_de_Cavalo_Leve_B
			2404, //�mago_de_Cavalo_Equip_B
			2405, //�mago_de_Andaluz_B
			2406, //�mago_de_Fenrir
			2411, //�mago_de_Unic�rnio
			2412, //�mago_de_P�gasus
			2413, //�mago_de_Unisus
			2390, //�mago_de_Porco
			2391, //�mago_de_Javali
			2392, //�mago_de_Lobo
			2393, //�mago_de_Drag�o_Menor
			2394, //�mago_de_Urso
			5972, //Fragmento_de_Refina��o
			5973, //Fragmento_da_Prote��o
			5974, //Fragmento_de_1kk
			5975, //Fragmento_de_5kk
			5976, //Fragmento_de_10kk
			5977, //Fragmento_do_colar
			5978, //Fragmento_de_brinco
			5979, //Fragmento_de_amuleto
			5980, //Sucata
			5981, //Fragmento_de_Bolsa
			5982, //Fragmento_de_Runas
			5983, //Fragmento_de_Adamantita
			4026, //Moeda_de_1kk
				4027, //Moeda_de_5kk
				4028, //Moeda_de_10kk
				5973, //Fragmento_da_Prote��o
				5974, //Fragmento_de_1kk
				5975, //Fragmento_de_5kk
				5976, //Fragmento_de_10kk
				5977, //Fragmento_do_colar
				5978, //Fragmento_de_brinco
				5979, //Fragmento_de_amuleto
				5980, //Sucata
				5981, //Fragmento_de_Bolsa
				5982, //Fragmento_de_Runas
				5983, //Fragmento_de_Adamantita
				5776, //Fragmento_da_Evolu��o
				5777, //Fragmento_do_Avatar
				5778, //Fragmento_do_Artes�o
				5779, //Fragmento_da_Fada
				5780, //Fragmento_de_Sephirot
				5782, //Fragmento_G�lido
				5783, //Fragmento_do_Jolheiro
				5752, //Power_Belt_Frag
				5753, //Power_Belt_Frag
				5754, //Mana's_Belt_Frag
				5755, //Mana's_Belt_Frag
				5756, //Physical_Belt_Frag
				5757, //Physical_Belt_Frag
				5758, //Magical_Belt_Frag
				5759, //Magical_Belt_Frag
				5760, //Power_Necklace_Frag
				5761, //Power_Necklace_Frag
				5762, //Mana's_Necklace_Frag
				5763, //Mana's_Necklace_Frag
				5764, //Physical_Necklace_Frag
				5765, //Physical_Necklace_Frag
				5766, //Magical_Necklace_Frag
				5767, //Magical_Necklace_Frag
				5768, //Griupan_Frag
				5769, //Griupan_Frag
				5770, //Griupan_Frag
				5771, //Ovo_de_P�scoa_I
				5772, //Ovo_de_P�scoa_II
				5773, //Ovo_de_P�scoa_III
				5774, //Ovo_de_P�scoa_IV
				5775, //Ovo_de_P�scoa_V
				5776, //Fragmento_da_Evolu��o
				5777, //Fragmento_do_Avatar
				5778, //Fragmento_do_Artes�o
				5779, //Fragmento_de_Fada
				5780, //Fragmento_de_Sephirot
				5781, //Escritura_do_Torneio
				5782, //Fragmento_G�lido
				5783, //Fragmento_do_Jolheiro
				5796, //Fragmento_Espiritual
				5797, //Fragmento_Mithril
				5825, //Fragmento_da_Escurid�o
				5826, //Estilha�o_da_Mem�ria
				5827, //Mem�ria_do_Artes�o
				5830, //Livro_de_Encantamentos
				5831, //Poeira_de_Infirmus
				5834, //Pedra_Mortal
				5835, //P�_de_Fada
				5836, //P�gina_do_Livro_de_Rel�quias
				5837, //Po��o_do_Alquimista
				5840, //Ba�_M�stico(N)
				5841, //Ba�_M�stico(M)
				5842, //Ba�_M�stico(A)
				5843, //Ba�_M�stico(Le)
			5110, //Ansuz
			5111, //Uraz
			5112, //Thurisaz
			5113, //Fehu
			5114, //Raidho
			5115, //Kenaz
			5116, //Naudhiz
			5117, //Gebo
			5118, //Wunjo
			5119, //Hagalaz
			5120, //Isa
			5121, //Jara
			5122, //Eihwaz
			5123, //Perthro
			5124, //Elhaz
			5125, //Sowilo
			5126, //Berkano
			5127, //Ehwaz
			5128, //Tiwaz
			5129, //Mannaz
			5130, //Laguz
			5131, //Dagaz
			5132, //Ing
			5133, //Othel
			5776, //Fragmento_da_Evolu��o
			5777, //Fragmento_do_Avatar
			5778, //Fragmento_do_Artes�o
			5779, //Fragmento_da_Fada
			5780, //Fragmento_de_Sephirot
			5782, //Fragmento_G�lido
			5783, //Fragmento_do_Jolheiro
			2395 //�mago_de_Dente_de_Sabre
		};

		for (int i = 0; i < (sizeof(List) / 4); i++) {
			if (itemId == List[i])
				return true;
		}
		return false;
	}
	return false;
}

bool LogList(int itemId)
{
	int List[] = {
		4026, //Moeda 1kk
		412, //Poeira_de_Oriharucon
		413, //Poeira_de_Lactolerium
		1774, //Pedra_do_S�bio
		2441, //Diamante
		2442, //Esmeralda
		2443, //Coral
		2444, //Garnet
		3140, //Pedra_da_Luz
		2399, //�mago_de_Cavalo_Equip_N
		2404, //�mago_de_Cavalo_Equip_B
		2400, //�mago_de_Andaluz_N
		2405, //�mago_de_Andaluz_B
		2411, //�mago_de_Unic�rnio
		2412, //�mago_de_P�gasus
		2413, //�mago_de_Unisus
		2414, //�mago_de_Grifo
		2416, //�mago_de_Grifo_Sang
		2417, //�mago_de_Svadilfari
		2418, //�mago_de_Sleipnir
		2409, //�mago_de_Tigre_de_Fogo
		2410, //�mago_de_Drag�o_Verm
		1741, //Alma_da_F�nix
		1740, //Alma_do_Unic�rnio
		1742, //Pedra_da_Imortalidade
		5110, //Ansuz
		5111, //Uraz
		5112, //Thurisaz
		5113, //Fehu
		5114, //Raidho
		5115, //Kenaz
		5116, //Naudhiz
		5117, //Gebo
		5118, //Wunjo
		5119, //Hagalaz
		5120, //Isa
		5121, //Jara
		5122, //Eihwaz
		5123, //Perthro
		5124, //Elhaz
		5125, //Sowilo
		5126, //Berkano
		5127, //Ehwaz
		5128, //Tiwaz
		5129, //Mannaz
		5130, //Laguz
		5131, //Dagaz
		5132, //Ing
		5133, //Othel
	};

	for (int i = 0; i < (sizeof(List) / 4); i++) {
		if (itemId == List[i])
			return true;
	}
	return false;
}

void CartaDrop(int conn, int target, int type)
{
	if (type == 1) {
		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		int _rand = rand() % 16;

		if (_rand == 0 || _rand == 1 || _rand == 2)
			item.sIndex = 419; //Resto_de_Oriharucon

		else if (_rand == 5)
			item.sIndex = 413; //Resto_de_Lactolerium

		else if (_rand == 12)
			item.sIndex = 412; //Poeira Ori

		else if (_rand == 15)
			item.sIndex = 413; //Poeira Lac

		SetItemBonus(&item, 0, 0, 0);

		//agrupador automatico teste
		if (AgroupList(conn, item.sIndex) == true && item.sIndex != 0) {
			int dropAmount = 1;
			if (item.stEffect[0].cEffect == 61)
				dropAmount = item.stEffect[0].cValue;
			else if (item.stEffect[1].cEffect == 61)
				dropAmount = item.stEffect[1].cValue;
			else if (item.stEffect[2].cEffect == 61)
				dropAmount = item.stEffect[2].cValue;

			SendItemagruparEx(conn, item.sIndex, dropAmount);

			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}

		if (PutItem(conn, &item) && item.sIndex != 0)
		{
			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}
	}

	if (type == 2) {
		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		int _rand = rand() % 16;

		if (_rand == 0 || _rand == 1)
			item.sIndex = 419; //Resto_de_Ori

		else if (_rand == 5)
			item.sIndex = 413; //Poeira Lac

		else if (_rand == 15)
			item.sIndex = 420; // Resto_de_Lactolerium

		SetItemBonus(&item, 0, 0, 0);

		//agrupador automatico teste
		if (AgroupList(conn, item.sIndex) == true && item.sIndex != 0) {
			int dropAmount = 1;
			if (item.stEffect[0].cEffect == 61)
				dropAmount = item.stEffect[0].cValue;
			else if (item.stEffect[1].cEffect == 61)
				dropAmount = item.stEffect[1].cValue;
			else if (item.stEffect[2].cEffect == 61)
				dropAmount = item.stEffect[2].cValue;

			SendItemagruparEx(conn, item.sIndex, dropAmount);

			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}

		if (PutItem(conn, &item) && item.sIndex != 0)
		{
			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}
	}
	if (type == 3) {
		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		int _rand = rand() % 23;

		if (_rand == 0 || _rand == 1)
			item.sIndex = 5974; //Fragmento_de_1kk

		else if (_rand == 2 || _rand == 3)
			item.sIndex = 5974; //Fragmento_de_1kk

		else if (_rand == 4 || _rand == 5)
			item.sIndex = 5974; //Fragmento_de_1kk

		else if (_rand == 6 || _rand == 7)
			item.sIndex = 5974; //Fragmento_de_1kk

		else if (_rand == 17 || _rand == 18)
			item.sIndex = 5974; //Fragmento_de_1kk

		else if (_rand == 10)
			item.sIndex = 5974; //Fragmento_de_1kk

		SetItemBonus(&item, 0, 0, 0);

		//agrupador automatico teste
		if (AgroupList(conn, item.sIndex) == true && item.sIndex != 0) {
			int dropAmount = 1;
			if (item.stEffect[0].cEffect == 61)
				dropAmount = item.stEffect[0].cValue;
			else if (item.stEffect[1].cEffect == 61)
				dropAmount = item.stEffect[1].cValue;
			else if (item.stEffect[2].cEffect == 61)
				dropAmount = item.stEffect[2].cValue;

			SendItemagruparEx(conn, item.sIndex, dropAmount);

			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}

		if (PutItem(conn, &item) && item.sIndex != 0)
		{
			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}
	}
	if (type == 4) {
		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		int _rand = rand() % 10;

		if (_rand == 0 || _rand == 1 || _rand == 2 || _rand == 3)
			item.sIndex = 412; //Poeira Ori

		else if (_rand == 5 || _rand == 6)
			item.sIndex = 413; //Poeira Lac

		else if (_rand == 14)
			item.sIndex = 419; //Resto_de_Oriharucon

		else if (_rand == 17)
			item.sIndex = 420; //Resto_de_Lactolerium


		SetItemBonus(&item, 0, 0, 0);

		//agrupador automatico teste
		if (AgroupList(conn, item.sIndex) == true && item.sIndex != 0) {
			int dropAmount = 1;
			if (item.stEffect[0].cEffect == 61)
				dropAmount = item.stEffect[0].cValue;
			else if (item.stEffect[1].cEffect == 61)
				dropAmount = item.stEffect[1].cValue;
			else if (item.stEffect[2].cEffect == 61)
				dropAmount = item.stEffect[2].cValue;

			SendItemagruparEx(conn, item.sIndex, dropAmount);

			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}

		if (PutItem(conn, &item) && item.sIndex != 0)
		{
			if (LogList(item.sIndex) == true) {
				snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
				MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
			}

			return;
		}
	}
}
void LanDrop(int conn, int target)
{
	int Lan = 0;

	if (pMob[conn].TargetX > 3604 && pMob[conn].TargetY > 3604 && pMob[conn].TargetX < 3691 && pMob[conn].TargetY < 3690)
		Lan = 0;

	if (pMob[conn].TargetX > 3732 && pMob[conn].TargetY > 3476 && pMob[conn].TargetX < 3819 && pMob[conn].TargetY < 3563)
		Lan = 1;

	if (pMob[conn].TargetX > 3860 && pMob[conn].TargetY > 3604 && pMob[conn].TargetX < 3946 && pMob[conn].TargetY < 3690)
		Lan = 2;


	STRUCT_ITEM item;
	memset(&item, 0, sizeof(STRUCT_ITEM));
		
	int _rand = rand() % 100;

	if (Lan == 0) {

		// resto ori 10%
		if (_rand > 0 && _rand < 10) {
			item.sIndex = 419;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// resto de lac 05%
		if (_rand > 39 && _rand < 45) {
			item.sIndex = 420;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		//classe D 10%
		if (_rand > 74 && _rand < 85) {
			item.sIndex = 4019;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// Amago 5%
		if (_rand > 84 && _rand < 90) {
			item.sIndex = 2404;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// Amago 5%
		if (_rand > 89 && _rand < 95) {
			item.sIndex = 2399;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// Barra 1kk 5%
		if (_rand > 94 && _rand < 100) {
			item.sIndex = 4026;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}
	}

	if (Lan == 1) {

		// resto ori 20%
		if (_rand > 0 && _rand < 20) {
			item.sIndex = 419;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// resto de lac 15%
		if (_rand > 31 && _rand < 45) {
			item.sIndex = 420;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		//classe E 5%
		if (_rand > 51 && _rand < 57) {
			item.sIndex = 4020;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}

		// Amago 4%
		if (_rand > 60 && _rand < 65) {
			item.sIndex = 2404;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}
		// Amago 4%
		if (_rand > 70 && _rand < 75) {
			item.sIndex = 2399;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}
		// Barra 1kk 5%
		if (_rand > 89 && _rand < 95) {
			item.sIndex = 4026;
			item.stEffect[0].cEffect = 0;
			item.stEffect[0].cValue = 0;
			item.stEffect[1].cEffect = 0;
			item.stEffect[1].cValue = 0;
			item.stEffect[2].cEffect = 0;
			item.stEffect[2].cValue = 0;
		}
	}

	//Filtro de Drop
	if (AutoDrop(conn, item.sIndex) == false && item.sIndex != 0)
		return;

	//agrupador automatico teste
	if (AgroupList(conn, item.sIndex) == true && item.sIndex != 0) {
		int dropAmount = 1;
		if (item.stEffect[0].cEffect == 61)
			dropAmount = item.stEffect[0].cValue;
		else if (item.stEffect[1].cEffect == 61)
			dropAmount = item.stEffect[1].cValue;
		else if (item.stEffect[2].cEffect == 61)
			dropAmount = item.stEffect[2].cValue;

		SendItemagruparEx(conn, item.sIndex, dropAmount);

		if (LogList(item.sIndex) == true) {
			snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
			MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
		}

		return;
	}

	if (PutItem(conn, &item) && item.sIndex != 0)
	{
		if (LogList(item.sIndex) == true) {
			snprintf(temp, sizeof(temp), "MobName:%s dropou o item: %s:%d %d.%d.%d.%d.%d.%d do mob:%s", pMob[conn].MOB.MobName, g_pItemList[item.sIndex].Name, item.sIndex, item.stEffect[0].cEffect, item.stEffect[0].cValue, item.stEffect[1].cEffect, item.stEffect[1].cValue, item.stEffect[2].cEffect, item.stEffect[2].cValue, pMob[target].MOB.MobName);
			MobDropLog(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, temp);
		}

		return;
	}
}

void BlockAdd(STRUCT_ITEM* item, int Type, int min, int max)
{
	if (min < 0)
		min = 0;

	int soma = 0;
	for (int i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == Type)
			soma += item->stEffect[i].cValue;
	}

	if (soma > max)
	{
		for (int i = 0; i < 3; i++)
		{
			item->stEffect[i].cEffect = 0;
			item->stEffect[i].cValue = 0;
		}

		item->stEffect[0].cEffect = Type;
		item->stEffect[0].cValue = min;
	}
}

//Controle de Drop por Tempo
bool DropItemControl(int sIndex) {

	int Index = sIndex;

	//Necess�rio colocar o id do item que queira efetuar o controle por tempo aqui e na ProcessSecMinTimer.cpp
	int List[] = {
		1740, 1741, 671
	};

	for (int i = 0; i < (sizeof(List) / 4); i++) {
		if (List[i] == Index) {
			if (ItemDropControl[Index] == 1) {
				ItemDropControl[Index] = 0;
				return true;
			}
			else
				return false;
		}
	}
	return true;
}