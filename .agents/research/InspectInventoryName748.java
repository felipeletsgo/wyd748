import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.program.model.listing.Function;
import java.util.LinkedHashSet;

// Read-only inspection: reuse the analyzed native program without reanalysis.
public class InspectInventoryName748 extends GhidraScript {
    public void run() throws Exception {
        println("sha256=" + currentProgram.getExecutableSHA256());
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        LinkedHashSet<Function> functions = new LinkedHashSet<>();
        for (String argument : getScriptArgs()) {
            var address = toAddr(argument);
            println("target=" + address);
            var function = getFunctionContaining(address);
            if (function != null) functions.add(function);
            for (var reference : getReferencesTo(address)) {
                println("reference=" + reference);
                function = getFunctionContaining(reference.getFromAddress());
                if (function != null) functions.add(function);
            }
        }
        for (var function : functions) {
            println("function=" + function.getEntryPoint());
            var result = decompiler.decompileFunction(function, 60, monitor);
            if (!result.decompileCompleted()) throw new Exception(result.getErrorMessage());
            println(result.getDecompiledFunction().getC());
        }
        decompiler.dispose();
    }
}
