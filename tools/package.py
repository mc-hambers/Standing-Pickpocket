from pathlib import Path
import struct,zipfile
root=Path(__file__).resolve().parents[1]
data=root/'Data'
required=['StandingPickpocket.esp','Seq/StandingPickpocket.seq','Scripts/SP_MCM.pex','Scripts/SP_Native.pex','SKSE/Plugins/StandingPickpocket.dll','SKSE/Plugins/StandingPickpocket.ini']
for rel in required:
 p=data/rel
 assert p.is_file() and p.stat().st_size>0, rel
b=(data/'StandingPickpocket.esp').read_bytes()
assert b[:4]==b'TES4'
assert struct.unpack_from('<I',b,8)[0] & 0x200, 'Missing ESL light flag'
h=24+struct.unpack_from('<I',b,4)[0]
assert b[h:h+4]==b'GRUP' and struct.unpack_from('<I',b,h+4)[0]==len(b)-h
p=h+24; assert b[p:p+4]==b'QUST'
assert struct.unpack_from('<I',b,p+12)[0]==0x01000800
assert 0x800 <= (struct.unpack_from('<I',b,p+12)[0] & 0xFFFFFF) <= 0xFFF, 'Record outside legacy light range'
assert (data/'Seq/StandingPickpocket.seq').read_bytes()==struct.pack('<I',0x01000800)
end=p+24+struct.unpack_from('<I',b,p+4)[0]; p+=24;subs={}
while p<end:
 tag=b[p:p+4];n=struct.unpack_from('<H',b,p+4)[0];subs[tag]=b[p+6:p+6+n];p+=6+n
assert p==end==len(b)
assert struct.unpack_from('<H',subs[b'DNAM'])[0]&1
assert subs[b'ALFR']==struct.pack('<I',0x14)
v=subs[b'VMAD'];pos=0
def numbers(fmt):
 global pos
 t=struct.unpack_from(fmt,v,pos);pos+=struct.calcsize(fmt);return t

def string():
 global pos
 n=numbers('<H')[0];s=v[pos:pos+n];pos+=n;return s
assert numbers('<HHH')==(5,2,1)
assert string()==b'SP_MCM';assert numbers('<BH')==(0,0)
assert numbers('<BHHH')==(2,0,0,1)
assert numbers('<HHIHHH')==(0,0,0x01000800,5,2,1)
assert string()==b'SKI_PlayerLoadGameAlias';assert numbers('<BH')==(0,0)
assert pos==len(v)
for rel in ['Scripts/SP_MCM.pex','Scripts/SP_Native.pex']:
 pex=(data/rel).read_bytes();assert pex[:4] in (b'\xfa\x57\xc0\xde',b'\xde\xc0\x57\xfa')
assert b'SKI_ConfigBase' in (data/'Scripts/SP_MCM.pex').read_bytes()
dll=(data/'SKSE/Plugins/StandingPickpocket.dll').read_bytes();assert dll[:2]==b'MZ'
for symbol in [b'SKSEPlugin_Load',b'SKSEPlugin_Query',b'SKSEPlugin_Version']:assert symbol in dll
assert struct.unpack_from('<H',dll,struct.unpack_from('<I',dll,0x3c)[0]+4)[0]==0x8664
# Check the real exported metadata, including SKSE's 1.6.629+ rejection branch.
import ctypes
library=ctypes.WinDLL(str(data/'SKSE/Plugins/StandingPickpocket.dll'))
metadata=bytes((ctypes.c_ubyte*0x350).in_dll(library,'SKSEPlugin_Version'))
extended,independence=struct.unpack_from('<II',metadata,0x304)
assert independence & 1, 'Missing Address Library declaration'
assert extended & 1, 'Missing runtime-independent structure declaration'
for runtime in [(1,5,97),(1,6,353),(1,6,629),(1,6,1170),(1,7,104)]:
    rejected=(runtime >= (1,6,629) and (independence & 3) and not (independence & 4) and not (extended & 1))
    assert not rejected, 'SKSE structure-layout rejection: '+str(runtime)
assert struct.unpack_from('<I',metadata,4)[0]==0x01000020
print('Exported metadata passes the SKSE structure-layout gate for SE and AE runtime families.')
out=root/'output/Standing-Pickpocket-v1.0.2-SE-AE.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
 for p in data.rglob('*'):
  if p.is_file() and p.suffix.lower() in {'.esp','.seq','.pex','.dll','.ini'}:z.write(p,p.relative_to(data).as_posix())
 z.write(root/'LICENSE','LICENSE')
with zipfile.ZipFile(out) as z:
 assert z.testzip() is None
 assert set(z.namelist())==set(required+['LICENSE'])
 assert z.read('SKSE/Plugins/StandingPickpocket.dll')==(data/'SKSE/Plugins/StandingPickpocket.dll').read_bytes()
print('Validated x64 SKSE exports, MCM quest/VMAD/player alias, SEQ, PEX files and install archive.')
print(out)




