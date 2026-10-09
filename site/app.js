'use strict';
const canvas = document.getElementById('canvas');
const status = document.getElementById('status');
const play = document.getElementById('play');
const audio = document.getElementById('audio');
const player = document.getElementById('player');
const viewport = document.getElementById('game-viewport');
const screen = document.getElementById('screen');
let started = false;
let sound = false;
function fail(message) {
  status.textContent = 'Unable to start the game';
  const error = document.getElementById('error');
  error.textContent = `${message} Reload to try again, or download the game to play in a desktop emulator.`;
  error.hidden = false;
  play.disabled = false;
  play.textContent = 'Reload and try again';
  play.onclick = () => location.reload();
}
// The release's Emscripten runtime reads this global before it starts.
var Module = {
  canvas,
  arguments: ['-prg', 'KAKURO.PRG', '-run', '-ram', '512', '-scale', '1', '-quality', 'nearest', '-keymap', 'en-us'],
  locateFile: name => `emulator/${name}`,
  preRun: [function () {
    ENV.SDL_EMSCRIPTEN_KEYBOARD_ELEMENT = '#canvas';
    addRunDependency('kakuro-manifest');
    fetch('game/manifest.json').then(response => {
      if (!response.ok) throw new Error('The game files could not be downloaded.');
      return response.json();
    }).then(manifest => {
      for (const name of manifest.resources) {
        FS.createPreloadedFile('/', name, `game/${name}`, true, true, undefined,
          () => fail(`The file ${name} could not be downloaded.`));
      }
      removeRunDependency('kakuro-manifest');
    }).catch(error => fail(error.message));
  }],
  postRun: [function () {
    document.getElementById('cover').hidden = true;
    canvas.hidden = false;
    status.textContent = 'Game running · press Enter to select a puzzle';
    audio.disabled = false;
    document.getElementById('fullscreen').disabled = false;
    document.getElementById('restart').disabled = false;
    Module.ccall('j2c_start_audio', 'void', ['bool'], [false]);
    Module.SDL2?.audioContext?.suspend();
    document.querySelectorAll('[data-key]').forEach(button => { button.disabled = false; });
    updateDisplay();
    canvas.focus({preventScroll: true});
  }],
  print: text => console.log(text),
  printErr: text => console.warn(text),
  setStatus: text => { if (text) status.textContent = 'Loading the emulator…'; },
  onAbort: () => fail('The emulator stopped before the game could start.')
};
play.addEventListener('click', () => {
  if (started) return;
  started = true;
  play.disabled = true;
  play.textContent = 'Loading…';
  status.textContent = 'Loading the game and emulator…';
  const script = document.createElement('script');
  script.src = 'emulator/x16emu.js';
  script.onerror = () => fail('The emulator could not be downloaded.');
  document.body.append(script);
});
canvas.addEventListener('click', () => canvas.focus({preventScroll: true}));
canvas.addEventListener('keydown', event => {
  if (['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' ','Backspace'].includes(event.key)) event.preventDefault();
});
canvas.addEventListener('contextmenu', event => event.preventDefault());
canvas.addEventListener('webglcontextlost', event => {
  event.preventDefault();
  fail('The browser lost its graphics context.');
});
audio.addEventListener('click', async () => {
  try {
    if (sound) {
      Module.ccall('j2c_start_audio', 'void', ['bool'], [false]);
    } else {
      Module.ccall('j2c_start_audio', 'void', ['bool'], [true]);
      const context = Module.SDL2?.audioContext;
      if (!context) throw new Error('Audio context unavailable');
      await context.resume();
    }
    sound = !sound;
    audio.textContent = sound ? 'Sound on' : 'Sound off';
    audio.setAttribute('aria-pressed', String(sound));
    canvas.focus({preventScroll: true});
  } catch { status.textContent = 'Audio could not be enabled. Try clicking Sound again.'; }
});
document.getElementById('fullscreen').addEventListener('click', async () => {
  try {
    if (document.fullscreenElement) await document.exitFullscreen();
    else await player.requestFullscreen();
    canvas.focus({preventScroll: true});
  } catch { status.textContent = 'Fullscreen is unavailable in this browser'; }
});
document.getElementById('restart').addEventListener('click', () => location.reload());
fetch('build.json').then(response => response.json()).then(info => {
  document.getElementById('version').textContent = `v${info.version.replace(/^v/, '')} · ${info.commit.slice(0, 7)}`;
}).catch(() => {});

// The ROM begins with a centered pointer while SDL's first host coordinate is
// zero. Drive both into the corner before accepting the first browser movement.
// Wait between packets so the emulated KERNAL can apply its edge clamping.
let mouseAligned = false;
let mouseAligning = false;
async function alignMouse(x, y) {
  if (mouseAligned) return;
  if (mouseAligning) return;
  mouseAligning = true;
  const move = (clientX, clientY) => canvas.dispatchEvent(new MouseEvent('mousemove', {bubbles:true,clientX,clientY}));
  const wait = ms => new Promise(resolve => setTimeout(resolve, ms));
  const rect = canvas.getBoundingClientRect();
  // Small steps avoid overwhelming the emulated PS/2 packet queue.
  for (let step = 1; step <= 10; step++) {
    move(rect.left + rect.width * step / 10, rect.top + rect.height * step / 10);
    await wait(40);
  }
  for (let step = 9; step >= 0; step--) {
    move(rect.left + rect.width * step / 10, rect.top + rect.height * step / 10);
    await wait(40);
  }
  for (let step = 1; step <= 4; step++) {
    move(rect.left + (x - rect.left) * step / 4, rect.top + (y - rect.top) * step / 4);
    await wait(40);
  }
  mouseAligned = true;
  mouseAligning = false;
}
canvas.addEventListener('mousemove', event => {
  if (!event.isTrusted || mouseAligned) return;
  event.stopImmediatePropagation();
  alignMouse(event.clientX, event.clientY);
}, true);
for (const type of ['mousedown','mouseup']) canvas.addEventListener(type, event => {
  if (mouseAligning) event.stopImmediatePropagation();
}, true);

// Fixed native dimensions in the page; whole physical-pixel multiples in
// fullscreen. Small/touch displays retain a pannable 1x view for legibility.
function updateDisplay() {
  const fullscreen = document.fullscreenElement === player;
  const dpr = window.devicePixelRatio || 1;
  let scale = 1;
  if (fullscreen) {
    const width = viewport.clientWidth, height = viewport.clientHeight;
    const touch = matchMedia('(pointer: coarse)').matches || width < 640;
    scale = touch ? Math.max(1, Math.floor(Math.min(width / 640, height / 480)))
      : Math.max(1, Math.floor(Math.min(width * dpr / 640, height * dpr / 480))) / dpr;
  }
  const width = 640 * scale, height = 480 * scale;
  screen.style.width = canvas.style.width = `${width}px`;
  screen.style.height = canvas.style.height = `${height}px`;
  const border = fullscreen ? 0 : 12;
  screen.style.marginLeft = `${Math.max(0, Math.floor((viewport.clientWidth - width - border) / 2))}px`;
  screen.style.marginTop = `${fullscreen ? Math.max(0, Math.floor((viewport.clientHeight - height) / 2)) : 0}px`;
  screen.style.transform = '';
  // Centered blocks and text above the game can introduce half-pixel origins.
  const rect = screen.getBoundingClientRect();
  const inset = border / 2;
  const x = rect.left + inset, y = rect.top + inset;
  screen.style.transform = `translate(${Math.round(x * dpr) / dpr - x}px, ${Math.round(y * dpr) / dpr - y}px)`;
  if (!started) viewport.scrollLeft = Math.max(0, Math.floor((width + border - viewport.clientWidth) / 2));
}
let displayFrame;
function scheduleDisplay() {
  cancelAnimationFrame(displayFrame);
  displayFrame = requestAnimationFrame(updateDisplay);
}
window.addEventListener('resize', scheduleDisplay);
document.addEventListener('fullscreenchange', scheduleDisplay);
window.visualViewport?.addEventListener('resize', scheduleDisplay);
new ResizeObserver(scheduleDisplay).observe(player);
updateDisplay();

// A compact keyboard makes the existing game usable without a hardware one.
const keys = {Enter: [13, 'Enter'], Escape: [27, 'Escape'], Delete: [46, 'Delete']};
async function sendKey(key) {
  const [keyCode, code] = keys[key] || [key.charCodeAt(0), `Digit${key}`];
  canvas.focus({preventScroll: true});
  const options = {key, code, keyCode, which:keyCode, bubbles:true};
  canvas.dispatchEvent(new KeyboardEvent('keydown', options));
  await new Promise(resolve => setTimeout(resolve, 100));
  canvas.dispatchEvent(new KeyboardEvent('keyup', options));
}
document.querySelectorAll('[data-key]').forEach(button => {
  button.addEventListener('click', () => sendKey(button.dataset.key));
});

// Leave swipe gestures to the scrolling viewport. A stationary tap becomes a
// native-coordinate mouse click; prevent SDL's duplicate touch/mouse handling.
let touchStart;
let lastTouch;
for (const type of ['touchstart','touchmove','touchend','touchcancel']) {
  canvas.addEventListener(type, async event => {
    event.stopImmediatePropagation();
    if (type === 'touchstart') {
      const touch = event.touches[0];
      touchStart = event.touches.length === 1 ? {x:touch.clientX,y:touch.clientY,moved:false} : null;
    } else if (type === 'touchmove' && touchStart) {
      const touch = event.touches[0];
      if (Math.hypot(touch.clientX-touchStart.x,touch.clientY-touchStart.y)>8) touchStart.moved = true;
    } else if (type === 'touchend' && touchStart) {
      const start = touchStart; touchStart = null;
      if (start.moved || mouseAligning) return;
      event.preventDefault();
      const touch = event.changedTouches[0];
      const rect = canvas.getBoundingClientRect();
      const target = {x:(touch.clientX-rect.left)*640/rect.width,y:(touch.clientY-rect.top)*480/rect.height};
      if (!mouseAligned) await alignMouse(touch.clientX,touch.clientY);
      else {
        const from = lastTouch || target;
        const steps = Math.max(1,Math.ceil(Math.max(Math.abs(target.x-from.x),Math.abs(target.y-from.y))/64));
        for (let step=1;step<=steps;step++) {
          canvas.dispatchEvent(new MouseEvent('mousemove', {bubbles:true,
            clientX:rect.left+(from.x+(target.x-from.x)*step/steps)*rect.width/640,
            clientY:rect.top+(from.y+(target.y-from.y)*step/steps)*rect.height/480}));
          await new Promise(resolve => setTimeout(resolve,40));
        }
      }
      lastTouch = target;
      const options = {bubbles:true,clientX:touch.clientX,clientY:touch.clientY,button:0};
      canvas.dispatchEvent(new MouseEvent('mousedown',{...options,buttons:1}));
      await new Promise(resolve => setTimeout(resolve,100));
      canvas.dispatchEvent(new MouseEvent('mouseup',{...options,buttons:0}));
    } else if (type === 'touchcancel') touchStart = null;
  }, {capture:true,passive:false});
}

document.getElementById('exit-fullscreen').addEventListener('click', async () => {
  if (document.fullscreenElement) await document.exitFullscreen();
  canvas.focus({preventScroll:true});
});
