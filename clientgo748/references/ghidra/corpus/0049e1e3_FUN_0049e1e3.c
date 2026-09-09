// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049e1e3 | Name: FUN_0049e1e3


void __thiscall FUN_0049e1e3(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined1 local_84 [128];
  
  *(undefined4 *)(param_1 + 0x200b0) = 0;
  _memset((void *)(param_1 + 0x200b4),0,28000);
  FUN_0058f078(local_84,s__s_bin_005b9218,param_2);
  uVar1 = FUN_0058f716(local_84,&DAT_005b9220);
  FUN_00590d68(param_1 + 0x200b0,1,4,uVar1);
  if (*(int *)(param_1 + 0x200b0) < 0x3e9) {
    if (*(int *)(param_1 + 0x200b0) < 0) {
      *(undefined4 *)(param_1 + 0x200b0) = 0;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x200b0) = 1000;
  }
  FUN_00590d68(param_1 + 0x200b4,1,*(int *)(param_1 + 0x200b0) * 0x1c,uVar1);
  FUN_0058f62c(uVar1);
  return;
}

