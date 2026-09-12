"""Original red/green military-terminal 7K theme; writes only a new output folder."""
from pathlib import Path
import argparse
import json
import sys
import shutil
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(HERE.parent / 'blockbeat'))
from skin_agent import NativeEditor, build_skin, field_columns, write_json
from build_blockbeat import label, dst

GREEN = '#a9d56a'
DIM = '#368453'
RED = '#ba3538'
PALE = '#d0f4da'


def text(d, x, y, s, scale=2, color=GREEN):
    font = ImageFont.truetype('C:/Windows/Fonts/bahnschrift.ttf', scale*7)
    d.text((x,y),s,font=font,fill=color,anchor='lt')


def panel(d, x, y, w, h, title):
    d.rectangle((x, y, x+w, y+h), fill='#101819', outline='#070909', width=4)
    d.line((x+3, y+3, x+w-3, y+3, x+w-3, y+h-3), fill='#823033', width=2)
    d.line((x+3, y+h-3, x+3, y+3), fill='#df4a4d', width=2)
    d.rectangle((x+9, y+9, x+w-9, y+h-9), outline='#303b38')
    d.rectangle((x+10, y+10, x+w-10, y+36), fill='#1b2423')
    text(d, x+20, y+18, title, 1)
    for xx in (x+5, x+w-6):
        for yy in (y+5, y+h-6):
            d.rectangle((xx, yy, xx+2, yy+2), fill='#a8a89a')


