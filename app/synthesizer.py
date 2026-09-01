"""Turn retrieved passages into a written answer, locally.

Uses a small instruction-tuned model that runs on CPU. This is
deliberately not a hosted API: the entire premise of this app is that
documents never leave the machine, and sending retrieved passages to
someone else's server would break that completely.

The trade is real and worth stating plainly - answers are shorter and
less fluent than a large hosted model would produce. Privacy is what's
being bought with that.

Every answer carries citations. A synthesized answer with no source is
worse than no answer at all: it reads as authoritative and can't be
checked.
"""

from dataclasses import dataclass
from typing import List

MODEL_NAME = "google/flan-t5-base"
MAX_CONTEXT_CHARS = 1500


@dataclass
class Citation:
    source: str
    chunk_index: int
    text: str


@dataclass
class Answer:
    text: str
    citations: List[Citation]


class Synthesizer:
    def __init__(self, model_name: str = MODEL_NAME):
        from transformers import (AutoModelForSeq2SeqLM, AutoTokenizer)

        print(f"loading synthesizer {model_name}...")
        self.tokenizer = AutoTokenizer.from_pretrained(model_name)
        self.model = AutoModelForSeq2SeqLM.from_pretrained(model_name)
        print("synthesizer loaded")

    def _build_prompt(self, question: str, passages: List[str]) -> str:
        # Truncate context rather than let it silently overflow the
        # model's input limit, which would drop passages from the end
        # without any indication it happened.
        context_parts = []
        used = 0
        for i, p in enumerate(passages, 1):
            piece = f"[{i}] {p.strip()}"
            if used + len(piece) > MAX_CONTEXT_CHARS:
                break
            context_parts.append(piece)
            used += len(piece)

        context = "\n\n".join(context_parts)

        return (
            f"Answer the question in a full sentence, using only the "
            f"context below. If the context does not contain the "
            f"answer, say so.\n\n"
            f"Context:\n{context}\n\n"
            f"Question: {question}\n\n"
            f"Answer:"
        )

    def synthesize(self, question: str, passages: List[str],
                   citations: List[Citation],
                   max_tokens: int = 200) -> Answer:
        prompt = self._build_prompt(question, passages)

        inputs = self.tokenizer(
            prompt, return_tensors="pt", truncation=True, max_length=512
        )
        outputs = self.model.generate(
    **inputs,
    max_new_tokens=max_tokens,
    min_new_tokens=20,
    num_beams=4,
    early_stopping=True,
)
        text = self.tokenizer.decode(outputs[0], skip_special_tokens=True)

        return Answer(text=text.strip(), citations=citations)