// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043387d | Name: FUN_0043387d


void __fastcall FUN_0043387d(int param_1)

{
  undefined4 uVar1;
  
  *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x8c) = 1;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x2009c) = uVar1;
  if (DAT_005c4958 == 1) {
    FUN_0049e1e3(s_UI_EndCamAction_005b4cbc);
  }
  else {
    *(undefined4 *)(param_1 + 0x200b0) = 1;
    _memset((void *)(param_1 + 0x200b4),0,28000);
    *(undefined4 *)(param_1 + 0x200b4) = 1;
    *(undefined2 *)(param_1 + 0x200b8) = 0;
    *(undefined4 *)(param_1 + 0x200bc) = 0x4513c000;
    *(undefined4 *)(param_1 + 0x200c0) = 0x40000000;
    *(undefined4 *)(param_1 + 0x200c4) = 0x450dc000;
    *(undefined4 *)(param_1 + 0x200c8) = 0x3fc90fdb;
    *(undefined4 *)(param_1 + 0x200cc) = 0xbdb2b8c3;
  }
  *(undefined2 *)(param_1 + 0x20098) = 1;
  return;
}

