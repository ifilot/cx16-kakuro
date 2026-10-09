/* Real browser check of the shipped WASM, ROM, assets and keyboard/mouse input.
 * npm install --prefix build/browser-tools playwright@1.64.0
 * NODE_PATH="$PWD/build/browser-tools/node_modules" node tests/check_site.cjs
 */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const {spawnSync} = require('node:child_process');
const {chromium} = require('playwright');
const root = path.resolve(__dirname, '..');
const site = path.join(root, 'build/site');
const captures = path.join(root, 'build/site-check');
fs.mkdirSync(captures, {recursive:true});
const mime = {'.html':'text/html','.js':'text/javascript','.css':'text/css','.wasm':'application/wasm','.json':'application/json','.svg':'image/svg+xml'};
const server = http.createServer((request,response) => {
  const url = new URL(request.url, 'http://localhost');
  if (!url.pathname.startsWith('/cx16-kakuro/')) {response.writeHead(404).end();return;}
  const name = path.resolve(site, '.'+url.pathname.slice('/cx16-kakuro'.length), url.pathname.endsWith('/')?'index.html':'');
  if (!name.startsWith(site+path.sep) || !fs.existsSync(name)) {response.writeHead(404).end();return;}
  response.setHeader('Content-Type', mime[path.extname(name)] || 'application/octet-stream');
  response.end(fs.readFileSync(name));
});
(async () => {
  await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
  let browser;
  try {
    browser = await chromium.launch({headless:true,executablePath:process.env.CHROMIUM_PATH || undefined,args:['--enable-unsafe-swiftshader']});
    const page = await browser.newPage({viewport:{width:1280,height:1000}});
    const errors = [], failed = [];
    page.on('pageerror', error=>errors.push(error.message));
    page.on('response', response=>{if(response.status()>=400)failed.push(response.url());});
    await page.goto(`http://127.0.0.1:${server.address().port}/cx16-kakuro/`);
    assert.equal(await page.locator('h1').textContent(),'Kakuro.');
    await page.screenshot({path:path.join(captures,'website.png'),fullPage:true});
    await page.click('#play');
    await page.waitForFunction(()=>typeof FS!=='undefined' && Module.calledRun && !document.querySelector('#canvas').hidden);
    await page.waitForTimeout(2500);
    const manifest=JSON.parse(fs.readFileSync(path.join(site,'game/manifest.json')));
    const loaded=await page.evaluate(()=>FS.readdir('/'));
    for(const file of manifest.resources) assert(loaded.includes(file),`${file} missing from emulator filesystem`);
    assert.deepEqual(await page.evaluate(()=>[canvas.width,canvas.height]),[640,480]);
    // Compare a static framebuffer region against the actual packed game assets.
    async function background(name,files,box) {
      const file=path.join(captures,`${name}.png`);
      // WebGL discards its drawing buffer; capture the composited canvas at
      // native resolution rather than using toDataURL (which returns black).
      await page.locator('#canvas').evaluate(element=>{
        element.style.width='640px';element.style.height='480px';
        const rect=element.getBoundingClientRect();
        element.style.transform=`translate(${Math.ceil(rect.x)-rect.x}px, ${Math.ceil(rect.y)-rect.y}px)`;
      });
      await page.locator('#canvas').screenshot({path:file});
      await page.locator('#canvas').evaluate(element=>{element.style.width='';element.style.height='';element.style.transform='';});
      const result=spawnSync('python3',['-c',`
import sys,numpy as np
from PIL import Image
from pathlib import Path
pixels=np.asarray(Image.open(sys.argv[1]).convert('RGB'))
data=b''.join(Path(p).read_bytes() for p in sys.argv[2:4])
raw=np.frombuffer(data,dtype=np.uint8)
indices=np.stack([(raw>>s)&3 for s in (6,4,2,0)],axis=1).reshape(480,640)
expected=np.array([[204,204,153],[136,102,102],[68,51,51],[34,34,34]],dtype=np.uint8)[indices]
x,y,w,h=map(int,sys.argv[4:])
assert np.array_equal(pixels[y:y+h,x:x+w],expected[y:y+h,x:x+w]),'Rendered scene does not match game assets'
if Path(sys.argv[1]).stem=='game':
    tiledata=np.frombuffer((Path(sys.argv[2]).parent/'GTILES.DAT').read_bytes(),dtype=np.uint8)
    colors=np.array([[0,0,0],[136,102,102],[68,51,51],[34,34,34],[204,204,153]],dtype=np.uint8)
    glyphs=[]
    for char in 'NO. 001':
        raw=tiledata[(285+ord(char)-32)*128:(286+ord(char)-32)*128]
        glyphs.append(colors[np.stack([raw>>4,raw&15],axis=1).reshape(16,16)])
    assert np.array_equal(pixels[96:112,448:560],np.concatenate(glyphs,axis=1)),'Mouse selected the wrong puzzle'

`,file,...files.map(f=>path.join(site,'game',f)),...box.map(String)],{encoding:'utf8'});
      assert.equal(result.status,0,result.stderr);
    }
    await background('start',['SPLASH0.DAT','SPLASH1.DAT'],[32,32,80,80]);
    await page.locator('#canvas').press('Enter',{delay:100});
    await page.waitForTimeout(1600);
    await background('journal',['JOURNAL0.DAT','JOURNAL1.DAT'],[16,16,40,40]);
    async function move(x,y) {
      const box=await page.locator('#canvas').boundingBox();
      await page.mouse.move(box.x+x/640*box.width,box.y+y/480*box.height);
      await page.waitForTimeout(1200);
    }
    await move(128,136);
    await page.mouse.down();await page.waitForTimeout(100);await page.mouse.up();
    await page.waitForTimeout(1600);
    await background('game',['GPLAY0.DAT','GPLAY1.DAT'],[64,434,352,28]);
    // The first puzzle's board starts at (128,160). Select a writable cell.
    await move(208,208);
    await page.locator('#canvas').press('1',{delay:100});
    await page.screenshot({path:path.join(captures,'playing.png'),fullPage:true});
    await page.click('#audio');
    assert.equal(await page.locator('#audio').getAttribute('aria-pressed'),'true');
    assert.equal(await page.evaluate(()=>Module.SDL2.audioContext.state),'running');
    await page.click('#audio');assert.equal(await page.locator('#audio').getAttribute('aria-pressed'),'false');
    await page.locator('#canvas').press('Escape',{delay:100});await page.waitForTimeout(500);
    await page.locator('#canvas').press('n',{delay:100});await page.waitForTimeout(500);
    await background('cancel',['GPLAY0.DAT','GPLAY1.DAT'],[64,434,352,28]);
    // Desktop stays at 1x; fullscreen uses the largest complete pixel blocks.
    for (const [width,height,scale] of [[1920,1080,2],[1536,864,1]]) {
      await page.setViewportSize({width,height});
      await page.waitForTimeout(150);
      let box=await page.locator('#canvas').boundingBox();
      assert.equal(box.width,640);assert.equal(box.height,480);
      await page.click('#fullscreen');
      await page.waitForFunction(()=>document.fullscreenElement?.id==='player');
      await page.waitForTimeout(200);
      box=await page.locator('#canvas').boundingBox();
      assert.equal(box.width,640*scale);assert.equal(box.height,480*scale);
      assert.equal(box.x,Math.round(box.x));assert.equal(box.y,Math.round(box.y));
      assert.deepEqual(await page.evaluate(()=>[canvas.width,canvas.height]),[640,480]);
      const file=path.join(captures,`fullscreen-${width}.png`);
      await page.locator('#canvas').screenshot({path:file});
      const result=spawnSync('python3',['-c',`
import sys,numpy as np
from PIL import Image
pixels=np.asarray(Image.open(sys.argv[1]).convert('RGB'))
k=int(sys.argv[2]);assert pixels.shape==(480*k,640*k,3)
blocks=pixels.reshape(480,k,640,k,3)
assert np.all(blocks==blocks[:,0:1,:,0:1,:]),'Fullscreen interpolates or uses uneven pixel blocks'
assert len(np.unique(pixels.reshape(-1,3),axis=0))==4,'Fullscreen introduces new colors'
`,file,String(scale)],{encoding:'utf8'});
      assert.equal(result.status,0,result.stderr);
      await page.evaluate(()=>document.exitFullscreen());
      await page.waitForTimeout(150);
    }
    await page.setViewportSize({width:390,height:900});
    assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),'Mobile page overflows');
    assert.equal((await page.locator('#canvas').boundingBox()).width,640);
    assert(await page.locator('#touch-keys').isVisible());
    await page.screenshot({path:path.join(captures,'mobile.png'),fullPage:true});

    // Fractional browser/OS scaling must still produce integer physical blocks.
    const hidpi=await browser.newContext({viewport:{width:1920,height:1080},deviceScaleFactor:1.25});
    const retina=await hidpi.newPage();
    await retina.goto(`http://127.0.0.1:${server.address().port}/cx16-kakuro/`);
    await retina.click('#play');
    await retina.waitForFunction(()=>typeof FS!=='undefined' && Module.calledRun && !document.querySelector('#canvas').hidden);
    await retina.waitForTimeout(2500);
    await retina.click('#fullscreen');await retina.waitForTimeout(250);
    const hidpiFile=path.join(captures,'fullscreen-hidpi.png');
    await retina.locator('#canvas').screenshot({path:hidpiFile});
    const hidpiResult=spawnSync('python3',['-c',`
import sys,numpy as np
from PIL import Image
pixels=np.asarray(Image.open(sys.argv[1]).convert('RGB'))
assert pixels.shape==(960,1280,3)
blocks=pixels.reshape(480,2,640,2,3)
assert np.all(blocks==blocks[:,0:1,:,0:1,:]),'Fractional DPR causes interpolation'
`,hidpiFile],{encoding:'utf8'});
    assert.equal(hidpiResult.status,0,hidpiResult.stderr);
    await hidpi.close();

    // Real touch events: pan the native view, tap a card, then use the keypad.
    const mobile=await browser.newContext({viewport:{width:390,height:844},hasTouch:true,isMobile:true});
    const phone=await mobile.newPage();
    phone.on('pageerror',error=>errors.push(error.message));
    await phone.goto(`http://127.0.0.1:${server.address().port}/cx16-kakuro/`);
    await phone.locator('#play').tap();
    await phone.waitForFunction(()=>typeof FS!=='undefined' && Module.calledRun && !document.querySelector('#canvas').hidden);
    await phone.waitForTimeout(2500);
    await phone.locator('[data-key="Enter"]').tap();await phone.waitForTimeout(1600);
    await phone.locator('#game-viewport').scrollIntoViewIfNeeded();
    await phone.locator('#game-viewport').evaluate(e=>{e.scrollLeft=150;});
    const before=await phone.locator('#game-viewport').evaluate(e=>e.scrollLeft);
    const view=await phone.locator('#game-viewport').boundingBox();
    const cdp=await mobile.newCDPSession(phone);
    const touch=(type,x)=>cdp.send('Input.dispatchTouchEvent',{type,touchPoints:type==='touchEnd'?[]:[{x,y:view.y+150}]});
    await touch('touchStart',view.x+60);
    for (let i=1;i<=8;i++) {await touch('touchMove',view.x+60+i*25);await phone.waitForTimeout(30);}
    await touch('touchEnd');await phone.waitForTimeout(400);
    assert((await phone.locator('#game-viewport').evaluate(e=>e.scrollLeft))<before,'Swipe does not pan the game');
    let box=await phone.locator('#canvas').boundingBox();
    await phone.touchscreen.tap(box.x+128,box.y+136);await phone.waitForTimeout(1900);
    // Landscape exposes the full board while retaining native pixels.
    await phone.setViewportSize({width:844,height:900});await phone.waitForTimeout(200);
    box=await phone.locator('#canvas').boundingBox();
    assert.equal(box.width,640);assert.equal(box.height,480);
    await phone.touchscreen.tap(box.x+208,box.y+208);await phone.waitForTimeout(700);
    await phone.locator('[data-key="1"]').tap();await phone.waitForTimeout(400);
    await phone.locator('#canvas').screenshot({path:path.join(captures,'touch-game.png')});
    const touchResult=spawnSync('python3',['-c',`
import sys,numpy as np
from PIL import Image
from pathlib import Path
pixels=np.asarray(Image.open(sys.argv[1]).convert('RGB'))
source=np.asarray(Image.open(sys.argv[2]).convert('RGB'))
# Selected digit 1 occupies cell kind 13; compare the quarter clear of the cursor.
assert np.array_equal(pixels[192:208,192:208],source[32:48,192:208]),'Touch keypad did not enter 1 in the selected cell'
`,path.join(captures,'touch-game.png'),path.join(root,'assets/tiles/cells.png')],{encoding:'utf8'});
    assert.equal(touchResult.status,0,touchResult.stderr);
    await mobile.close();
    assert.deepEqual(errors,[]);assert.deepEqual(failed,[]);
    console.log('PASS: WASM boot, all runtime files, start/journal/game rendering, mouse selection, audio, modal cancellation, exact desktop/fullscreen pixels and touch pan/tap/keypad under a project subpath.');
  } finally {if(browser)await browser.close();server.close();}
})().catch(error=>{console.error(error);process.exitCode=1;server.close();});
