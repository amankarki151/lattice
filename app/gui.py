"""A minimal web UI for the document assistant.

Deliberately plain: a text box, an answer, and the sources it came
from. The interesting engineering is underneath - this exists so the
thing is demo-able without a terminal, not to be impressive on its
own.

Uses Python's built-in http.server rather than a framework. The
FastAPI server in server/ wraps the database; this wraps the app, and
it doesn't need to be more than one file.
"""

import json
import os
import sys
import urllib.parse
from http.server import BaseHTTPRequestHandler, HTTPServer

from store import DocumentStore

DB_DIR = "data/db"
PORT = 8080

PAGE = """<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Lattice</title>
  <style>
    body {
      font-family: -apple-system, system-ui, sans-serif;
      max-width: 720px; margin: 60px auto; padding: 0 20px;
      color: #1a1a1a; line-height: 1.6;
    }
    h1 { font-size: 24px; margin-bottom: 4px; }
    .sub { color: #666; font-size: 14px; margin-bottom: 32px; }
    input[type=text] {
      width: 100%; padding: 12px; font-size: 16px;
      border: 1px solid #ccc; border-radius: 6px;
      font-family: inherit; box-sizing: border-box;
    }
    button {
      margin-top: 12px; padding: 10px 20px; font-size: 15px;
      background: #1a2b4c; color: white; border: none;
      border-radius: 6px; cursor: pointer; font-family: inherit;
    }
    button:disabled { background: #999; cursor: default; }
    .answer {
      margin-top: 32px; padding: 20px; background: #f4f6f9;
      border-radius: 6px; white-space: pre-wrap;
    }
    .sources { margin-top: 24px; }
    .sources h3 { font-size: 14px; color: #666; margin-bottom: 8px; }
    .cite {
      font-size: 13px; padding: 10px; border-left: 3px solid #ccc;
      margin-bottom: 10px; color: #444;
    }
    .cite .src { font-weight: 600; color: #1a2b4c; }
    .stats { margin-top: 40px; font-size: 13px; color: #888; }
  </style>
</head>
<body>
  <h1>Lattice</h1>
  <div class="sub">Ask your documents. Everything runs locally.</div>

  <input type="text" id="q" placeholder="What do you want to know?"
         autofocus>
  <button id="go" onclick="ask()">Ask</button>

  <div id="out"></div>
  <div class="stats" id="stats"></div>

  <script>
    async function ask() {
      const q = document.getElementById('q').value.trim();
      if (!q) return;

      const btn = document.getElementById('go');
      const out = document.getElementById('out');
      btn.disabled = true;
      btn.textContent = 'Thinking...';
      out.innerHTML = '';

      try {
        const res = await fetch('/ask', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({question: q})
        });
        const data = await res.json();

        let html = '<div class="answer">' + escapeHtml(data.answer)
                 + '</div>';
        if (data.citations && data.citations.length) {
          html += '<div class="sources"><h3>SOURCES</h3>';
          for (const c of data.citations) {
            html += '<div class="cite"><span class="src">'
                 + escapeHtml(c.source) + ' (chunk ' + c.chunk_index
                 + ')</span><br>' + escapeHtml(c.text) + '</div>';
          }
          html += '</div>';
        }
        out.innerHTML = html;
      } catch (e) {
        out.innerHTML = '<div class="answer">Error: '
                      + escapeHtml(String(e)) + '</div>';
      }

      btn.disabled = false;
      btn.textContent = 'Ask';
    }

    function escapeHtml(s) {
      const d = document.createElement('div');
      d.textContent = s;
      return d.innerHTML;
    }

    document.getElementById('q').addEventListener('keydown', e => {
      if (e.key === 'Enter') ask();
    });

    fetch('/stats').then(r => r.json()).then(d => {
      document.getElementById('stats').textContent =
        d.chunks + ' chunks from ' + d.files + ' files';
    });
  </script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    store = None  # set in main()

    def log_message(self, fmt, *args):
        pass

    def _json(self, payload, status=200):
        body = json.dumps(payload).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/":
            body = PAGE.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        elif self.path == "/stats":
            self._json(Handler.store.stats())

        else:
            self._json({"error": "not found"}, 404)

    def do_POST(self):
        if self.path != "/ask":
            self._json({"error": "not found"}, 404)
            return

        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length)

        try:
            payload = json.loads(raw)
            question = payload.get("question", "").strip()
        except Exception:
            self._json({"error": "bad request"}, 400)
            return

        if not question:
            self._json({"error": "empty question"}, 400)
            return

        print(f"  asked: {question}")
        answer = Handler.store.ask(question)

        self._json({
            "answer": answer.text,
            "citations": [
                {
                    "source": c.source,
                    "chunk_index": c.chunk_index,
                    "text": c.text,
                }
                for c in answer.citations
            ],
        })


def main():
    print("opening store...")
    Handler.store = DocumentStore(DB_DIR)

    print("warming up models...")
    Handler.store._ensure_embedder()
    Handler.store._ensure_reranker()
    Handler.store._ensure_synthesizer()

    server = HTTPServer(("127.0.0.1", PORT), Handler)
    print(f"\nready at http://127.0.0.1:{PORT}")
    print("ctrl-c to stop\n")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nstopped")


if __name__ == "__main__":
    main()