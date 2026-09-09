// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e2b3 | Name: FUN_0047e2b3


int __fastcall FUN_0047e2b3(int param_1)

{
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_8 = 0;
  local_c = 0;
  do {
    if (0x3f < local_c) {
LAB_0047e36e:
      *(undefined4 *)(DAT_0092e640 + 8 + local_10 * 0xc) = 0xffffffff;
      if (*(int *)(DAT_0092e640 + 4 + local_10 * 0xc) != 0) {
        (**(code **)(**(int **)(DAT_0092e640 + 4 + local_10 * 0xc) + 8))
                  (*(undefined4 *)(DAT_0092e640 + 4 + local_10 * 0xc));
        *(undefined4 *)(DAT_0092e640 + 4 + local_10 * 0xc) = 0;
      }
      *(undefined4 *)(DAT_0092e640 + 0xc + local_10 * 0xc) = 0;
      *(int *)(param_1 + 0x28e68) = *(int *)(param_1 + 0x28e68) + -1;
      return local_10;
    }
    if (*(int *)(DAT_0092e640 + 8 + local_c * 0xc) != 0) {
      if (*(int *)(DAT_0092e640 + 0xc + local_c * 0xc) == 0) {
        local_10 = local_c;
        goto LAB_0047e36e;
      }
      if (local_8 == 0) {
        local_10 = local_c;
        local_8 = *(uint *)(DAT_0092e640 + 0xc + local_c * 0xc);
      }
      else if (*(uint *)(DAT_0092e640 + 0xc + local_c * 0xc) < local_8) {
        local_10 = local_c;
        local_8 = *(uint *)(DAT_0092e640 + 0xc + local_c * 0xc);
      }
    }
    local_c = local_c + 1;
  } while( true );
}

