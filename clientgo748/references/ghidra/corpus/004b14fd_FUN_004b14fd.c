// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b14fd | Name: FUN_004b14fd


undefined4 __thiscall FUN_004b14fd(int param_1,int param_2,int param_3,undefined4 param_4)

{
  undefined4 uVar1;
  DWORD DVar2;
  
  if (param_2 < 0xbe8) {
    if (param_2 < 0) {
      uVar1 = 0;
    }
    else {
      if (*(int *)(param_1 + 0x48 + param_2 * 0x58) == 0) {
        FUN_004b140a(param_2,param_3);
      }
      if (*(int *)(param_1 + 0x48 + param_2 * 0x58) == 0) {
        uVar1 = 0;
      }
      else {
        if (*(int *)(*(int *)(param_1 + 0x48 + param_2 * 0x58) + 4) != param_3) {
          *(int *)(*(int *)(param_1 + 0x48 + param_2 * 0x58) + 4) = param_3;
        }
        DVar2 = timeGetTime();
        *(DWORD *)(*(int *)(param_1 + 0x48 + param_2 * 0x58) + 800) = DVar2;
        *(undefined4 *)(*(int *)(param_1 + 0x48 + param_2 * 0x58) + 0x324) = param_4;
        uVar1 = *(undefined4 *)(param_1 + 0x48 + param_2 * 0x58);
      }
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

