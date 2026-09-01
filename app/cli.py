"""Command-line interface for the document store.

usage:
  python cli.py ingest <directory>
  python cli.py search "<query>" [k]
  python cli.py stats
"""

import sys

from store import DocumentStore

DB_DIR = "data/db"


def cmd_ingest(directory: str):
    store = DocumentStore(DB_DIR)
    store.ingest_directory(directory)


def cmd_search(query: str, k: int):
    store = DocumentStore(DB_DIR)
    results = store.search(query, k)

    if not results:
        print("no results - has anything been ingested?")
        return

    print(f'\ntop {len(results)} for: "{query}"\n')
    for i, (chunk, distance) in enumerate(results, 1):
        print(f"{i}. {chunk.source} (chunk {chunk.index})  "
              f"distance {distance:.4f}")
        snippet = chunk.text.replace("\n", " ")
        if len(snippet) > 250:
            snippet = snippet[:250] + "..."
        print(f"   {snippet}\n")


def cmd_stats():
    store = DocumentStore(DB_DIR)
    s = store.stats()
    print(f"chunks:  {s['chunks']}")
    print(f"files:   {s['files']}")
    print(f"vectors: {s['vectors_in_db']}")

def cmd_ask(question: str):
    store = DocumentStore(DB_DIR)
    answer = store.ask(question)

    print(f'\nQ: {question}\n')
    print(f'A: {answer.text}\n')

    if answer.citations:
        print("sources:")
        for c in answer.citations:
            snippet = c.text.replace("\n", " ")
            if len(snippet) > 120:
                snippet = snippet[:120] + "..."
            print(f"  [{c.source} chunk {c.chunk_index}]")
            print(f"    {snippet}")

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1

    cmd = sys.argv[1]

    if cmd == "ingest":
        if len(sys.argv) < 3:
            print("ingest needs a directory")
            return 1
        cmd_ingest(sys.argv[2])

    elif cmd == "search":
        if len(sys.argv) < 3:
            print("search needs a query")
            return 1
        k = int(sys.argv[3]) if len(sys.argv) > 3 else 5
        cmd_search(sys.argv[2], k)

    elif cmd == "ask":
        if len(sys.argv) < 3:
            print("ask needs a question")
            return 1
        cmd_ask(sys.argv[2])

    elif cmd == "stats":
        cmd_stats()

    else:
        print(f"unknown command: {cmd}")
        print(__doc__)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())