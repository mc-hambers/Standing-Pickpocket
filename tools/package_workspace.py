from pathlib import Path
import zipfile
ws=Path(__file__).resolve().parents[1]
out=ws/'output/Standing-Pickpocket-v1.0.2-Workspace.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in ws.rglob('*'):
  if not p.is_file():continue
  r=p.relative_to(ws)
  if len(r.parts)>1 and r.parts[0]=='vendor' and r.parts[1] in {'dak-current','DynamicActivationKey'}:continue
  if r.as_posix()=='src/Behind.h':continue
  if '.git' in r.parts or r.parts[0] in {'build','output'}:continue
  if len(r.parts)>1 and r.parts[0]=='tools' and r.parts[1] in {'msvc','compiler-cache','python-libs'}:continue
  if p.name in {'commonlib-prebuilt.zip','caprica.7z','7zr.exe','vsdownload.py'}:continue
  if p.suffix in {'.lib','.exp'} and r.parts[0]=='Data':continue
  z.write(p,'Standing-Pickpocket-v1.0.2/'+r.as_posix())
 z.write(ws/'output/Standing-Pickpocket-v1.0.2-SE-AE.zip','Standing-Pickpocket-v1.0.2/output/Standing-Pickpocket-v1.0.2-SE-AE.zip')
with zipfile.ZipFile(out) as z:assert z.testzip() is None
print('Workspace ZIP verified:',out.stat().st_size,'bytes')


