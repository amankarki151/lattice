"""Turn text into vectors, locally.

Uses sentence-transformers with all-MiniLM-L6-v2: runs on CPU, no
API key, no network call once the model is cached. The entire point
of this app is that nothing leaves the machine, so calling a hosted
embedding API would defeat it.

384 dimensions - small enough to stay fast, big enough to be useful.
Note that's 3x the 128 dimensions of the SIFT benchmark data, so
every distance computation costs proportionally more.
"""

from typing import List

import numpy as np

MODEL_NAME = "all-MiniLM-L6-v2"
DIM = 384


class Embedder:
    def __init__(self, model_name: str = MODEL_NAME):
        # Imported here rather than at module level - it's a slow
        # import and pulls in torch, so anything that only needs the
        # chunker shouldn't pay for it.
        from sentence_transformers import SentenceTransformer

        print(f"loading {model_name}...")
        self.model = SentenceTransformer(model_name)
        self.dim = self.model.get_embedding_dimension()
        print(f"loaded, {self.dim} dimensions")

    def embed(self, texts: List[str], batch_size: int = 32) -> np.ndarray:
        """Embed a list of texts into an (n, dim) float32 array."""
        if not texts:
            return np.zeros((0, self.dim), dtype=np.float32)

        vectors = self.model.encode(
            texts,
            batch_size=batch_size,
            show_progress_bar=len(texts) > 100,
            convert_to_numpy=True,
        )
        return vectors.astype(np.float32)

    def embed_one(self, text: str) -> np.ndarray:
        """Embed a single string."""
        return self.embed([text])[0]