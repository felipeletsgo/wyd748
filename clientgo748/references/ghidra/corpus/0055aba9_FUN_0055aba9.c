// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055aba9 | Name: FUN_0055aba9


undefined4 FUN_0055aba9(int param_1,int param_2,undefined4 param_3)

{
  byte bVar1;
  byte bVar2;
  undefined4 uVar3;
  char local_1c;
  undefined4 local_1b;
  undefined1 local_14;
  undefined2 local_13;
  undefined1 local_11;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = param_2 % 3;
  local_10 = param_2 - local_8;
  local_14 = DAT_00e3852c;
  local_13 = 0;
  local_11 = 0;
  local_1c = DAT_00e38530;
  local_1b = 0;
  for (local_c = 0; uVar3 = local_1b, local_c < local_10; local_c = local_c + 3) {
    local_14 = *(undefined1 *)(param_1 + local_c);
    local_13 = CONCAT11(*(undefined1 *)(param_1 + local_c + 2),
                        *(undefined1 *)(param_1 + local_c + 1));
    FUN_0055aad5(&local_14,&local_1c);
    FUN_0058ee30(param_3,&local_1c);
  }
  if (local_8 == 1) {
    bVar1 = *(byte *)(param_1 + param_2 + -1);
    local_1c = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[(int)(uint)bVar1 >> 2];
    local_1b._0_2_ = CONCAT11(0x3d,s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[(bVar1 & 3) * 0x10]);
    local_1b._3_1_ = SUB41(uVar3,3);
    local_1b._0_3_ = CONCAT12(0x3d,(undefined2)local_1b);
    FUN_0058ee30(param_3,&local_1c);
  }
  uVar3 = local_1b;
  if (local_8 == 2) {
    bVar1 = *(byte *)(param_1 + param_2 + -2);
    bVar2 = *(byte *)(param_1 + param_2 + -1);
    local_1c = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[(int)(uint)bVar1 >> 2];
    local_1b._0_2_ =
         CONCAT11(s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[(bVar2 & 0xf) * 4],
                  s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914
                  [(byte)((bVar1 & 3) << 4 | (byte)((int)(uint)bVar2 >> 4))]);
    local_1b._3_1_ = SUB41(uVar3,3);
    local_1b._0_3_ = CONCAT12(0x3d,(undefined2)local_1b);
    FUN_0058ee30(param_3,&local_1c);
  }
  return 1;
}

