// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553ab1 | Name: FUN_00553ab1


undefined4 FUN_00553ab1(int param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  undefined4 uVar1;
  undefined4 local_98;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  uint local_84;
  short local_80 [2];
  undefined4 auStack_7c [29];
  int local_8;
  
  FUN_0058f220(local_80,param_4,0x78);
  FUN_0058f220(param_1,param_2,0x200);
  for (local_8 = 0; local_8 < 0xf; local_8 = local_8 + 1) {
    local_84 = (uint)*(byte *)(param_3 + local_8);
    if (local_84 != 0xffffffff) {
      FUN_00552fdf(param_1 + local_84 * 8);
    }
  }
  FUN_00553930(local_80,0x21);
  local_8 = 0;
  do {
    if (0xe < local_8) {
      return 1;
    }
    if (local_80[local_8 * 4] != 0) {
      for (local_88 = 0; local_88 < 0x40; local_88 = local_88 + 1) {
        local_90 = local_88 % 9;
        local_94 = local_88 / 9;
        local_98 = 0xfffffffe;
        local_8c = FUN_00553464(local_80 + local_8 * 4,param_1,local_90,local_94,&local_98);
        if (local_8c == 1) {
          uVar1 = auStack_7c[local_8 * 2];
          *(undefined4 *)(param_1 + local_88 * 8) = *(undefined4 *)(local_80 + local_8 * 4);
          *(undefined4 *)(param_1 + 4 + local_88 * 8) = uVar1;
          break;
        }
      }
      if (local_88 == 0x40) {
        return 0;
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}

