# SPDX-License-Identifier: GPL-3.0-or-later
"""Read-only game discovery with original and prepared folder layouts."""
from pathlib import Path
import hashlib
import os
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class GameFilesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.build.name)/('game_files.exe' if os.name == 'nt' else 'game_files')
        subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                        '-Icommon', 'tests/test_game_files.c', 'common/catalog.c',
                        'common/game_files.c', '-o', str(cls.binary)], cwd=ROOT, check=True)

    @classmethod
    def tearDownClass(cls):
        cls.build.cleanup()

    def fixture(self, folder, game='00000001', metadata='tmd.1', size=2312):
        folder.mkdir(parents=True, exist_ok=True)
        content = bytearray(0x4000)
        content[0x12] = 2
        struct.pack_into('<II', content, 0x230, 0x4b545345, 0x30004)
        tmd = bytearray(size)
        struct.pack_into('>I', tmd, 0, 0x10001)
        issuer = b'Root-CA00000001-CP00000007'
        tmd[0x140:0x140+len(issuer)] = issuer
        struct.pack_into('>II', tmd, 0x18c, 0x30004, 0x4b545345)
        struct.pack_into('>H', tmd, 0x1de, 1)
        struct.pack_into('>IHHQ', tmd, 0x1e4, 1, 0, 1, len(content))
        tmd[0x1f4:0x208] = hashlib.sha1(content).digest()
        (folder/game).write_bytes(content)
        (folder/metadata).write_bytes(tmd)
        return folder/game, folder/metadata

    def resolve(self, game, code=0):
        before = {p.name: p.read_bytes() for p in game.parent.iterdir() if p.is_file()}
        result = subprocess.run([str(self.binary), game.as_posix()], capture_output=True, text=True)
        self.assertEqual(result.returncode, code, result.stdout+result.stderr)
        self.assertEqual(before, {p.name: p.read_bytes() for p in game.parent.iterdir() if p.is_file()})
        if code == 0:
            self.assertEqual(result.stdout.strip(),
                             '4b545345 00000001 '+hashlib.sha1(game.read_bytes()).hexdigest())
        return result.stdout

    def test_original_and_prepared_names(self):
        for game, metadata in [('00000001', 'tmd.1'), ('00000001', 'tmd.256'),
                               ('00000001.APP', 'TITLE.TMD'), ('game.nds', 'game.tmd'),
                               ('game.dsi', 'tmd.0'), ('game.srl', 'TMD.1'), ('game.app', 'tmd')]:
            for size in (520, 2312):
                with self.subTest(game=game, metadata=metadata, size=size), tempfile.TemporaryDirectory() as tmp:
                    path, _ = self.fixture(Path(tmp)/'DSiWare/Game (USA).folder', game, metadata, size)
                    (path.parent/'titleKey.bin').write_bytes(bytes(16))
                    (path.parent/'titlePass.txt').write_text('unused')
                    self.resolve(path)

    def test_ambiguous_versions_are_rejected_regardless_of_filename(self):
        for game in ('game.nds', '00000001'):
            with self.subTest(game=game), tempfile.TemporaryDirectory() as tmp:
                path, metadata = self.fixture(Path(tmp), game, 'game.tmd')
                data = bytearray(metadata.read_bytes()); data[0x1f4] ^= 1
                (path.parent/'tmd.2').write_bytes(data)
                self.assertIn('More than one', self.resolve(path, 1))

    def test_identical_metadata_records_are_accepted(self):
        with tempfile.TemporaryDirectory() as tmp:
            path, metadata = self.fixture(Path(tmp))
            (path.parent/'title.tmd').write_bytes(metadata.read_bytes()[:520])
            self.resolve(path)

    def test_mismatched_or_invalid_metadata_does_not_match(self):
        for offset in (0, 0x140, 0x18c, 0x190, 0x1df, 0x1e7, 0x1e9, 0x1eb, 0x1f0):
            with self.subTest(offset=offset), tempfile.TemporaryDirectory() as tmp:
                path, metadata = self.fixture(Path(tmp))
                data = bytearray(metadata.read_bytes()); data[offset] ^= 1
                metadata.write_bytes(data)
                self.resolve(path, 1)

    def test_does_not_search_other_folders_or_use_partial_files(self):
        for condition in ('sibling', 'truncated', 'directory', 'missing'):
            with self.subTest(condition=condition), tempfile.TemporaryDirectory() as tmp:
                path, metadata = self.fixture(Path(tmp)/'game')
                data = metadata.read_bytes(); metadata.unlink()
                if condition == 'sibling': (path.parent.parent/'tmd.1').write_bytes(data)
                if condition == 'truncated': metadata.write_bytes(data[:519])
                if condition == 'directory': metadata.mkdir()
                self.resolve(path, 1)

    def test_unrelated_metadata_is_ignored(self):
        with tempfile.TemporaryDirectory() as tmp:
            path, metadata = self.fixture(Path(tmp))
            data = bytearray(metadata.read_bytes()); data[0x190] ^= 1
            (path.parent/'tmd.2').write_bytes(data)
            self.resolve(path)

    def test_invalid_content_is_not_a_game(self):
        for game in ('titleKey.bin', 'file.zip', '00000001'):
            with self.subTest(game=game), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp)/game; path.write_bytes(bytes(0x4000))
                self.resolve(path, 2)
