// Search immediate scalar uses and decompile each containing function once.
// Run with Ghidra analyzeHeadless as a post-script, passing hexadecimal values.
//@category WYD Research

import java.util.LinkedHashMap;
import java.util.Map;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.scalar.Scalar;

public class FindScalarUses extends GhidraScript {
    private long parseHex(String value) {
        String normalized = value.trim().replaceFirst("^(?i)0x", "");
        return Long.parseUnsignedLong(normalized, 16);
    }

    private boolean operandContains(Instruction instruction, int operandIndex, long target) {
        for (Object object : instruction.getOpObjects(operandIndex)) {
            if (object instanceof Scalar) {
                Scalar scalar = (Scalar) object;
                if (scalar.getUnsignedValue() == target) {
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length == 0) {
            printerr("Usage: FindScalarUses.java <hex-value> [<hex-value> ...]");
            return;
        }

        println("PROGRAM name=" + currentProgram.getName()
            + " path=" + currentProgram.getExecutablePath()
            + " md5=" + currentProgram.getExecutableMD5()
            + " sha256=" + currentProgram.getExecutableSHA256()
            + " imageBase=" + currentProgram.getImageBase());

        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        for (String arg : args) {
            long target = parseHex(arg);
            Map<String, Function> functions = new LinkedHashMap<>();
            int useCount = 0;

            println("================ SCALAR 0x" + Long.toHexString(target).toUpperCase() + " ================");
            InstructionIterator instructions = currentProgram.getListing().getInstructions(true);
            while (instructions.hasNext() && !monitor.isCancelled()) {
                Instruction instruction = instructions.next();
                boolean matched = false;
                for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                    if (operandContains(instruction, operand, target)) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    continue;
                }

                Function function = getFunctionContaining(instruction.getAddress());
                println("USE " + instruction.getAddress() + "  " + instruction
                    + (function == null ? "  function=<none>"
                        : "  function=" + function.getName() + "@" + function.getEntryPoint()));
                useCount++;
                if (function != null) {
                    functions.put(function.getEntryPoint().toString(), function);
                }
            }

            println("SUMMARY uses=" + useCount + " functions=" + functions.size());
            for (Function function : functions.values()) {
                println("--- DECOMPILE " + function.getName() + " @ " + function.getEntryPoint() + " ---");
                DecompileResults result = decompiler.decompileFunction(function, 120, monitor);
                if (result != null && result.decompileCompleted()) {
                    println(result.getDecompiledFunction().getC());
                }
                else {
                    println("(decompile failed: "
                        + (result == null ? "no result" : result.getErrorMessage()) + ")");
                }
            }
        }

        decompiler.dispose();
    }
}
