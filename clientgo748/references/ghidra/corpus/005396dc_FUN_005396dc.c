// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005396dc | Name: FUN_005396dc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall FUN_005396dc(int param_1,undefined4 param_2,undefined4 param_3,float *param_4)

{
  float fVar1;
  float10 fVar2;
  undefined4 local_c;
  
  local_c = 0;
  do {
    fVar1 = _DAT_005a44b0;
    if (9 < local_c) {
LAB_00539765:
      return (float10)fVar1;
    }
    if (*(int *)(param_1 + 0x20a34 + local_c * 4) != 0) {
      fVar2 = (float10)FUN_00546dda(param_2,param_3);
      if ((float10)_DAT_005a44b0 < fVar2) {
        *param_4 = (float)fVar2 + *(float *)(*(int *)(param_1 + 0x20a34 + local_c * 4) + 0x30);
        fVar1 = *param_4;
        goto LAB_00539765;
      }
    }
    local_c = local_c + 1;
  } while( true );
}

