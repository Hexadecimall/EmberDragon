// ember-apple — bridge to Apple Intelligence's ON-DEVICE foundation model.
//
// EmberDragon's local brain: smart, private, free, no download (ships with macOS),
// runs on the Neural Engine. Reads a user prompt on stdin, prints the model's reply.
// The Python ember-agent calls this for its `--brain local` path; Claude stays the
// premium brain for serious RE.
//
//   echo "explain this decompiled fn: ..." | ember-apple
//   echo "..." | ember-apple --system "custom instructions"
import FoundationModels
import Foundation

@main
struct EmberApple {
    static func main() async {
        var system = """
        You are EmberDragon's on-device reverse-engineering assistant. You read \
        decompiled C/C++ (placeholder names like sub_130, v24, Record0, arg1) and: \
        explain what the code does, propose meaningful names for functions/variables/\
        fields, and recognize well-known algorithms (gcd, hashing, sorts, linked-list \
        ops, etc.). Be concise and technical. Output code or short explanations, no fluff.
        """
        let args = CommandLine.arguments
        // --rewrite: the BACKEND PIPELINE pass — rewrite decompiled code to clean named
        // source, output ONLY code (no prose). This is what `emberdragon --ai local` calls
        // so every decompile passes through the on-device model.
        if args.contains("--rewrite") {
            system = """
            You rewrite decompiled C/C++ into clean, idiomatic, NAMED source. The input has \
            placeholder names (sub_<hex>, v8/v16, arg0, Record0, fields f0/f8). Rename every \
            function, variable, parameter, field, and struct to a meaningful name inferred from \
            behavior, control flow, call patterns, and constants. Recognize well-known algorithms \
            and collapse decompiler artifacts back to idiom (e.g. a-(a/b)*b becomes a%b). PRESERVE \
            structure and semantics EXACTLY — rename and lightly comment only, never change logic. \
            Output ONLY the rewritten code — no prose, no explanation, no markdown fences.
            """
        }
        if let i = args.firstIndex(of: "--system"), i + 1 < args.count { system = args[i + 1] }

        // availability guard — gives a clean message if Apple Intelligence is off
        switch SystemLanguageModel.default.availability {
        case .available: break
        case .unavailable(let reason):
            FileHandle.standardError.write(Data("ember-apple: on-device model unavailable: \(reason)\n".utf8))
            exit(2)
        }

        let prompt = String(data: FileHandle.standardInput.readDataToEndOfFile(), encoding: .utf8) ?? ""
        if prompt.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty {
            FileHandle.standardError.write(Data("ember-apple: empty prompt on stdin\n".utf8)); exit(1)
        }

        let session = LanguageModelSession(instructions: system)
        do {
            let response = try await session.respond(to: prompt)
            var content = response.content
            if args.contains("--rewrite") {                       // strip stray ``` fences -> compilable code
                content = content.components(separatedBy: "\n")
                    .filter { !$0.trimmingCharacters(in: .whitespaces).hasPrefix("```") }
                    .joined(separator: "\n")
            }
            print(content)
        } catch {
            FileHandle.standardError.write(Data("ember-apple: \(error)\n".utf8))
            exit(1)
        }
    }
}
