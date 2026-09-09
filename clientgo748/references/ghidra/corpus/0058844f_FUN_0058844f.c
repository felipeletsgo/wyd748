// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058844f | Name: FUN_0058844f


void FUN_0058844f(int param_1)

{
  int iVar1;
  int *in_EAX;
  int iVar2;
  int iVar3;
  int unaff_ESI;
  int iVar4;
  uint uVar5;
  int local_38;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  ushort *local_10;
  int local_c;
  ushort *local_8;
  
  local_1c = 0;
  local_20 = 0;
  local_24 = 0;
  local_28 = 0;
  iVar3 = in_EAX[2];
  iVar1 = in_EAX[4];
  iVar2 = *in_EAX;
  if (iVar2 <= in_EAX[1]) {
    local_18 = iVar2 * 8 + 4;
    do {
      if (iVar3 <= in_EAX[3]) {
        local_14 = iVar3 * 4 + 2;
        local_10 = (ushort *)
                   (*(int *)(*(int *)(*(int *)(unaff_ESI + 0x1a4) + 0x18) + iVar2 * 4) +
                   (iVar3 * 0x20 + iVar1) * 2);
        iVar4 = (in_EAX[3] - iVar3) + 1;
        do {
          local_8 = local_10;
          if (iVar1 <= in_EAX[5]) {
            local_c = iVar1 * 8 + 4;
            local_38 = (in_EAX[5] - iVar1) + 1;
            do {
              uVar5 = (uint)*local_8;
              local_8 = local_8 + 1;
              if (uVar5 != 0) {
                local_1c = local_1c + uVar5;
                local_24 = local_24 + local_14 * uVar5;
                local_20 = local_20 + local_18 * uVar5;
                local_28 = local_28 + local_c * uVar5;
              }
              local_c = local_c + 8;
              local_38 = local_38 + -1;
            } while (local_38 != 0);
          }
          local_10 = local_10 + 0x20;
          local_14 = local_14 + 4;
          iVar4 = iVar4 + -1;
        } while (iVar4 != 0);
      }
      local_18 = local_18 + 8;
      iVar2 = iVar2 + 1;
    } while (iVar2 <= in_EAX[1]);
  }
  iVar3 = local_1c >> 1;
  *(char *)(param_1 + **(int **)(unaff_ESI + 0x74)) = (char)((local_20 + iVar3) / local_1c);
  *(char *)(param_1 + *(int *)(*(int *)(unaff_ESI + 0x74) + 4)) =
       (char)((local_24 + iVar3) / local_1c);
  *(char *)(param_1 + *(int *)(*(int *)(unaff_ESI + 0x74) + 8)) =
       (char)((local_28 + iVar3) / local_1c);
  return;
}

