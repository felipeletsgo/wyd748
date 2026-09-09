// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042cdbe | Name: FUN_0042cdbe


int __fastcall FUN_0042cdbe(int param_1)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_44 [64];
  
  iVar2 = *(int *)(param_1 + 0x2a508) - *(int *)(param_1 + 0x2a5e8);
  uVar3 = 0;
  fVar1 = (float)(uint)(*(int *)(param_1 + 0x2a504) - *(int *)(param_1 + 0x2a5e4)) / (float)iVar2;
  if (DAT_005b892c == 2) {
    FUN_00562979(param_1 + 0x2a5ec,0x3f61307a,fVar1,0x3dcccccd,DAT_005b48e8);
  }
  else {
    FUN_00562979(param_1 + 0x2a5ec,0x3f34f4ab,fVar1,0x3dcccccd,DAT_005b48e8);
  }
  iVar2 = (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x94))
                    (*(undefined4 *)(param_1 + 0x2a3e4),3,param_1 + 0x2a5ec,iVar2,uVar3);
  FUN_00430f10();
  thunk_FUN_00561ca4(local_44,param_1 + 0x2a5ec);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x13c))
            (*(undefined4 *)(param_1 + 0x2a3e4),2,local_44,4);
  if (-1 < iVar2) {
    iVar2 = 0;
  }
  return iVar2;
}

