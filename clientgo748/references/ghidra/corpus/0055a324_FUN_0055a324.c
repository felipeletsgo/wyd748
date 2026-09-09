// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a324 | Name: FUN_0055a324


void FUN_0055a324(undefined4 param_1,undefined4 param_2,int param_3,char param_4)

{
  int local_c;
  int local_8;
  
  FUN_0055a709(&DAT_00e37dd0,param_2,0x40);
  FUN_0055a626(&DAT_00e37dd0,&DAT_00e37dd0,&DAT_005a5dbc,0x40);
  if (param_4 == '\0') {
    for (local_8 = 0; local_8 < 0x10; local_8 = local_8 + 1) {
      FUN_0058f220(&DAT_00e37db0,PTR_DAT_005c4908,0x20);
      FUN_0055a529(PTR_DAT_005c4908,param_3 + local_8 * 0x30);
      FUN_0055a678(PTR_DAT_005c4908,PTR_DAT_005c4904,0x20);
      FUN_0058f220(PTR_DAT_005c4904,&DAT_00e37db0,0x20);
    }
  }
  else {
    for (local_c = 0xf; -1 < local_c; local_c = local_c + -1) {
      FUN_0058f220(&DAT_00e37db0,PTR_DAT_005c4904,0x20);
      FUN_0055a529(PTR_DAT_005c4904,param_3 + local_c * 0x30);
      FUN_0055a678(PTR_DAT_005c4904,PTR_DAT_005c4908,0x20);
      FUN_0058f220(PTR_DAT_005c4908,&DAT_00e37db0,0x20);
    }
  }
  FUN_0055a626(&DAT_00e37dd0,&DAT_00e37dd0,&DAT_005a5dfc,0x40);
  FUN_0055a74d(param_1,&DAT_00e37dd0,0x40);
  return;
}

