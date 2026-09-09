// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005794c0 | Name: FUN_005794c0


undefined4 FUN_005794c0(int param_1)

{
  uint *puVar1;
  
  if ((param_1 != 0) && (puVar1 = *(uint **)(param_1 + 0x1c), puVar1 != (uint *)0x0)) {
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(undefined4 *)(param_1 + 8) = 0;
    *(undefined4 *)(param_1 + 0x18) = 0;
    *puVar1 = -(uint)(puVar1[3] != 0) & 7;
    FUN_00589971(*(undefined4 *)(*(int *)(param_1 + 0x1c) + 0x14),param_1,0);
    return 0;
  }
  return 0xfffffffe;
}

