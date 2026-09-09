// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004baddf | Name: FUN_004baddf


int __fastcall FUN_004baddf(int param_1)

{
  DWORD DVar1;
  uint uVar2;
  int local_14;
  uint local_10;
  int local_c;
  uint local_8;
  
  DVar1 = timeGetTime();
  local_10 = DVar1 - *(int *)(param_1 + 0x58);
  DAT_0092e658 = *(int *)(param_1 + 0x54) + local_10;
  local_8 = local_10 / 60000;
  local_c = DAT_0092e658;
  if (*(uint *)(param_1 + 0x68) != local_8) {
    FUN_005913b8(&local_14);
    uVar2 = local_14 - *(int *)(param_1 + 100);
    if ((uVar2 != local_10 / 1000) && (*(int *)(param_1 + 0x68) != 0)) {
      FUN_00431159(s_Wrong_Match_Time_Clock___l____l__005ba9e4,uVar2,local_10 / 1000);
    }
    *(uint *)(param_1 + 0x68) = local_8;
  }
  return local_c + *(int *)(param_1 + 0x60);
}

