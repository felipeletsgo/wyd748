// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567d5f | Name: FUN_00567d5f


void __fastcall FUN_00567d5f(int *param_1)

{
  int iVar1;
  
  iVar1 = *param_1;
  (*(code *)PTR_FUN_005c57c4)
            (*(undefined4 *)(param_1[1] + 0x18),*(undefined4 *)(iVar1 + 0x18),
             *(undefined4 *)(iVar1 + 0x1058),*(undefined4 *)(iVar1 + 0x105c),
             *(undefined4 *)(iVar1 + 0x1050),*(undefined4 *)(param_1[1] + 0x1050));
  return;
}

