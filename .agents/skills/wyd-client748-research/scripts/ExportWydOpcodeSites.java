// Decompiles every function of the WYD 7.48 program (read-only) and records
// where each known opcode appears: as a switch case or comparison (a receive
// or dispatch candidate) or as an assignment (a packet-construction
// candidate), and whether the function calls the native send routine.
// Optionally saves the decompiled corpus (one .c per function plus
// functions.tsv) for later text search. The census is evidence to review,
// not a contract by itself.
//
// Arguments: <sites.tsv> <opcodes.txt> <send-entry> [<corpus-dir>]
//   opcodes.txt: one hexadecimal opcode per line (for example 0x36b).
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.Set;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ExportWydOpcodeSites extends GhidraScript {
    private static final int DECOMPILE_TIMEOUT_SECONDS = 60;
    private static final Pattern HEX = Pattern.compile("0x([0-9a-f]+)\\b");

    private String clean(String line) {
        return line.trim().replace("\t", " ").replace("\r", " ");
    }

    // Classify one hexadecimal literal by the code around it on its line.
    private String classify(String line, int start, int end) {
        String before = line.substring(0, start).trim();
        String after = line.substring(end).trim();
        if (before.endsWith("case")) {
            return "case";
        }
        if (before.endsWith("==") || before.endsWith("!=") || before.endsWith("<")
                || before.endsWith(">") || before.endsWith("<=") || before.endsWith(">=")) {
            return "compare";
        }
        if (before.endsWith("=") && after.startsWith(";")) {
            return "assign";
        }
        if (before.endsWith("(") || before.endsWith(",")) {
            return "argument";
        }
        return "other";
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3) {
            throw new IllegalArgumentException("usage: <sites.tsv> <opcodes.txt> <send-entry> [<corpus-dir>]");
        }
        File output = new File(args[0]);
        Set<Long> opcodes = new TreeSet<>();
        for (String raw : Files.readAllLines(new File(args[1]).toPath(), StandardCharsets.UTF_8)) {
            String value = raw.trim().toLowerCase();
            if (!value.isEmpty()) {
                opcodes.add(Long.parseLong(value.replace("0x", ""), 16));
            }
        }
        Address sendEntry = toAddr(args[2]);
        File corpus = args.length > 3 ? new File(args[3]) : null;
        if (corpus != null) {
            corpus.mkdirs();
        }

        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        int decompiled = 0;
        int failed = 0;
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(output), StandardCharsets.UTF_8));
             PrintWriter catalog = corpus == null ? null : new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(new File(corpus, "functions.tsv")), StandardCharsets.UTF_8))) {
            writer.println("entry\tname\topcode\tkind\tcalls_send\tline");
            writer.println("#program\t" + currentProgram.getExecutableSHA256() + "\t\t\t\t");
            if (catalog != null) {
                catalog.println("entry\tname\tsize\tcalls_send");
            }
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext() && !monitor.isCancelled()) {
                Function function = functions.next();
                boolean callsSend = false;
                for (Function callee : function.getCalledFunctions(monitor)) {
                    if (callee.getEntryPoint().equals(sendEntry)) {
                        callsSend = true;
                        break;
                    }
                }
                DecompileResults results = decompiler.decompileFunction(function, DECOMPILE_TIMEOUT_SECONDS, monitor);
                if (results == null || !results.decompileCompleted()) {
                    failed++;
                    writer.println(function.getEntryPoint() + "\t" + function.getName() + "\t\tdecompile_failed\t" + callsSend + "\t");
                    continue;
                }
                decompiled++;
                String code = results.getDecompiledFunction().getC();
                if (corpus != null) {
                    // Generated names such as CSimpleArray<> are not valid file names.
                    String fileName = function.getName().matches("[A-Za-z0-9_]+")
                            ? function.getName()
                            : function.getName().replaceAll("[^A-Za-z0-9_]", "_") + "@" + function.getEntryPoint();
                    try (PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(
                            new File(corpus, fileName + ".c")), StandardCharsets.UTF_8))) {
                        out.println("// Native SHA-256: " + currentProgram.getExecutableSHA256());
                        out.print(code);
                    }
                    catalog.println(function.getEntryPoint() + "\t" + function.getName() + "\t"
                            + function.getBody().getNumAddresses() + "\t" + callsSend);
                }
                for (String line : code.split("\n")) {
                    Matcher matcher = HEX.matcher(line);
                    while (matcher.find()) {
                        if (matcher.group(1).length() > 8) {
                            continue;                     // 64-bit constants are never opcodes
                        }
                        long value = Long.parseLong(matcher.group(1), 16);
                        if (!opcodes.contains(value)) {
                            continue;
                        }
                        writer.println(function.getEntryPoint() + "\t" + function.getName() + "\t0x"
                                + Long.toHexString(value) + "\t" + classify(line, matcher.start(), matcher.end())
                                + "\t" + callsSend + "\t" + clean(line));
                    }
                }
            }
        } finally {
            decompiler.dispose();
        }
        println("decompiled=" + decompiled + " failed=" + failed);
    }
}
