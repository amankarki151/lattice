"""Turn files into overlapping text chunks.

Fixed-size character windows with overlap. A fixed cut can land
mid-sentence and split an idea across two chunks so neither one is
retrievable on its own; overlapping means any given passage appears
at least once with its surrounding context intact.

Character-based rather than token-based on purpose - it avoids a
tokenizer dependency, and the embedding model truncates anything
overlong anyway.
"""

import os
from dataclasses import dataclass
from typing import List

CHUNK_SIZE = 500
OVERLAP = 100


@dataclass
class Chunk:
    text: str
    source: str      # file path it came from
    index: int       # which chunk within that file


def read_text_file(path: str) -> str:
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def read_pdf(path: str) -> str:
    from pypdf import PdfReader

    reader = PdfReader(path)
    parts = []
    for page in reader.pages:
        text = page.extract_text() or ""
        if text.strip():
            parts.append(text)
    return "\n\n".join(parts)


def read_file(path: str) -> str:
    """Extract text from a file, or return empty string if unsupported."""
    ext = os.path.splitext(path)[1].lower()

    if ext == ".pdf":
        return read_pdf(path)
    if ext in (".txt", ".md", ".markdown", ".rst", ".py", ".js", ".json"):
        return read_text_file(path)

    return ""  # unsupported type, skipped rather than crashed on


def chunk_text(text: str, source: str) -> List[Chunk]:
    """Split text into overlapping windows."""
    text = text.strip()
    if not text:
        return []

    chunks: List[Chunk] = []
    start = 0
    index = 0

    step = CHUNK_SIZE - OVERLAP
    if step <= 0:
        raise ValueError("OVERLAP must be smaller than CHUNK_SIZE")

    while start < len(text):
        piece = text[start:start + CHUNK_SIZE].strip()
        if piece:
            chunks.append(Chunk(text=piece, source=source, index=index))
            index += 1
        start += step

    return chunks


def chunk_directory(directory: str) -> List[Chunk]:
    """Walk a directory and chunk everything readable in it."""
    all_chunks: List[Chunk] = []

    for root, _dirs, files in os.walk(directory):
        for name in sorted(files):
            if name.startswith("."):
                continue

            path = os.path.join(root, name)
            try:
                text = read_file(path)
            except Exception as e:
                print(f"  skipped {path}: {e}")
                continue

            if not text.strip():
                continue

            file_chunks = chunk_text(text, path)
            all_chunks.extend(file_chunks)
            print(f"  {path}: {len(file_chunks)} chunks")

    return all_chunks