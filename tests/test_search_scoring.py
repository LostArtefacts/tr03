from __future__ import annotations

import sys
import unittest
from pathlib import Path
from typing import Any
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import compare
import permute


class SearchScoringTests(unittest.TestCase):
    def setUp(self) -> None:
        self.scorer: permute.Scorer = object.__new__(permute.Scorer)
        self.scorer.addr = 0x480000
        self.scorer.symbol_names = ['_f']
        self.scorer.matched = None
        self.scorer.image_start = 0x400000
        self.scorer.image_end = 0x700000
        self.original: bytes = bytes.fromhex('31c0c3')
        self.scorer.orig_asm = compare.disasm(self.original, self.scorer.addr)
        self.scorer.orig_shapes = permute.instruction_shapes(
            self.scorer.orig_asm, self.scorer.image_start, self.scorer.image_end)

    def score(self, result: tuple[Any, ...]) -> int | None:
        with patch.object(self.scorer, 'compile', return_value='unused.obj'), \
                patch.object(self.scorer, 'results', return_value=[result]), \
                patch('permute.os.remove'):
            return self.scorer.score(('', '', ''))

    def test_relocated_operand_after_length_change(self) -> None:
        code = bytes.fromhex('8b0d00000000c3')
        original = bytes.fromhex('0faf1ddce34c00c3')
        mask = bytes.fromhex('00000101010100')
        result = ('_f', False, code, mask, original, [])
        assembly = self.scorer.ours(result)
        self.assertTrue(any('mov' in line and '0xeeeeeeee' in line for line in assembly))
        self.assertFalse(any('0x4ce3dc1d' in line for line in assembly))
        shapes = permute.instruction_shapes(assembly, 0x400000, 0x700000)
        self.assertTrue(any('mov' in shape and '<address>' in shape for shape in shapes))

    def test_different_encoding_with_same_mnemonics_is_not_a_match(self) -> None:
        encoded = bytes.fromhex('33c0c3')
        self.assertEqual(self.score(('_f', False, encoded, bytes(3), self.original, [])), 1)

    def test_indexed_relocation_marker_has_the_same_shape(self) -> None:
        code = bytes.fromhex('8b0c8500000000c3')
        original = bytes.fromhex('8b0c85dce34c00c3')
        mask = bytes.fromhex('0000000101010100')
        marked = self.scorer.ours(('_f', False, code, mask, original, []))
        expected = compare.disasm(original, self.scorer.addr)
        self.assertEqual(permute.instruction_shapes(marked, 0x400000, 0x700000),
                         permute.instruction_shapes(expected, 0x400000, 0x700000))

    def test_data_difference_is_not_a_match(self) -> None:
        self.assertEqual(self.score(('_f', False, self.original, bytes(3), self.original, [object()])), 1)

    def test_exact_comparison_matches(self) -> None:
        self.assertEqual(self.score(('_f', True, self.original, bytes(3), self.original, [])), 0)

    def test_breaking_a_previous_match_still_has_a_penalty(self) -> None:
        self.scorer.matched = {'_kept'}
        self.assertEqual(self.score(('_f', True, self.original, bytes(3), self.original, [])), 1000)


if __name__ == '__main__':
    unittest.main()
