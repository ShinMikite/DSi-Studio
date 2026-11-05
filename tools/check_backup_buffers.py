"""Check the linked storage buffers are in shared RAM, not private ARM9 DTCM."""
from pathlib import Path
import re

layout=Path('build/arm9.map').read_text()
for name,expected in [('backup_raw_buffer',32768),('backup_hardware_sector',512)]:
    match=re.search(r'\.bss\.'+name+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',layout)
    if not match:
        raise RuntimeError(f'Missing linked buffer: {name}')
    address,size=map(lambda x:int(x,16),match.groups())
    assert size==expected and address%32==0
    assert 0x02000000<=address and address+size<=0x02400000
    print(f'{name}: {size} bytes in shared RAM at {address:#010x}, 32-byte aligned')