def background():
    image = Image.new('RGBA', (1280, 720), '#090f10')
    d = ImageDraw.Draw(image)
    for y in range(0, 720, 4):
        d.line((0, y, 1279, y), fill='#0e1415')
    d.rectangle((16, 14, 1263, 79), fill='#171e1d', outline=RED, width=2)
    d.polygon([(30, 24), (63, 24), (74, 35), (74, 63), (41, 63), (30, 52)], outline=GREEN, width=2)
    text(d, 39, 35, 'R', 3)
    text(d, 94, 25, 'REDLINE', 5, PALE)
    text(d, 311, 57, 'COMBAT TERMINAL / RHYTHM OPERATIONS', 1)
    text(d, 780, 28, 'SECTOR 07 / ACTIVE', 2)
    text(d, 782, 55, 'TACTICAL LINK ESTABLISHED', 1, DIM)
    text(d, 1142, 32, '7K+S', 3, '#cf6460')
    panel(d, 18, 92, 538, 534, '01 / INPUT ARRAY                         PLAYER 01')
    starts, widths = [40]+[116+60*i for i in range(7)], [76]+[60]*7
    for i, (x, w) in enumerate(zip(starts, widths)):
        d.rectangle((x, 133, x+w-1, 570), fill='#141211' if i==0 else '#09150f' if i%2 else '#0b1010')
        d.line((x+w-1, 133, x+w-1, 570), fill='#2a4035')
        for y in range(160, 560, 64):
            d.line((x+2, y, x+w-3, y), fill='#12271b')
        d.rectangle((x+2, 583, x+w-3, 611), fill='#1c2922', outline=RED if i==0 else '#4d6e5b', width=2)
        word = 'SCR' if i==0 else str(i)
        text(d, x+(w-len(word)*12)//2, 590, word, 2, '#da6460' if i==0 else GREEN)
    d.line((40, 574, 535, 574), fill='#ed6861', width=3)
    panel(d, 572, 92, 690, 389, '02 / COMMS FEED                                      BGA / LIVE')
    # The actual BGA draws inside this dark monitor. Grid remains as fallback.
    d.rectangle((589, 137, 1245, 463), fill='#04100a')
    for x in range(589, 1245, 32): d.line((x,137,x,463), fill='#0b2115')
    for y in range(137, 463, 32): d.line((589,y,1245,y), fill='#0b2115')
    for r in (34, 70, 108): d.ellipse((917-r,299-r,917+r,299+r), outline='#215532')
    d.line((794,299,1040,299), fill=DIM)
    d.line((917,176,917,422), fill=DIM)
    text(d, 841, 286, 'AWAITING', 3)
    text(d, 846, 317, 'SIGNAL', 3)
    text(d, 607, 440, 'VIDEO UPLINK / STANDBY', 1, DIM)
    panel(d, 18, 638, 538, 66, '03 / REACTOR OUTPUT')
    d.rectangle((38, 673, 491, 693), fill='#030b07', outline='#3f5745')
    text(d, 521, 648, '%', 2)
    panel(d, 572, 493, 690, 211, '04 / COMBAT TELEMETRY')
    for x in (795,1023): d.line((x,539,x,620), fill='#643033')
    text(d, 593, 545, 'EX SCORE', 2)
    text(d, 818, 545, 'BPM', 2)
    text(d, 1045, 545, 'COMBO', 2)
    d.line((589,625,1244,625), fill=RED)
    text(d, 595, 643, 'FAST', 1)
    text(d, 810, 643, 'SLOW', 1)
    text(d, 1040, 643, 'MAX COMBO', 1)
    text(d, 596, 684, 'SYSTEM NOMINAL / NO EXTERNAL ASSETS', 1, DIM)
    return image


def main(editor_path, output):
    if output.exists(): raise ValueError('Output exists; choose a new folder.')
    inputs = output.with_name(output.name+'-inputs')
    inputs.mkdir(parents=True, exist_ok=False)
    assets = []
    def asset(name, image, bindings=(), div_x=1, cycle=0, target=None):
        image.save(inputs / (name+'.png'))
        item = dict(id=name, path=name+'.png', div_x=div_x, cycle=cycle)
        if bindings: item['bind'] = list(bindings)
        if target: item['image'] = dict(zip(('x','y','w','h'),target), layer='front')
        assets.append(item)
    def bind(obj, command, index=None):
        result = dict(object=obj, command=command)
        if index is not None: result['index'] = index
        return result
    # Keep generated artwork unchanged. LR2 scales it and samples the top cap.
    art = HERE/'armored-console.png'
    shutil.copyfile(art,inputs/'terminal.png')
    assets.append(dict(id='terminal',path='terminal.png',bind=[bind('preset_background','#SRC_IMAGE')]))
    asset('entry-cap', Image.new('RGBA',(1,1)), target=(18,0,565,116))
    starts, widths = [69]+[149+59*i for i in range(7)], [80]+[59]*7
    edits = [{'id':'preset_background','set':dict(r=255,g=255,b=255)},
             {'id':'preset_bga','set':dict(x=638,y=128,w=566,h=232)}]
    for i,(x,w) in enumerate(zip(starts,widths)):
        edits += [dict(id=f'preset_note_{i}',set=dict(x=x+2,y=452,w=w-4,h=12)),
                  dict(id=f'preset_bomb_{i}',set=dict(x=x+(w-60)//2,y=422,w=60,h=60))]
    for name, values in {
        'line_0':dict(x=69,y=452,w=493,h=1),
        'judgeline_0':dict(x=69,y=452,w=493,h=4),
        'nowjudge_0':dict(x=175,y=355,w=280,h=28),
        'nowcombo_0':dict(x=140,y=34,w=18,h=24),
        'gauge_0':dict(x=70,y=631,w=7,h=25),
        'fast':dict(x=687,y=584,w=18,h=24),
        'slow':dict(x=880,y=584,w=18,h=24),
    }.items(): edits.append(dict(id='preset_'+name,set=values))
    for name,color,commands,indices in [
        ('scratch','#dd4c46',['#SRC_NOTE'],[0]),
        ('ivory','#c4ead0',['#SRC_NOTE'],[1,3,5,7]),
        ('green','#45986e',['#SRC_NOTE'],[2,4,6]),
        ('mine','#ed873c',['#SRC_MINE'],range(8)),
        ('long-body','#226443',['#SRC_LN_BODY'],range(8)),
        ('long-start','#57db7f',['#SRC_LN_START'],range(8)),
        ('long-end','#b6f7ae',['#SRC_LN_END'],range(8)),
    ]:
        im = Image.new('RGBA',(72,12),color); d=ImageDraw.Draw(im)
        d.rectangle((0,0,71,11),outline='#0b2012',width=1)
        d.line((2,1,69,1),fill=PALE)
        d.line((2,9,69,9),fill='#25492f')
        if name=='mine': text(d,33,2,'X',1,'#482017')
        asset(name,im,[bind(f'preset_note_{i}',cmd) for i in indices for cmd in commands])
    for name,cmd,height,color in [('line','#SRC_LINE',1,'#2b5539'),('judgeline','#SRC_JUDGELINE',4,'#f06762')]:
        asset(name,Image.new('RGBA',(496,height),color),[bind('preset_'+('line_0' if name=='line' else 'judgeline_0'),cmd)])
    for i,word in enumerate(['POOR','MISS','BAD','GOOD','GREAT','PERFECT']):
        im=Image.new('RGBA',(280,28)); d=ImageDraw.Draw(im)
        font=ImageFont.truetype('C:/Windows/Fonts/bahnschrift.ttf',28)
        text(d,int((280-d.textlength(word,font=font))/2),0,word,4,GREEN if i>2 else '#f07461')
        asset('judge'+str(i),im,[bind('preset_nowjudge_0','#SRC_NOWJUDGE_1P',i)])
    digits=Image.new('RGBA',(180,24)); d=ImageDraw.Draw(digits)
    for i in range(10): text(d,i*18,1,str(i),3)
    asset('digits',digits,[bind('preset_fast','#SRC_NUMBER'),bind('preset_slow','#SRC_NUMBER')]+
          [bind('preset_nowcombo_0','#SRC_NOWCOMBO_1P',i) for i in range(3,6)],div_x=10)
    gauge=Image.new('RGBA',(36,14)); d=ImageDraw.Draw(gauge)
    for i,color in enumerate(['#43c875','#dc6150','#143020','#382020']):
        d.rectangle((9*i,0,9*i+7,13),fill=color)
        d.line((9*i,1,9*i+7,1),fill='#a3d4aa' if i<2 else '#34543b')
    asset('gauge',gauge,[bind('preset_gauge_0','#SRC_GROOVEGAUGE')],div_x=4)
    bomb=Image.new('RGBA',(360,60)); d=ImageDraw.Draw(bomb)
    for f in range(6):
        r=6+f*4
        d.ellipse((60*f+30-r,30-r,60*f+30+r,30+r),outline=(102,255,139,255-f*35),width=2)
        d.line((60*f+30,8,60*f+30,52),fill=(164,255,174,255-f*35),width=2)
    asset('bomb',bomb,[bind(f'preset_bomb_{i}','#SRC_IMAGE') for i in range(8)],div_x=6,cycle=280)
    recipe=dict(version=1,scene='play7',width=1280,height=720,title='REDLINE - Combat Terminal 7K',maker='SkinEditor experimental',objects=edits,assets=assets)
    write_json(inputs/'recipe.json',recipe)
    editor=NativeEditor(editor_path)
    base=build_skin(editor,recipe,output,preview=False,recipe_dir=inputs)
    skin=Path(base['skin'])
    columns=field_columns(editor.request('schema'))
    lines=skin.read_text(encoding='cp932').splitlines()
    for n,line in enumerate(lines):
        p=line.split(',')
        if p[0]=='#SRC_GROOVEGAUGE': p[columns[p[0]]['add_x']]='7'
        if p[0]=='#SRC_NOWJUDGE_1P': p[columns[p[0]]['noshift']]='1'
        lines[n]=','.join(p)
    manifest=json.loads((output/'assets.json').read_text(encoding='utf-8'))
    plate=next(a for a in manifest['assets'] if a['id']=='terminal')
    current=''
    for n,line in enumerate(lines):
        if line.startswith('$SE_OBJECT_ID,'): current=line.split(',')[1]
        if current=='asset_entry-cap' and line.startswith('#SRC_IMAGE,'):
            p=line.split(','); c=columns[p[0]]
            rect=(plate['gr'],plate['x']+round(18*plate['w']/1280),plate['y'],round(565*plate['w']/1280),round(116*plate['h']/720))
            for f,v in zip(('gr','x','y','w','h'),rect): p[c[f]]=str(v)
            lines[n]=','.join(p)
    digit=next(a for a in manifest['assets'] if a['id']=='digits')
    rect=','.join(str(digit[k]) for k in ('gr','x','y','w','h'))
    for identity,num,x,y,count in [('exscore',101,669,476,8),('bpm',160,897,476,4),('combo',104,1098,476,7),('maxcombo',105,1122,584,7),('gauge',107,459,628,3)]:
        lines += [f'$SE_OBJECT_NAME,{identity}',f'$SE_OBJECT_ID,redline_{identity}',
                  f'#SRC_NUMBER,0,{rect},10,1,0,0,{num},0,{count}',dst('#DST_NUMBER',x,y,12 if identity=='maxcombo' else 18,24)]
    named=[]
    for line in lines:
        p=line.split(',')
        if (p[0]=='#SRC_NOWJUDGE_1P' and int(p[1])>0) or (p[0]=='#SRC_NOWCOMBO_1P' and int(p[1])>3):
            kind='judge' if p[0]=='#SRC_NOWJUDGE_1P' else 'combo'
            named += [f'$SE_OBJECT_NAME,{kind} variant {p[1]}',f'$SE_OBJECT_ID,redline_{kind}_{p[1]}']
        named.append(line)
    lines=named
    # Rename only this invocation's newly generated preset; never install over a user skin.
    old_folder = output/'LR2files/Theme/Generated'
    new_folder = output/'LR2files/Theme/REDLINE-Combat-Terminal-7K'
    assert skin.parent.resolve() == old_folder.resolve() and not new_folder.exists()
    lines = [line.replace('LR2files\\Theme\\Generated\\', 'LR2files\\Theme\\REDLINE-Combat-Terminal-7K\\') for line in lines]
    skin.write_bytes(('\r\n'.join(lines)+'\r\n').encode('cp932'))
    old_folder.rename(new_folder)
    skin = new_folder/skin.name
    renamed = new_folder/'REDLINE-7K.lr2skin'
    skin.rename(renamed)
    skin = renamed
    def remap(value):
        if isinstance(value,str):
            return value.replace('Theme/Generated/','Theme/REDLINE-Combat-Terminal-7K/').replace('Theme\\Generated\\','Theme\\REDLINE-Combat-Terminal-7K\\').replace('skin.lr2skin','REDLINE-7K.lr2skin')
        if isinstance(value,list): return [remap(v) for v in value]
        if isinstance(value,dict): return {k:remap(v) for k,v in value.items()}
        return value
    for report in output.glob('*.json'):
        write_json(report,remap(json.loads(report.read_text(encoding='utf-8'))))
    inspection=editor.request('inspect',skin)
    ids=[o['id'] for o in inspection['objects']]
    assert len(ids)==38 and all(ids) and len(set(ids))==38
    for command in ('#SRC_NOTE','#SRC_MINE','#SRC_LN_START','#SRC_LN_BODY','#SRC_LN_END','#DST_NOTE'):
        assert sorted(int(row.split(',')[1]) for row in lines if row.startswith(command+','))==list(range(8))
    write_json(output/'objects.json',inspection)
    editor.request('render',skin,output/'preview.png')
    write_json(output/'redline-report.json',dict(skin=str(skin),objects=inspection['object_count'],native_render=True,lr2_gameplay_checked=False))
    (output/'README.md').write_text((HERE/'README.md').read_text(encoding='utf-8'),encoding='utf-8')
    print(json.dumps(dict(skin=str(skin),preview=str(output/'preview.png'))))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--editor',required=True,type=Path)
    p.add_argument('--out',required=True,type=Path)
    args=p.parse_args()
    main(args.editor,args.out.resolve())
