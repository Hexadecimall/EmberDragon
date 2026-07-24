#!/usr/bin/env python3
# ember-mind — EmberDragon's PyTorch domain brain.
#
# A decoder-only transformer trained on the ember decompile corpus: condition on
# `input` (decompiled placeholder C: Record0 / sub_130 / v24 …) and generate
# `target` (clean, named, commented C). Domain-focused on purpose — that's the
# fix for Kyte-1a's failure (it trained on generic English and stayed dumb).
# MPS (Metal) accelerated on Apple Silicon. This is the local brain that plugs
# into ember-agent's `--brain local` seam.
#
#   .venv_torch/bin/python ember-mind.py --train --steps 400
#   .venv_torch/bin/python ember-mind.py --infer "class Record0 { ... }"
#
# Run with the project's torch venv:  ~/pullio/decompile/.venv_torch/bin/python
import json, math, os, sys, time, argparse
import torch, torch.nn as nn, torch.nn.functional as F

HERE   = os.path.dirname(os.path.abspath(__file__))
CORPUS = os.environ.get("EMBER_CORPUS", os.path.join(HERE, "corpus", "corpus.jsonl"))
CKPT   = os.path.join(HERE, ".ember-mind.pt")
DEV    = "mps" if torch.backends.mps.is_available() else ("cuda" if torch.cuda.is_available() else "cpu")

