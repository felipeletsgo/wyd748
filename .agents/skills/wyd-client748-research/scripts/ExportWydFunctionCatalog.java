// Exporta o catalogo estrutural completo do programa Ghidra 7.48.
// O relatorio e somente leitura: ele registra o call graph que Ghidra conhece,
// mas nao tenta inferir callbacks, destinos calculados ou semantica de negocio.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

public class ExportWydFunctionCatalog extends GhidraScript {
    private String safeValue(Object value) {
        return value == null ? "" : value.toString()
                .replace("\t", " ").replace("\r", " ").replace("\n", " ");
    }

    private String address(Address address) {
        return address == null ? "" : address.toString().toUpperCase();
    }

    private String functionLabel(Function function) {
        return address(function.getEntryPoint()) + ":" + safeValue(function.getName());
    }

    private String functionList(Set<Function> functions) {
        List<String> labels = new ArrayList<>();
        for (Function function : functions) {
            labels.add(functionLabel(function));
        }
        Collections.sort(labels);
        return String.join(";", labels);
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) {
            throw new IllegalArgumentException("uso: <arquivo-saida.tsv>");
        }

        File output = new File(args[0]);
        FunctionManager functions = currentProgram.getFunctionManager();
        ReferenceManager references = currentProgram.getReferenceManager();
        Memory memory = currentProgram.getMemory();
        List<Function> allFunctions = new ArrayList<>();
        FunctionIterator iterator = functions.getFunctions(true);
        while (iterator.hasNext() && !monitor.isCancelled()) {
            allFunctions.add(iterator.next());
        }
        allFunctions.sort(Comparator.comparing(function ->
                address(function.getEntryPoint())));

        output.getParentFile().mkdirs();
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(output), StandardCharsets.UTF_8))) {
            writer.println("# program_sha256=" + currentProgram.getExecutableSHA256());
            writer.println("# image_base=" + currentProgram.getImageBase());
            writer.println("entry\tsymbol\tcalling_convention\tsignature\tbody_min\tbody_max"
                    + "\tbody_address_count\tis_thunk\tthunk_target\tdirect_callers_count"
                    + "\tdirect_callers\tdirect_callees_count\tdirect_callees"
                    + "\tincoming_ref_count\tincoming_flow_refs\tincoming_data_refs"
                    + "\tincoming_other_refs");

            int exported = 0;
            for (Function function : allFunctions) {
                if (monitor.isCancelled()) {
                    break;
                }
                Set<Function> callers = function.getCallingFunctions(monitor);
                Set<Function> callees = function.getCalledFunctions(monitor);
                ReferenceIterator incoming = references.getReferencesTo(function.getEntryPoint());
                int incomingCount = 0;
                int incomingFlow = 0;
                int incomingData = 0;
                int incomingOther = 0;
                while (incoming.hasNext()) {
                    Reference reference = incoming.next();
                    incomingCount++;
                    if (reference.getReferenceType().isFlow()) {
                        incomingFlow++;
                    }
                    else if (reference.getReferenceType().isData()) {
                        incomingData++;
                    }
                    else {
                        incomingOther++;
                    }
                }

                Function thunked = function.isThunk()
                        ? function.getThunkedFunction(true) : null;
                writer.println(address(function.getEntryPoint()) + "\t"
                        + safeValue(function.getName()) + "\t"
                        + safeValue(function.getCallingConventionName()) + "\t"
                        + safeValue(function.getSignature()) + "\t"
                        + address(function.getBody().getMinAddress()) + "\t"
                        + address(function.getBody().getMaxAddress()) + "\t"
                        + function.getBody().getNumAddresses() + "\t"
                        + function.isThunk() + "\t"
                        + (thunked == null ? "" : address(thunked.getEntryPoint())) + "\t"
                        + callers.size() + "\t" + functionList(callers) + "\t"
                        + callees.size() + "\t" + functionList(callees) + "\t"
                        + incomingCount + "\t" + incomingFlow + "\t" + incomingData + "\t"
                        + incomingOther);
                exported++;
            }
            println("Funcoes catalogadas: " + exported + "/" + allFunctions.size()
                    + " em " + output.getAbsolutePath());
        }
    }
}
