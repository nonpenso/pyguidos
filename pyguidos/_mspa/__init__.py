"""
Internal MSPA engine package for pyGuidos.

This subpackage bundles a minimal subset of the miallib C library
(Morphological Segmentation of Binary Patterns, by Pierre Soille and
Peter Vogt) compiled as the private extension module ``_mspa``.

The compiled function is exposed to the rest of pyGuidos through
``pyguidos.mspa`` (the public ``pg.mspa`` tool). End users do not import
this package directly.

miallib is licensed under the GNU General Public License v3; see the
vendored source headers under ``miallib/`` and the project LICENSE notes.
"""
