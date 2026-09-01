"""Second-pass relevance scoring with a cross-encoder.

The embedder is a bi-encoder: it encodes the query and each document
separately, then compares vectors. That's fast, because documents can
be embedded once ahead of time and reused for every query. It's also
less accurate, because the model never sees the query and the
document together - it's comparing two independently-formed
impressions.

A cross-encoder feeds the query and document through the model as a
pair and outputs a relevance score directly. Much more accurate, and
far too slow to run across a whole collection. Which is exactly why
it runs second, over ~15 candidates, rather than first over
thousands.
"""

from typing import List, Tuple

MODEL_NAME = "cross-encoder/ms-marco-MiniLM-L-6-v2"


class Reranker:
    def __init__(self, model_name: str = MODEL_NAME):
        from sentence_transformers import CrossEncoder

        print(f"loading reranker {model_name}...")
        self.model = CrossEncoder(model_name)
        print("reranker loaded")

    def rerank(self, query: str, passages: List[str],
               top_k: int = 4) -> List[Tuple[int, float]]:
        """Score passages against the query.

        Returns (original_index, score) pairs, best first, capped at
        top_k. Scores are raw model logits - higher is better, but the
        absolute value isn't calibrated to anything, so only the
        ordering and relative gaps are meaningful.
        """
        if not passages:
            return []

        pairs = [(query, p) for p in passages]
        scores = self.model.predict(pairs)

        ranked = sorted(
            enumerate(scores), key=lambda x: x[1], reverse=True
        )
        return [(i, float(s)) for i, s in ranked[:top_k]]