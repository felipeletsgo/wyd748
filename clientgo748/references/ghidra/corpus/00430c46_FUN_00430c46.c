// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00430c46 | Name: FUN_00430c46


undefined4 __fastcall FUN_00430c46(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_a8 [12];
  undefined1 local_9c [64];
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [64];
  
  FUN_00430f10();
  iVar1 = *(int *)(param_1 + 0x2a4d8);
  uVar2 = 0;
  FUN_00562979(local_44,0x3dcccccd,(float)*(uint *)(param_1 + 0x2a4d4) / (float)iVar1,0x41200000,
               0x42c80000);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x94))
            (*(undefined4 *)(param_1 + 0x2a3e4),3,local_44,iVar1,uVar2);
  FUN_00430f10();
  FUN_00430f50(0,0x3f800000,0);
  FUN_00430f50(0,0,0x42480000);
  FUN_00430f50(0,0,0);
  FUN_005640d2(local_9c,local_5c,local_50,local_a8);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x94))(*(undefined4 *)(param_1 + 0x2a3e4),2,local_9c)
  ;
  return 1;
}

