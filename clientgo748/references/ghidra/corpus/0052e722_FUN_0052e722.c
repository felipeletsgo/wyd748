// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e722 | Name: FUN_0052e722


undefined4 FUN_0052e722(int param_1)

{
  uint uVar1;
  undefined4 extraout_var;
  char *local_a0 [4];
  int local_90;
  int local_8c;
  int local_88;
  undefined1 local_84 [128];
  
  local_90 = DAT_0067cf38;
  local_88 = param_1;
  _memset(local_84,0,0x80);
  local_8c = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(local_88 + 0xc),extraout_var);
  if (local_8c != 0) {
    local_a0[0] = s_1___1_005bb8c4;
    local_a0[1] = s_5___5_005bb8cc;
    local_a0[2] = s_10___10_005bb8d4;
    local_a0[3] = s_All___All_005bb8dc;
    uVar1 = *(uint *)(local_88 + 0x10) & 0x80000003;
    if ((int)uVar1 < 0) {
      uVar1 = (uVar1 - 1 | 0xfffffffc) + 1;
    }
    FUN_0058f078(local_84,&DAT_00a38df8,local_8c + 0x168,local_a0[uVar1]);
    (**(code **)(**(int **)(local_90 + 0x8c) + 0x8c))(local_84,0x39f,&DAT_00a38e78);
    (**(code **)(**(int **)(local_90 + 0x8c) + 0x60))(1);
    *(undefined4 *)(*(int *)(local_90 + 0x8c) + 0x1e8) = *(undefined4 *)(local_88 + 0xc);
  }
  return 1;
}

