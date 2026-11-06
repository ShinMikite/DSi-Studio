# SPDX-License-Identifier: GPL-3.0-or-later
"""I/O fault tests of the production installer; no real NAND or crypto used."""
from pathlib import Path
import os
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
TITLE = 'nand/title/00030004/4b545345'
TICKET = 'nand/ticket/00030004/4b545345.tik'
STAGE = 'nand/tmp/dsistudio/4b545345'


class InstallerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.build.name)/('installer.exe' if os.name == 'nt' else 'installer')
        subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                        '-Icommon', '-Itests', '-Ivendor/ntm-crypto/include',
                        'tests/test_installer.c', 'common/catalog.c', '-o', str(cls.binary)],
                       cwd=ROOT, check=True)

    @classmethod
    def tearDownClass(cls):
        cls.build.cleanup()

    def fixture(self, folder):
        for path in ('card', 'nand/sys', 'nand/ticket/00030004',
                     *(f'nand/title/{kind}' for kind in ('00030004', '00030005', '00030015', '00030017'))):
            (folder/path).mkdir(parents=True, exist_ok=True)
        rom = bytearray(0x4000)
        rom[0x12] = 2
        struct.pack_into('<I', rom, 0x1b0, 2)
        rom[0x1bf] = 4
        struct.pack_into('<IIII', rom, 0x230, 0x4b545345, 0x30004, 4096, 4096)
        (folder/'card/game.nds').write_bytes(rom)
        tmd = bytearray(520)
        struct.pack_into('>I', tmd, 0, 0x10001)
        issuer = b'Root-CA00000001-CP00000007'
        tmd[0x140:0x140+len(issuer)] = issuer
        struct.pack_into('>II', tmd, 0x18c, 0x30004, 0x4b545345)
        struct.pack_into('>H', tmd, 0x1de, 1)
        struct.pack_into('>IHHQ', tmd, 0x1e4, 1, 0, 1, len(rom))
        (folder/'card/game.tmd').write_bytes(tmd)
        cert = bytearray(0xa00)
        cert[0x840:0x84e] = b'Root-CA00000001'
        cert[0x884:0x88e] = b'CP00000007'
        struct.pack_into('>I', cert, 0x880, 1)
        (folder/'nand/sys/cert.sys').write_bytes(cert)
        sentinel = folder/'nand/title/00030004/12345678/data/public.sav'
        sentinel.parent.mkdir(parents=True)
        sentinel.write_bytes(b'existing save must survive')

    def run_install(self, folder, fault=0, scenario='ok', game='card/game.nds'):
        completed = subprocess.run([str(self.binary), str(fault), scenario, game], cwd=folder,
                                   capture_output=True, text=True)
        self.assertEqual(completed.returncode, 0, completed.stderr)
        values = completed.stdout.strip().split(' ', 4)
        self.assertEqual((folder/'nand/title/00030004/12345678/data/public.sav').read_bytes(),
                         b'existing save must survive')
        return tuple(map(int, values[:4])), values[4] if len(values) > 4 else ''

    def test_original_folder_installs_and_creates_saves(self):
        for content, metadata in [('00000001', 'tmd.1'), ('00000001.app', 'title.tmd')]:
            with self.subTest(content=content), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (folder/'card/game.nds').rename(folder/'card'/content)
                record = (folder/'card/game.tmd').read_bytes()
                (folder/'card/game.tmd').unlink()
                (folder/'card'/metadata).write_bytes(record + bytes(2312-520))
                # Original filenames also support optional same-stem save files.
                (folder/'card/00000001.pub').write_bytes(bytes([37])*4096)
                (success, _, unlocks, _), _ = self.run_install(folder, game='card/'+content)
                self.assertEqual((success, unlocks), (1, 1))
                self.assertEqual((folder/TITLE/'content/title.tmd').read_bytes(), record)
                self.assertEqual((folder/TITLE/'data/public.sav').read_bytes(), bytes([37])*4096)
                self.assertEqual((folder/TITLE/'data/private.sav').stat().st_size, 4096)
                self.assertEqual((folder/TITLE/'data/banner.sav').stat().st_size, 16384)

    def test_original_folder_preconditions_never_unlock_storage(self):
        for scenario in ('signature', 'digest'):
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (folder/'card/game.nds').rename(folder/'card/00000001')
                (folder/'card/game.tmd').rename(folder/'card/tmd.1')
                (success, _, unlocks, _), _ = self.run_install(folder, scenario=scenario, game='card/00000001')
                self.assertEqual((success, unlocks), (0, 0))

    def test_metadata_is_resolved_again_before_installing(self):
        for problem in ('missing', 'wrong-content', 'conflicting-version'):
            with self.subTest(problem=problem), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (folder/'card/game.nds').rename(folder/'card/00000001')
                tmd = folder/'card/game.tmd'
                if problem == 'missing':
                    tmd.unlink()
                else:
                    data = bytearray(tmd.read_bytes())
                    if problem == 'wrong-content':
                        struct.pack_into('>I', data, 0x1e4, 2)
                        tmd.unlink()
                    else:
                        data[0x1f4] = 1
                    (folder/'card/tmd.2').write_bytes(data)
                (success, _, unlocks, _), _ = self.run_install(folder, game='card/00000001')
                self.assertEqual((success, unlocks), (0, 0))
                self.assertFalse((folder/STAGE).exists())

    def test_every_success_path_io_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp); self.fixture(folder)
            (success, count, unlocks, _), _ = self.run_install(folder)
            self.assertEqual((success, unlocks), (1, 1))
            self.assertTrue((folder/TICKET).is_file())
            self.assertEqual((folder/TITLE/'content/00000001.app').read_bytes(),
                             (folder/'card/game.nds').read_bytes())
            for name, size in [('public', 4096), ('private', 4096), ('banner', 16384)]:
                self.assertEqual((folder/TITLE/f'data/{name}.sav').stat().st_size, size)
        for fault in range(1, count+1):
            with self.subTest(operation=fault), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (success, _, _, _), _ = self.run_install(folder, fault)
                self.assertFalse(success)
                self.assertFalse((folder/TITLE).exists())
                self.assertFalse((folder/TICKET).exists())

    def test_preconditions_never_unlock_storage(self):
        for scenario in ('backup', 'cached-backup', 'cancel', 'charger', 'signature', 'digest', 'guard', 'space'):
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (success, _, unlocks, _), _ = self.run_install(folder, scenario=scenario)
                self.assertEqual((success, unlocks), (0, 0))
                self.assertFalse((folder/STAGE).exists())

    def test_existing_title_and_unfinished_copy_are_preserved(self):
        for existing in (TITLE, STAGE, 'nand/tmp/dsistudio/4b545345.tik'):
            with self.subTest(existing=existing), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                marker = folder/existing
                if not existing.endswith('.tik'): marker /= 'preserved'
                marker.parent.mkdir(parents=True, exist_ok=True)
                marker.write_bytes(b'keep')
                (success, _, unlocks, _), _ = self.run_install(folder)
                self.assertEqual((success, unlocks), (0, 0))
                self.assertEqual(marker.read_bytes(), b'keep')

    def test_commit_and_rollback_failures(self):
        for scenario in ('commit', 'rollback'):
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as tmp:
                folder = Path(tmp); self.fixture(folder)
                (success, _, unlocks, resets), notice = self.run_install(folder, scenario=scenario)
                self.assertEqual((success, unlocks, resets), (0, 1, 1))
                self.assertEqual(notice, 'COPY STOPPED')
                self.assertFalse((folder/TITLE).exists())
                self.assertTrue((folder/STAGE).exists())
                self.assertEqual((folder/TICKET).exists(), scenario == 'rollback')

    def test_existing_ticket_is_never_deleted(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder = Path(tmp); self.fixture(folder)
            (folder/TICKET).write_bytes(b'existing ticket')
            (success, _, _, _), _ = self.run_install(folder, scenario='commit')
            self.assertFalse(success)
            self.assertEqual((folder/TICKET).read_bytes(), b'existing ticket')
