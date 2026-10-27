from pathlib import Path
import json,zipfile,hashlib,subprocess,ctypes,struct,argparse
root=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--quickpocket',required=True);a.add_argument('--standing',required=True);args=a.parse_args()
profiles=json.loads((root/'profiles.json').read_text())
import pefile
for profile,path in zip(profiles,[args.quickpocket,args.standing]):
 file=Path(path);data=file.read_bytes();assert hashlib.sha256(data).hexdigest()==profile['sha256']
 pe=pefile.PE(data=data);assert pe.FILE_HEADER.TimeDateStamp==profile['timestamp'] and pe.OPTIONAL_HEADER.SizeOfImage==profile['imageSize']
 for site in profile['sites']:
  expected=bytes.fromhex(site['bytes']);assert pe.get_data(site['rva'],len(expected))==expected
  assert any(s.Characteristics&0x20000000 and s.VirtualAddress<=site['rva']<s.VirtualAddress+s.Misc_VirtualSize for s in pe.sections)
# The three copied prologues were disassembled: exact, whole instructions with
# no relative call, branch or RIP-relative operand. Their fixed byte profiles
# reject compiler changes before any runtime modification.
assert bytes.fromhex(profiles[0]['sites'][0]['bytes'])==bytes.fromhex('48895c24104889742418574883ec40')
assert bytes.fromhex(profiles[0]['sites'][2]['bytes'])==bytes.fromhex('48895c24085556574883ec700fb6f9')
assert bytes.fromhex(profiles[1]['sites'][0]['bytes'])==bytes.fromhex('48895c2408574883ec408bda488bf9')
assert profiles[0]['sites'][1]['rva']+6+int.from_bytes(bytes.fromhex(profiles[0]['sites'][1]['bytes'])[2:],'little',signed=True)==0x1ef4c
for test in ['AdapterTests.exe','HookTests.exe']:
 result=subprocess.run([str(root/'build'/test)],text=True,capture_output=True,check=True);print(result.stdout.strip())
dll=root/'build/package/SKSE/Plugins/StandingQuickPocket.dll';lib=ctypes.WinDLL(str(dll.resolve()))
getattr(lib,'SKSEPlugin_Load');version=(ctypes.c_uint32*2).in_dll(lib,'SKSEPlugin_Version');assert version[1]==0x10010
release=root/'releases';release.mkdir(exist_ok=True)
runtime=release/'Standing-QuickPocket-Compatibility-v0.1.1.zip';source=release/'Standing-QuickPocket-Compatibility-v0.1.1-source.zip'
sourceNote='GPL-3.0-or-later. Corresponding adapter, CommonLib and dependency source is supplied in '+source.name+' alongside this download. No QuickPocket/Standing Pickpocket implementation is included.\n'
with zipfile.ZipFile(runtime,'w',zipfile.ZIP_DEFLATED) as z:
 z.write(dll,'SKSE/Plugins/StandingQuickPocket.dll');z.write(root/'LICENSE.txt','LICENSE.txt');z.writestr('SOURCE-LICENSE.txt',sourceNote)
 # Preserve complete license texts for the same CommonLib/dependency stack.
 with zipfile.ZipFile(root.parent/'Pickpocket-Overhaul/releases/Pickpocket-Chance-Rework-v1.0.0.zip') as old:
  for name in old.namelist():
   if name.startswith('Third-Party-Licenses/'):
    parts=name.split('/');z.writestr('Third-Party-Licenses/'+'/'.join(parts[2:]),old.read(name))
report={'version':'0.1.1','standing_version':'1.0.2 unchanged','quickpocket_version':'1.0.0 unchanged','binary_profiles':'verified','native_build':'passed','native_hook_smoke':'passed','session_policy':'passed','gameplay_tested':False,'diagnostic_only':True,'combat_transfer_observation':True,'pcr_hooks':False,'runtime_ini':False,'runtime_esp':False,'runtime_papyrus':False,'runtime_dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest()}
(root/'validation-results.json').write_text(json.dumps(report,indent=2)+'\n')
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as z:
 prefix='Standing-QuickPocket-Compatibility/'
 for name in ['CMakeLists.txt','vcpkg.json','LICENSE.txt','README.md','profiles.json','validation-results.json']:
  z.write(root/name,prefix+name)
 for folder in ['src','tests','scripts']:
  for p in (root/folder).rglob('*'):
   if p.is_file() and '__pycache__' not in p.parts:z.write(p,prefix+p.relative_to(root).as_posix())
 vendor=root.parent/'Pickpocket-Overhaul/vendor/CommonLibSSE-NG'
 for p in vendor.rglob('*'):
  if p.is_file() and '.git' not in p.parts and '__pycache__' not in p.parts:z.write(p,prefix+'vendor/CommonLibSSE-NG/'+p.relative_to(vendor).as_posix())
 deps=root.parent/'Pickpocketing-Loot/tools/vcpkg-downloads'
 for p in deps.glob('*'):
  if p.is_file() and (p.name.endswith('.tar.gz') or p.suffix=='.patch'):z.write(p,prefix+'dependency-sources/'+p.name)
 for name in ['spdlog','fmt','directxmath','directxtk','rapidcsv','vcpkg-cmake','vcpkg-cmake-config']:
  port=root.parent/'Pickpocketing-Loot/tools/vcpkg/ports'/name
  for p in port.rglob('*'):
   if p.is_file():z.write(p,prefix+'dependency-sources/ports/'+name+'/'+p.relative_to(port).as_posix())
for p in [runtime,source]:
 with zipfile.ZipFile(p) as z:assert z.testzip() is None
with zipfile.ZipFile(runtime) as z:
 assert [n for n in z.namelist() if n.lower().endswith('.dll')]==['SKSE/Plugins/StandingQuickPocket.dll']
 assert not any(n.lower().endswith(('.ini','.esp','.esl','.pex','.pdb')) for n in z.namelist())
with zipfile.ZipFile(source) as z:assert not any('QuickPocket-reference' in n or n.endswith('/QuickPocket.dll') or n.endswith('/StandingPickpocket.dll') for n in z.namelist())
(release/'SHA256.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [runtime,source]},indent=2)+'\n')
print(json.dumps(report,indent=2));print(runtime);print(source)