# byte-level tokenizer + 4 special tokens
PAD, BOS, SEP, EOS = 256, 257, 258, 259
VOCAB = 260
def encode_pair(inp, tgt, ctx):
    # Budget the window so BOTH input and target survive truncation — otherwise a
    # long input eats the whole context, the target region vanishes, every label
    # is "ignore", and cross-entropy returns NaN. Reserve ~60% for input.
    ib = list(inp.encode("utf-8", "replace"))[: (ctx * 6) // 10]
    src = [BOS] + ib + [SEP]
    room = max(1, ctx - len(src) - 1)               # leave space for >=1 target tok + EOS
    tb = list(tgt.encode("utf-8", "replace"))[:room]
    seq = src + tb + [EOS]
    return seq, len(src)            # (token ids, index where target begins)
def decode(ids):
    return bytes(b for b in ids if b < 256).decode("utf-8", "replace")

class Block(nn.Module):
    def __init__(self, d, h):
        super().__init__()
        self.h = h
        self.ln1 = nn.LayerNorm(d); self.qkv = nn.Linear(d, 3*d); self.proj = nn.Linear(d, d)
        self.ln2 = nn.LayerNorm(d)
        self.mlp = nn.Sequential(nn.Linear(d, 4*d), nn.GELU(), nn.Linear(4*d, d))
    def forward(self, x):
        B, T, D = x.shape
        qkv = self.qkv(self.ln1(x)).view(B, T, 3, self.h, D // self.h).permute(2, 0, 3, 1, 4)
        q, k, v = qkv[0], qkv[1], qkv[2]
        a = F.scaled_dot_product_attention(q, k, v, is_causal=True)   # MPS-friendly causal attn
        a = a.transpose(1, 2).reshape(B, T, D)
        x = x + self.proj(a)
        x = x + self.mlp(self.ln2(x))
        return x

class Mind(nn.Module):
    def __init__(self, vocab=VOCAB, d=256, layers=4, heads=8, ctx=512):
        super().__init__()
        self.ctx = ctx
        self.tok = nn.Embedding(vocab, d); self.pos = nn.Embedding(ctx, d)
        self.blocks = nn.ModuleList([Block(d, heads) for _ in range(layers)])
        self.lnf = nn.LayerNorm(d); self.head = nn.Linear(d, vocab, bias=False)
    def forward(self, idx):
        T = idx.size(1)
        x = self.tok(idx) + self.pos(torch.arange(T, device=idx.device))[None]
        for b in self.blocks: x = b(x)
        return self.head(self.lnf(x))
    def n_params(self):
        return sum(p.numel() for p in self.parameters())

def load_corpus(ctx):
    rows = []
    for line in open(CORPUS):
        line = line.strip()
        if not line: continue
        r = json.loads(line)
        if "input" in r and "target" in r:
            seq, tstart = encode_pair(r["input"], r["target"], ctx)
            rows.append((seq, tstart))
    return rows

def make_batch(rows, B, ctx, device):
    import random
    pick = [rows[random.randrange(len(rows))] for _ in range(B)]
    L = min(ctx, max(len(s) for s, _ in pick))
    X = torch.full((B, L), PAD, dtype=torch.long)
    Y = torch.full((B, L), -100, dtype=torch.long)         # -100 = ignore in CE
    for i, (seq, tstart) in enumerate(pick):
        seq = seq[:L]
        X[i, :len(seq)] = torch.tensor(seq)
        # next-token labels, but only SCORE the target region (teach input→target)
        for t in range(len(seq) - 1):
            if t + 1 >= tstart:                            # position predicts a target token
                Y[i, t] = seq[t + 1]
    return X.to(device), Y.to(device)

def evaluate(model, rows, ctx, device, iters=8, B=6):
    model.eval(); tot = 0.0
    with torch.no_grad():
        for _ in range(iters):
            X, Y = make_batch(rows, B, ctx, device)
            logits = model(X)
            tot += F.cross_entropy(logits.view(-1, VOCAB), Y.view(-1), ignore_index=-100).item()
    model.train()
    loss = tot / iters
    return loss, math.exp(min(loss, 20))

def train(steps, B, ctx, lr, d=256, layers=4, heads=8):
    rows = load_corpus(ctx)
    n_val = max(8, len(rows) // 10); train_rows, val_rows = rows[:-n_val], rows[-n_val:]
    model = Mind(d=d, layers=layers, heads=heads, ctx=ctx).to(DEV)
    opt = torch.optim.AdamW(model.parameters(), lr=lr, weight_decay=0.01)
    print("ember-mind: device=%s  params=%.2fM  corpus=%d (train %d / val %d)  ctx=%d" % (
        DEV, model.n_params() / 1e6, len(rows), len(train_rows), len(val_rows), ctx))
    t0 = time.time()
    for step in range(1, steps + 1):
        X, Y = make_batch(train_rows, B, ctx, DEV)
        if (Y != -100).sum() == 0:        # defensive: never backprop an all-ignore batch
            continue
        logits = model(X)
        loss = F.cross_entropy(logits.view(-1, VOCAB), Y.view(-1), ignore_index=-100)
        opt.zero_grad(); loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0); opt.step()
        if step == 1 or step % max(1, steps // 10) == 0 or step == steps:
            vl, vppl = evaluate(model, val_rows, ctx, DEV)
            print("  step %4d/%d  train_loss %.3f  val_loss %.3f  val_ppl %.1f  (%.1fs)" % (
                step, steps, loss.item(), vl, vppl, time.time() - t0))
    torch.save({"model": model.state_dict(), "cfg": {"ctx": ctx, "d": d, "layers": layers, "heads": heads}}, CKPT)
    print("saved -> %s" % CKPT)

@torch.no_grad()
def infer(prompt, max_new=400, temp=0.2, topk=40):
    if not os.path.exists(CKPT):
        print("no checkpoint yet — train first (--train)"); sys.exit(1)
    st = torch.load(CKPT, map_location=DEV); c = st["cfg"]; ctx = c["ctx"]
    model = Mind(d=c.get("d", 256), layers=c.get("layers", 4), heads=c.get("heads", 8), ctx=ctx).to(DEV)
    model.load_state_dict(st["model"]); model.eval()
    ids = [BOS] + list(prompt.encode("utf-8", "replace")) + [SEP]
    for _ in range(max_new):
        x = torch.tensor(ids[-ctx:], device=DEV)[None]
        logits = model(x)[0, -1]
        if temp <= 0:                                  # greedy — best for confident/structured code
            nxt = int(torch.argmax(logits))
        else:
            logits = logits / temp
            v, ixs = torch.topk(logits, min(topk, logits.size(-1)))
            nxt = int(ixs[torch.multinomial(F.softmax(v, dim=-1), 1)])
        if nxt == EOS: break
        ids.append(nxt)
    sep = ids.index(SEP)
    print(decode(ids[sep + 1:]))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--train", action="store_true")
    ap.add_argument("--infer", type=str, default=None)
    ap.add_argument("--steps", type=int, default=400)
    ap.add_argument("--batch", type=int, default=4)
    ap.add_argument("--ctx", type=int, default=1024)
    ap.add_argument("--lr", type=float, default=3e-4)
    ap.add_argument("--d-model", type=int, default=256, dest="d")
    ap.add_argument("--layers", type=int, default=4)
    ap.add_argument("--heads", type=int, default=8)
    ap.add_argument("--temp", type=float, default=0.2)   # <=0 = greedy
    a = ap.parse_args()
    if a.infer is not None: infer(a.infer, temp=a.temp)
    elif a.train:          train(a.steps, a.batch, a.ctx, a.lr, a.d, a.layers, a.heads)
    else:                  ap.print_help()

if __name__ == "__main__":
    main()
