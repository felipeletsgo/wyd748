// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042dbe3 | Name: FUN_0042dbe3


bool __thiscall
FUN_0042dbe3(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6,undefined4 param_7)

{
  undefined4 *puVar1;
  int iVar2;
  undefined1 local_10 [12];
  
  FUN_00430f50(0,0x3f800000,0);
  puVar1 = (undefined4 *)FUN_00430f50(param_2,param_3,param_4);
  *(undefined4 *)(param_1 + 0x2a6ac) = *puVar1;
  *(undefined4 *)(param_1 + 0x2a6b0) = puVar1[1];
  *(undefined4 *)(param_1 + 0x2a6b4) = puVar1[2];
  puVar1 = (undefined4 *)FUN_00430f50(param_5,param_6,param_7);
  *(undefined4 *)(param_1 + 0x2a6b8) = *puVar1;
  *(undefined4 *)(param_1 + 0x2a6bc) = puVar1[1];
  *(undefined4 *)(param_1 + 0x2a6c0) = puVar1[2];
  FUN_005640d2(param_1 + 0x2a62c,param_1 + 0x2a6ac,param_1 + 0x2a6b8,local_10);
  iVar2 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x94))
                    (*(undefined4 *)(param_1 + 0x2a3e4),2,param_1 + 0x2a62c);
  return -1 < iVar2;
}

