// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b16c0 | Name: FUN_004b16c0


void __fastcall FUN_004b16c0(int param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *local_c;
  
  puVar2 = DAT_0067cf38;
  local_c = DAT_0067cf38;
  if (DAT_0067cf38 != (undefined4 *)0x0) {
    do {
      puVar1 = local_c;
      if (local_c != (undefined4 *)0x0) {
        puVar1 = (undefined4 *)local_c[1];
        if (*(char *)(local_c + 5) == '\0') {
          if (local_c[4] == 0) {
            while (local_c[3] == 0) {
              local_c = (undefined4 *)local_c[1];
              puVar1 = local_c;
              if ((local_c == puVar2) || (local_c == (undefined4 *)0x0)) goto LAB_004b1771;
            }
            puVar1 = (undefined4 *)local_c[3];
          }
          else {
            puVar1 = (undefined4 *)local_c[4];
          }
        }
        else if (local_c != (undefined4 *)0x0) {
          (**(code **)*local_c)(1);
        }
      }
LAB_004b1771:
      local_c = puVar1;
    } while ((local_c != puVar2) && (local_c != (undefined4 *)0x0));
    if (*(int *)(param_1 + 0x1b088) != 0) {
      if (*(undefined4 **)(param_1 + 0x1b088) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x1b088))(1);
      }
      *(undefined4 *)(param_1 + 0x1b088) = 0;
    }
    *(undefined4 *)(param_1 + 0x1b08c) = 0;
  }
  return;
}

