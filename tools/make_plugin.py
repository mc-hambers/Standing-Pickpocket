from pathlib import Path
import struct
root=Path(__file__).resolve().parents[1]
def sub(t,b):return t.encode()+struct.pack('<H',len(b))+b
def text(t,s):return sub(t,s.encode()+b'\0')
def rec(t,fid,b,flags=0):return t.encode()+struct.pack('<IIIIHH',len(b),flags,fid,0,44,0)+b
header=sub('HEDR',struct.pack('<fII',1.7,2,0x801))+text('CNAM','Max')+text('SNAM','Standing Pickpocket v1.0.2')+text('MAST','Skyrim.esm')+sub('DATA',b'\0'*8)
name=b'SP_MCM'
vmad=struct.pack('<HHH',5,2,1)+struct.pack('<H',len(name))+name+b'\0'+struct.pack('<H',0)
# Quest fragment header: version, fragment count, empty filename, zero aliases.
vmad+=struct.pack('<BHHH',2,0,0,1)
alias=b'SKI_PlayerLoadGameAlias'
vmad+=struct.pack('<HHIHHH',0,0,0x01000800,5,2,1)+struct.pack('<H',len(alias))+alias+b'\0'+struct.pack('<H',0)
body=text('EDID','SP_ConfigQuest')+sub('VMAD',vmad)+text('FULL','Standing Pickpocket')+sub('DNAM',struct.pack('<HBBfI',0x0111,0,255,0.,0))
body+=sub('NEXT',b'')+sub('ANAM',struct.pack('<I',1))+sub('ALST',struct.pack('<I',0))+text('ALID','PlayerAlias')+sub('FNAM',struct.pack('<I',0))+sub('ALFR',struct.pack('<I',0x14))+sub('VTCK',struct.pack('<I',0))+sub('ALED',b'')
quest=rec('QUST',0x01000800,body)
group=b'GRUP'+struct.pack('<I',24+len(quest))+b'QUST'+struct.pack('<IHHHH',0,0,0,0,0)+quest
(root/'Data/StandingPickpocket.esp').write_bytes(rec('TES4',0,header,flags=0x200)+group)
seq=root/'Data/Seq';seq.mkdir(exist_ok=True)
(seq/'StandingPickpocket.seq').write_bytes(struct.pack('<I',0x01000800))
print('Generated ESL-flagged ESP, start-enabled MCM quest and SEQ')




