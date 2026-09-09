// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573ceb | Name: FUN_00573ceb


undefined4 FUN_00573ceb(int param_1,int param_2,uint *param_3)

{
  if ((((param_1 != 0) && (param_2 != 0)) && ((*(uint *)(param_2 + 8) & 0x800) != 0)) &&
     (param_3 != (uint *)0x0)) {
    *param_3 = (uint)*(byte *)(param_2 + 0x2c);
    return 0x800;
  }
  return 0;
}

