#!/usr/bin/env python3
# ember-sigjoin — splice the GOLD source signature (from `ember-gen --sig`) into a
# per-function `--records` JSONL, joined by the function name. Adds `src_sig` to each
# record so ember-vardata can read the real param names as gold. Records with no
# matching signature get src_sig=null (real-code / no-sidecar case). Pass-through of all
# other keys (incl. var-context fields). NO deps.
#
#   ember-sigjoin.py <sig.json> <records.jsonl>   -> joined JSONL on stdout
import sys, json

def main():
    if len(sys.argv) < 3:
        print("usage: ember-sigjoin.py <sig.json> <records.jsonl>", file=sys.stderr); sys.exit(2)
    try: sig = json.load(open(sys.argv[1]))
    except Exception: sig = {}
    n = 0; hit = 0
    for line in open(sys.argv[2]):
        line = line.strip()
        if not line: continue
        try: rec = json.loads(line)
        except Exception: continue
        nm = rec.get("name", "")
        rec["src_sig"] = sig.get(nm)            # join by the full function name (e.g. gcd__int_0)
        n += 1; hit += (rec["src_sig"] is not None)
        sys.stdout.write(json.dumps(rec) + "\n")
    print("ember-sigjoin: %d records, %d with src_sig" % (n, hit), file=sys.stderr)

if __name__ == "__main__":
    main()
