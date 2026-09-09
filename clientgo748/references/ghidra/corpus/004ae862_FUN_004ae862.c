// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae862 | Name: FUN_004ae862


undefined4 __thiscall FUN_004ae862(int param_1,undefined4 param_2)

{
  int iVar1;
  HMODULE pHVar2;
  undefined4 uVar3;
  undefined *puVar4;
  undefined4 uVar5;
  
  uVar5 = 0;
  iVar1 = param_1 + 4;
  puVar4 = &DAT_005a9efc;
  uVar3 = 0x800;
  pHVar2 = GetModuleHandleA((LPCSTR)0x0);
  iVar1 = DirectInput8Create(pHVar2,uVar3,puVar4,iVar1,uVar5);
  if (iVar1 < 0) {
    uVar3 = 0;
  }
  else {
    iVar1 = (**(code **)(**(int **)(param_1 + 4) + 0xc))
                      (*(undefined4 *)(param_1 + 4),&DAT_005a9ebc,param_1 + 8,0);
    if (iVar1 < 0) {
      uVar3 = 0;
    }
    else {
      iVar1 = (**(code **)(**(int **)(param_1 + 8) + 0x2c))
                        (*(undefined4 *)(param_1 + 8),&DAT_005a9e74);
      if (iVar1 < 0) {
        uVar3 = 0;
      }
      else {
        iVar1 = (**(code **)(**(int **)(param_1 + 8) + 0x34))
                          (*(undefined4 *)(param_1 + 8),param_2,6);
        if (iVar1 < 0) {
          uVar3 = 0;
        }
        else {
          (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(undefined4 *)(param_1 + 8));
          *(undefined4 *)(param_1 + 0x448) = 1;
          uVar3 = 1;
        }
      }
    }
  }
  return uVar3;
}

