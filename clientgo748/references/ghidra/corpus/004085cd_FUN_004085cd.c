// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004085cd | Name: FUN_004085cd


void __thiscall
FUN_004085cd(int param_1,int param_2,int param_3,int param_4,undefined4 param_5,undefined4 param_6,
            undefined4 param_7,undefined4 param_8)

{
  undefined1 local_8c [128];
  undefined4 local_c;
  int local_8;
  
  *(int *)(param_1 + 0x9c) = param_4;
  *(int *)(param_1 + 0xa0) = param_2;
  *(int *)(param_1 + 0xa4) = param_3;
  *(undefined4 *)(param_1 + 0xb0) = param_5;
  *(undefined4 *)(param_1 + 0xb4) = param_6;
  *(undefined4 *)(param_1 + 0xa8) = param_7;
  *(undefined4 *)(param_1 + 0xac) = param_8;
  for (local_8 = 0; local_8 < 0xe; local_8 = local_8 + 1) {
    if (local_8 == 0) {
      FUN_0058f078(local_8c,&DAT_005b10dc);
    }
    else if (local_8 == 1) {
      FUN_0058f078(local_8c,&DAT_005b10e0);
    }
    else if (local_8 == 0xc) {
      FUN_0058f078(local_8c,&DAT_005b10e4);
    }
    else if (local_8 == 0xd) {
      FUN_0058f078(local_8c,&DAT_005b10e8);
    }
    else {
      FUN_0058f078(local_8c,&DAT_005b10ec,param_2 + -2 + local_8);
      if (param_3 < param_2 + -2 + local_8) {
        (**(code **)(**(int **)(param_1 + 100 + local_8 * 4) + 0x60))(0);
      }
      else {
        (**(code **)(**(int **)(param_1 + 100 + local_8 * 4) + 0x60))(1);
      }
    }
    FUN_0058f078(*(int *)(param_1 + 100 + local_8 * 4) + 0xaa,local_8c);
    local_c = 0xffffffff;
    if (local_8 + -1 == param_4) {
      local_c = 0xffffff00;
    }
    FUN_00421b8f(*(int *)(param_1 + 100 + local_8 * 4) + 0xaa,local_c,0);
  }
  return;
}

