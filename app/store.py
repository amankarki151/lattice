"""Ties chunking, embedding, and Lattice together.

Lattice stores vectors keyed by uint64. It doesn't store text. So
this keeps a sidecar mapping from id -> (source file, chunk index,
text) so a search result can be turned back into something readable.

The sidecar is plain JSON for now. A real version would use SQLite -
JSON means loading the whole mapping into memory and rewriting the
entire file on every save, which stops being reasonable somewhere
around a few hundred thousand chunks.
"""

import json
import os
import sys
from dataclasses import asdict, dataclass
from typing import Dict, List, Optional, Tuple
from reranker import Reranker
from synthesizer import Answer, Citation, Synthesizer

import numpy as np

from chunker import Chunk, chunk_directory
from embedder import Embedder

# The compiled module lives in a build directory rather than being
# installed. LATTICE_BUILD_DIR lets each environment declare its own
# name (local dev uses build-test, CI uses build) instead of
# hardcoding one and breaking the other - the exact bug bench/run.py
# hit yesterday.
_BUILD_DIR_NAME = os.environ.get("LATTICE_BUILD_DIR", "build-test")
_BUILD_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    _BUILD_DIR_NAME,
    "bindings",
)
sys.path.insert(0, _BUILD_DIR)

import lattice  # noqa: E402


@dataclass
class StoredChunk:
    id: int
    text: str
    source: str
    index: int


class DocumentStore:
    def __init__(self, db_dir: str):
        self.db_dir = db_dir
        os.makedirs(db_dir, exist_ok=True)

        self.sidecar_path = os.path.join(db_dir, "chunks.json")
        self.db = lattice.Database(db_dir)
        self.embedder: Optional[Embedder] = None
        self.reranker: Optional[Reranker] = None
        self.synthesizer: Optional[Synthesizer] = None

        self.chunks: Dict[int, StoredChunk] = {}
        self._load_sidecar()

    def _load_sidecar(self):
        if not os.path.exists(self.sidecar_path):
            return
        with open(self.sidecar_path) as f:
            raw = json.load(f)
        self.chunks = {
            int(k): StoredChunk(**v) for k, v in raw.items()
        }
        print(f"loaded {len(self.chunks)} chunk records")

    def _save_sidecar(self):
        raw = {str(k): asdict(v) for k, v in self.chunks.items()}
        tmp = self.sidecar_path + ".tmp"
        with open(tmp, "w") as f:
            json.dump(raw, f)
        # Same atomic-rename trick the segment writer uses - a crash
        # mid-write leaves the old file intact rather than a corrupt one.
        os.replace(tmp, self.sidecar_path)

    def _ensure_embedder(self):
        if self.embedder is None:
            self.embedder = Embedder()

    def _ensure_reranker(self):
        if self.reranker is None:
            self.reranker = Reranker()

    def _ensure_synthesizer(self):
        if self.synthesizer is None:
            self.synthesizer = Synthesizer()

    def ingest_directory(self, directory: str) -> int:
        """Chunk, embed, and store everything in a directory."""
        print(f"scanning {directory}...")
        chunks: List[Chunk] = chunk_directory(directory)

        if not chunks:
            print("nothing to ingest")
            return 0

        print(f"\n{len(chunks)} chunks to embed")
        self._ensure_embedder()

        texts = [c.text for c in chunks]
        vectors = self.embedder.embed(texts)

        # Ids continue from wherever the store left off, so ingesting
        # a second directory doesn't overwrite the first.
        next_id = max(self.chunks.keys(), default=0) + 1

        print(f"inserting into lattice...")
        for chunk, vec in zip(chunks, vectors):
            cid = next_id
            next_id += 1

            self.db.insert(lattice.Vector(cid, vec.tolist()))
            self.chunks[cid] = StoredChunk(
                id=cid, text=chunk.text, source=chunk.source,
                index=chunk.index,
            )

        self._save_sidecar()
        self.db.checkpoint()

        print(f"done. {len(self.chunks)} chunks total in store")
        return len(chunks)

    def search(self, query: str, k: int = 5) -> List[Tuple[StoredChunk, float]]:
        """Find the k chunks most similar to a query string."""
        if not self.chunks:
            return []

        self._ensure_embedder()
        qvec = self.embedder.embed_one(query)

        hits = self.db.search(qvec.tolist(), k)

        results = []
        for h in hits:
            chunk = self.chunks.get(h.id)
            if chunk is None:
                # Vector in the index with no sidecar record - shouldn't
                # happen, but skip rather than crash if the two ever
                # drift out of sync.
                continue
            results.append((chunk, h.distance))
        return results

    def ask(self, question: str, retrieve_k: int = 15,
            rerank_k: int = 4) -> Answer:
        """Full pipeline: retrieve, re-rank, synthesize, cite.

        Retrieves wide and re-ranks narrow on purpose. Vector distance
        answers "is this about the same topic"; the cross-encoder
        answers "does this actually address the question". Those are
        different questions, and the second one is worth asking over a
        shortlist even though it's too slow to ask over everything.
        """
        hits = self.search(question, k=retrieve_k)
        if not hits:
            return Answer(
                text="Nothing has been ingested yet, so there's "
                     "nothing to search.",
                citations=[],
            )

        chunks = [c for c, _dist in hits]
        passages = [c.text for c in chunks]

        # Re-rank - slower, more accurate, only over the shortlist.
        self._ensure_reranker()
        ranked = self.reranker.rerank(question, passages, top_k=rerank_k)

        kept_chunks = [chunks[i] for i, _score in ranked]
        kept_passages = [passages[i] for i, _score in ranked]

        citations = [
            Citation(source=c.source, chunk_index=c.index, text=c.text)
            for c in kept_chunks
        ]

        # Synthesize an answer, carrying its sources with it.
        self._ensure_synthesizer()
        return self.synthesizer.synthesize(
            question, kept_passages, citations
        )

    def stats(self) -> dict:
        sources = {c.source for c in self.chunks.values()}
        return {
            "chunks": len(self.chunks),
            "files": len(sources),
            "vectors_in_db": len(self.db),
        }
