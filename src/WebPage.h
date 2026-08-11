// Web UI for the Train Button. One self-contained page — no external fonts,
// scripts or styles, so it also works in setup mode without internet access.
#pragma once
#include <Arduino.h>

static const char WEB_PAGE_HTML[] PROGMEM = R"HTMLPAGE(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="color-scheme" content="dark">
<title>Train Button</title>
<link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'><circle cx='16' cy='16' r='11' fill='%23ff2d55'/><circle cx='16' cy='16' r='14' fill='none' stroke='%23b794f4' stroke-width='2'/></svg>">
<style>
:root{
  --void:#06060f; --ink:#0b0716;
  --panel:rgba(26,16,37,.62); --panel-2:rgba(13,8,24,.72);
  --line:rgba(183,148,244,.16); --line-2:rgba(183,148,244,.34);
  --purple:#b794f4; --pink:#ed64a6; --cyan:#4fd1c5;
  --red:#ff4d6d; --amber:#ffb020;
  --text:#efeaf8; --muted:#9d92b8; --dim:#6b6389;
  --mono:ui-monospace,"SFMono-Regular","JetBrains Mono","Cascadia Mono",Menlo,Consolas,monospace;
  --sans:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif;
  --r:16px;
  --accent:var(--purple);
}
*{box-sizing:border-box}
html,body{margin:0;padding:0}
body{background:var(--void);color:var(--text);font-family:var(--sans);
  -webkit-font-smoothing:antialiased;min-height:100vh;overflow-x:hidden;padding-bottom:64px}
body::before{
  content:"";position:fixed;inset:0;z-index:-2;pointer-events:none;
  background:
    radial-gradient(60rem 40rem at 12% -10%, rgba(183,148,244,.20), transparent 60%),
    radial-gradient(48rem 34rem at 92% 4%, rgba(237,100,166,.16), transparent 62%),
    radial-gradient(52rem 40rem at 50% 108%, rgba(79,209,197,.12), transparent 60%),
    var(--ink);
}
body::after{
  content:"";position:fixed;inset:0;z-index:-1;pointer-events:none;opacity:.35;
  background-image:linear-gradient(rgba(183,148,244,.06) 1px,transparent 1px),
                   linear-gradient(90deg,rgba(183,148,244,.06) 1px,transparent 1px);
  background-size:64px 64px;
  -webkit-mask-image:radial-gradient(70% 55% at 50% 22%,#000,transparent 78%);
          mask-image:radial-gradient(70% 55% at 50% 22%,#000,transparent 78%);
}
.wrap{max-width:1080px;margin:0 auto;padding:0 20px}

/* ---------- top bar ---------- */
.top{display:flex;align-items:center;gap:14px;padding:22px 0 6px}
.brand{display:flex;align-items:center;gap:11px;min-width:0}
.mark{width:34px;height:34px;flex:none;border-radius:11px;position:relative;
  background:linear-gradient(140deg,var(--purple),var(--pink) 55%,var(--cyan));
  box-shadow:0 0 22px rgba(183,148,244,.45)}
.mark::after{content:"";position:absolute;inset:9px;border-radius:50%;background:#ff2d55;
  box-shadow:inset 0 -2px 4px rgba(0,0,0,.5),0 0 10px rgba(255,45,85,.7)}
.brand h1{font-size:15px;letter-spacing:.16em;text-transform:uppercase;margin:0;font-weight:700}
.brand small{display:block;font-family:var(--mono);font-size:10.5px;color:var(--dim);letter-spacing:.1em;margin-top:2px}
.top .spacer{flex:1}
.pill{display:inline-flex;align-items:center;gap:8px;font-family:var(--mono);font-size:11px;
  letter-spacing:.08em;text-transform:uppercase;color:var(--muted);
  border:1px solid var(--line);background:var(--panel-2);padding:7px 12px;border-radius:999px;
  backdrop-filter:blur(10px);white-space:nowrap}
.dot{width:7px;height:7px;border-radius:50%;background:var(--dim);flex:none}
.dot.on{background:var(--cyan);box-shadow:0 0 9px var(--cyan)}
.dot.warn{background:var(--amber);box-shadow:0 0 9px var(--amber)}
.dot.err{background:var(--red);box-shadow:0 0 9px var(--red)}
.dot.live{animation:pulse-dot 1.8s ease-in-out infinite}
@keyframes pulse-dot{0%,100%{opacity:1}50%{opacity:.35}}

/* ---------- stage ---------- */
.stage{display:grid;grid-template-columns:1fr 300px;gap:34px;align-items:center;
  padding:34px 0 30px;animation:rise .5s ease-out both}
@keyframes rise{from{opacity:0;transform:translateY(14px)}to{opacity:1;transform:none}}
.eyebrow{font-family:var(--mono);font-size:11px;letter-spacing:.24em;text-transform:uppercase;
  color:var(--accent);display:flex;align-items:center;gap:9px;margin-bottom:14px}
.eyebrow::after{content:"";height:1px;flex:1;background:linear-gradient(90deg,var(--line-2),transparent)}
.stage h2{margin:0;font-size:clamp(30px,5.2vw,50px);line-height:1.03;letter-spacing:-.035em;font-weight:800}
.stage h2 em{font-style:normal;background:linear-gradient(100deg,var(--purple),var(--pink) 48%,var(--cyan));
  -webkit-background-clip:text;background-clip:text;color:transparent}
.stage p{margin:16px 0 0;color:var(--muted);font-size:15.5px;line-height:1.6;max-width:44ch}
.stage p b{color:var(--text);font-weight:600}

.knob{position:relative;width:280px;height:280px;margin:0 auto;display:grid;place-items:center}
.knob svg{position:absolute;inset:0;width:100%;height:100%;transform:rotate(-90deg)}
.knob .track{fill:none;stroke:rgba(183,148,244,.12);stroke-width:5}
.knob .prog{fill:none;stroke:var(--accent);stroke-width:5;stroke-linecap:round;
  stroke-dasharray:653.45;stroke-dashoffset:653.45;
  filter:drop-shadow(0 0 6px var(--accent));transition:stroke-dashoffset .6s linear,stroke .3s}
.cap{width:168px;height:168px;border-radius:50%;position:relative;border:0;padding:0;cursor:pointer;
  background:radial-gradient(circle at 38% 30%,#ff6b81,#d8102f 62%,#7c0a1e);
  box-shadow:0 18px 40px rgba(0,0,0,.6),inset 0 -10px 22px rgba(0,0,0,.45),inset 0 6px 14px rgba(255,255,255,.18);
  transition:transform .12s ease;-webkit-tap-highlight-color:transparent}
.cap:active{transform:translateY(4px) scale(.985)}
.cap:focus-visible{outline:2px solid var(--cyan);outline-offset:8px}
.cap::before{content:"";position:absolute;inset:-13px;border-radius:50%;
  border:2px solid rgba(120,120,140,.35);
  background:linear-gradient(160deg,#3a3550,#15111f 55%,#26223a)}
.cap>span{position:absolute;inset:0;z-index:1;border-radius:50%;display:grid;place-items:center;
  font-family:var(--mono);font-size:11px;letter-spacing:.2em;text-transform:uppercase;
  color:rgba(255,255,255,.82);text-shadow:0 1px 3px rgba(0,0,0,.6)}
/* the halo mirrors the physical ring: data-vis = what the LED does right now */
.knob{--halo:rgba(255,60,95,.55);--period:800ms}
.knob .glow{position:absolute;width:200px;height:200px;border-radius:50%;pointer-events:none;
  box-shadow:0 0 46px 12px var(--halo),0 0 110px 30px var(--halo);
  opacity:0;transition:opacity .25s}
.knob[data-vis="solid"] .glow{opacity:1}
.knob[data-vis="blink"] .glow{opacity:1;animation:blk var(--period) steps(1,end) infinite}
.knob[data-vis="breathe"] .glow{opacity:1;animation:brt var(--period) ease-in-out infinite}
.knob[data-sem="error"]{--halo:rgba(255,45,85,.85)}
.knob[data-sem="idle"]{--halo:rgba(255,60,95,.22)}
.knob[data-sem="offline"],.knob[data-sem="setup"]{--halo:rgba(183,148,244,.35)}
@keyframes blk{0%,49%{opacity:1}50%,100%{opacity:0}}
@keyframes brt{0%,100%{opacity:.12}50%{opacity:1}}
.knob-cap{font-family:var(--mono);font-size:11px;letter-spacing:.18em;text-transform:uppercase;
  color:var(--dim);text-align:center;margin-top:14px}

/* ---------- telemetry ---------- */
.strip{display:flex;flex-wrap:wrap;gap:1px;border:1px solid var(--line);border-radius:var(--r);
  overflow:hidden;background:var(--line);margin-bottom:30px}
.strip div{flex:1 1 130px;background:var(--panel-2);padding:12px 14px;backdrop-filter:blur(8px)}
.strip dt{font-family:var(--mono);font-size:10px;letter-spacing:.16em;text-transform:uppercase;color:var(--dim);margin:0 0 5px}
.strip dd{margin:0;font-family:var(--mono);font-size:14px;color:var(--text);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}

/* ---------- tabs ---------- */
.tabs{display:flex;gap:6px;overflow-x:auto;padding:5px;margin-bottom:20px;
  border:1px solid var(--line);border-radius:999px;background:var(--panel-2);backdrop-filter:blur(10px);
  scrollbar-width:none;position:sticky;top:12px;z-index:20}
.tabs::-webkit-scrollbar{display:none}
.tabs button{flex:none;border:0;background:transparent;color:var(--muted);cursor:pointer;
  font-family:var(--mono);font-size:11.5px;letter-spacing:.14em;text-transform:uppercase;
  padding:10px 18px;border-radius:999px;transition:color .2s,background .2s}
.tabs button:hover{color:var(--text)}
.tabs button[aria-selected="true"]{color:#12091d;background:linear-gradient(100deg,var(--purple),var(--pink));
  box-shadow:0 0 18px rgba(183,148,244,.35)}
.tabs button:focus-visible{outline:2px solid var(--cyan);outline-offset:2px}

/* ---------- cards ---------- */
.card{border:1px solid var(--line);border-radius:var(--r);background:var(--panel);
  backdrop-filter:blur(14px);padding:22px;margin-bottom:16px}
.card>h3{margin:0 0 4px;font-size:17px;letter-spacing:-.01em;font-weight:700}
.card>p.hint{margin:0 0 18px;color:var(--muted);font-size:13.5px;line-height:1.55}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:14px}
.f{display:flex;flex-direction:column;gap:7px;min-width:0}
.f.wide{grid-column:1/-1}
.f>label{font-family:var(--mono);font-size:10.5px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted)}
.f .sub{font-size:12px;color:var(--dim);line-height:1.5}
.f code{font-family:var(--mono);color:var(--muted)}
input[type=text],input[type=password],input[type=number],select{
  width:100%;background:rgba(6,5,15,.66);border:1px solid var(--line);border-radius:11px;
  color:var(--text);font-family:var(--mono);font-size:14px;padding:11px 13px;transition:border-color .2s,box-shadow .2s}
input:focus,select:focus{outline:0;border-color:var(--cyan);box-shadow:0 0 0 3px rgba(79,209,197,.16)}
input::placeholder{color:#544d70}
select{appearance:none;cursor:pointer}
.inline{display:flex;gap:8px}
.inline input{flex:1}

.seg{display:flex;gap:5px;background:rgba(6,5,15,.6);border:1px solid var(--line);border-radius:12px;padding:4px;flex-wrap:wrap}
.seg label{flex:1 1 auto;position:relative;text-align:center;cursor:pointer;letter-spacing:.08em;
  font-family:var(--mono);font-size:11px;text-transform:uppercase;color:var(--muted);
  padding:9px 6px;border-radius:9px;transition:color .2s,background .2s;white-space:nowrap}
.seg input{position:absolute;opacity:0;pointer-events:none}
.seg label.on{color:#12091d;background:linear-gradient(100deg,var(--purple),var(--pink))}
.seg label:focus-within{outline:2px solid var(--cyan);outline-offset:2px}

input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:22px;background:transparent;cursor:pointer}
input[type=range]::-webkit-slider-runnable-track{height:5px;border-radius:99px;
  background:linear-gradient(90deg,var(--purple),var(--pink))}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;
  background:#fff;margin-top:-6.5px;box-shadow:0 0 12px rgba(237,100,166,.7)}
input[type=range]::-moz-range-track{height:5px;border-radius:99px;background:linear-gradient(90deg,var(--purple),var(--pink))}
input[type=range]::-moz-range-thumb{width:18px;height:18px;border:0;border-radius:50%;background:#fff;box-shadow:0 0 12px rgba(237,100,166,.7)}
.rangerow{display:flex;align-items:center;gap:12px}
.rangerow output{font-family:var(--mono);font-size:13px;color:var(--accent);min-width:52px;text-align:right}

.sw{display:flex;align-items:center;gap:11px;cursor:pointer;user-select:none}
.sw input{position:absolute;opacity:0;pointer-events:none}
.sw i{width:42px;height:24px;border-radius:99px;background:rgba(6,5,15,.8);border:1px solid var(--line);
  position:relative;flex:none;transition:background .25s,border-color .25s}
.sw i::after{content:"";position:absolute;top:2px;left:2px;width:18px;height:18px;border-radius:50%;
  background:var(--dim);transition:transform .25s,background .25s}
.sw input:checked + i{background:linear-gradient(100deg,var(--purple),var(--pink));border-color:transparent}
.sw input:checked + i::after{transform:translateX(18px);background:#fff}
.sw input:focus-visible + i{outline:2px solid var(--cyan);outline-offset:2px}
.sw span{font-size:13.5px;color:var(--text)}

.row{display:flex;gap:10px;flex-wrap:wrap;margin-top:18px;align-items:center}
.btn{border:1px solid var(--line-2);background:rgba(183,148,244,.07);color:var(--text);cursor:pointer;
  font-family:var(--sans);font-size:13.5px;font-weight:600;padding:11px 18px;border-radius:11px;
  transition:transform .12s,background .2s,border-color .2s,box-shadow .2s}
.btn:hover{background:rgba(183,148,244,.15);border-color:var(--purple)}
.btn:active{transform:translateY(1px)}
.btn:focus-visible{outline:2px solid var(--cyan);outline-offset:2px}
.btn.primary{border-color:transparent;color:#12091d;background:linear-gradient(100deg,var(--purple),var(--pink));
  box-shadow:0 6px 22px rgba(183,148,244,.28)}
.btn.primary:hover{box-shadow:0 8px 30px rgba(237,100,166,.42)}
.btn.ghost{background:transparent;border-color:var(--line);color:var(--muted)}
.btn.ghost:hover{color:var(--text);border-color:var(--line-2)}
.btn.danger{border-color:rgba(255,77,109,.4);color:#ffb3c0;background:rgba(255,77,109,.08)}
.btn.danger:hover{background:rgba(255,77,109,.18);border-color:var(--red)}
.btn[disabled]{opacity:.45;pointer-events:none}
.note{font-size:12.5px;color:var(--dim);line-height:1.55}

.nets{display:flex;flex-direction:column;gap:1px;background:var(--line);border:1px solid var(--line);
  border-radius:12px;overflow:hidden;margin-top:12px;max-height:260px;overflow-y:auto}
.nets button{display:flex;align-items:center;gap:12px;width:100%;text-align:left;border:0;cursor:pointer;
  background:rgba(13,8,24,.85);color:var(--text);padding:11px 14px;font-family:var(--mono);font-size:13px;
  transition:background .15s}
.nets button:hover{background:rgba(183,148,244,.13)}
.nets .ss{flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.nets .rs{color:var(--dim);font-size:11px}
.bars{display:flex;align-items:flex-end;gap:2px;height:13px;flex:none}
.bars i{width:3px;background:var(--dim);border-radius:1px}
.bars i.a{background:var(--cyan);box-shadow:0 0 6px rgba(79,209,197,.7)}

.toast{position:fixed;left:50%;bottom:26px;transform:translate(-50%,140%);z-index:80;
  display:flex;align-items:center;gap:10px;max-width:min(92vw,460px);
  border:1px solid var(--line-2);background:rgba(21,14,38,.95);backdrop-filter:blur(14px);
  padding:13px 18px;border-radius:13px;font-size:14px;box-shadow:0 18px 50px rgba(0,0,0,.6);
  transition:transform .35s cubic-bezier(.2,.9,.3,1.2)}
.toast.show{transform:translate(-50%,0)}
.toast.ok{border-color:rgba(79,209,197,.5)}
.toast.err{border-color:rgba(255,77,109,.55)}

.tag{font-family:var(--mono);font-size:11px;letter-spacing:.06em;
  border:1px solid var(--line);border-radius:7px;padding:8px 10px;color:var(--muted);
  overflow:hidden;text-overflow:ellipsis;white-space:nowrap}

@media (max-width:860px){
  .stage{grid-template-columns:1fr;gap:26px;padding:26px 0 22px}
  .knob{width:250px;height:250px}
  .cap{width:150px;height:150px}
  .tabs{position:static}
}
@media (prefers-reduced-motion:reduce){
  *{animation-duration:.001ms !important;animation-iteration-count:1 !important;transition-duration:.001ms !important}
}
</style>
</head>
<body>
<div class="wrap">

  <header class="top">
    <div class="brand">
      <div class="mark"></div>
      <div>
        <h1>Train Button</h1>
        <small id="hostLabel">trainbutton.local</small>
      </div>
    </div>
    <div class="spacer"></div>
    <span class="pill"><i class="dot live" id="netDot"></i><span id="netText">connecting</span></span>
  </header>

  <section class="stage">
    <div>
      <div class="eyebrow" id="eyebrow">Status</div>
      <h2 id="headline">Reading the <em>status</em></h2>
      <p id="subline">The button asks the server whether the train may run.</p>
      <div class="row">
        <button class="btn primary" id="btnPress">Run the train</button>
        <span class="note" id="pressNote">Does exactly what the physical button does.</span>
      </div>
    </div>
    <div>
      <div class="knob" id="knob" data-vis="off" data-sem="offline">
        <div class="glow"></div>
        <svg viewBox="0 0 240 240" aria-hidden="true">
          <circle class="track" cx="120" cy="120" r="104"/>
          <circle class="prog" id="ring" cx="120" cy="120" r="104"/>
        </svg>
        <button class="cap" id="capBtn" title="Run the train"><span id="capText"></span></button>
      </div>
      <div class="knob-cap" id="knobCap">LED mirror</div>
    </div>
  </section>

  <dl class="strip">
    <div><dt>IP</dt><dd id="mIp">—</dd></div>
    <div><dt>Signal</dt><dd id="mRssi">—</dd></div>
    <div><dt>Last poll</dt><dd id="mPoll">—</dd></div>
    <div><dt>Last press</dt><dd id="mLast">—</dd></div>
    <div><dt>Uptime</dt><dd id="mUp">—</dd></div>
    <div><dt>Free memory</dt><dd id="mHeap">—</dd></div>
  </dl>

  <nav class="tabs" role="tablist" id="tabs">
    <button role="tab" data-t="start" aria-selected="true">Setup</button>
    <button role="tab" data-t="api" aria-selected="false">Server</button>
    <button role="tab" data-t="led" aria-selected="false">LED</button>
    <button role="tab" data-t="hw" aria-selected="false">Hardware</button>
    <button role="tab" data-t="sys" aria-selected="false">System</button>
  </nav>

  <!-- Wi-Fi -->
  <section id="p-start">
    <div class="card">
      <h3>1 · Name and password</h3>
      <p class="hint">The name becomes the address you reach the device at. The password protects this page — set one now and the browser will ask for it from the next click on.</p>
      <div class="grid">
        <div class="f"><label for="host">Device name</label>
          <input type="text" id="host" placeholder="trainbutton" maxlength="30">
          <span class="sub">Reachable at <span id="hostPreview">trainbutton</span>.local after the next restart.</span>
        </div>
        <div class="f"><label for="uiUser">Username</label>
          <input type="text" id="uiUser" placeholder="admin" autocomplete="username">
        </div>
        <div class="f"><label for="uiPass">Password</label>
          <input type="password" id="uiPass" placeholder="leave empty to keep" autocomplete="new-password">
          <span class="sub">Optional, and plain HTTP on your own network — enough to keep housemates out, not something to expose to the internet.</span>
        </div>
      </div>
      <div class="row"><button class="btn primary" data-save="device">Save name and password</button>
        <button class="btn ghost" type="button" id="btnNoAuth">Remove password</button>
        <span class="note" id="authState"></span></div>
    </div>

    <div class="card">
      <h3>2 · Wi-Fi</h3>
      <p class="hint">Saving restarts the device so it can join the network. If the network is out of reach, it opens its own setup access point again.</p>
      <div class="grid">
        <div class="f"><label for="ssid">Network (SSID)</label>
          <div class="inline"><input type="text" id="ssid" autocomplete="off" spellcheck="false" placeholder="My-WiFi">
          <button class="btn ghost" id="btnScan" type="button">Scan</button></div>
        </div>
        <div class="f"><label for="pass">Password</label>
          <input type="password" id="pass" autocomplete="off" placeholder="leave empty to keep">
          <span class="sub">An empty field keeps the stored password.</span>
        </div>
      </div>
      <div class="nets" id="nets" hidden></div>
      <div class="row"><button class="btn primary" data-save="wifi">Save Wi-Fi</button></div>
    </div>
  </section>

  <!-- Server -->
  <section id="p-api" hidden>
    <div class="card">
      <h3>Server and token</h3>
      <p class="hint">The token identifies your account. It is stored on the device only and never sent back to this page.</p>
      <div class="grid">
        <div class="f wide"><label for="base">Server address</label>
          <input type="text" id="base" placeholder="https://data.unserioes24.de" spellcheck="false" autocomplete="off">
          <span class="sub">Base URL without a path. The device appends <code>/api/train-button/…</code> itself.</span>
        </div>
        <div class="f wide"><label for="token">Token</label>
          <div class="inline">
            <input type="password" id="token" placeholder="leave empty to keep" spellcheck="false" autocomplete="off">
            <button class="btn ghost" type="button" id="btnEye">Show</button>
          </div>
          <span class="sub" id="tokState">—</span>
        </div>
        <div class="f"><label for="poll">Poll interval</label>
          <div class="rangerow"><input type="range" id="poll" min="2" max="60" step="1"><output id="pollOut">5 s</output></div>
          <span class="sub">How often the button asks whether the train may run.</span>
        </div>
        <div class="f"><label for="timeout">Request timeout</label>
          <div class="rangerow"><input type="range" id="timeout" min="2" max="20" step="1"><output id="timeoutOut">8 s</output></div>
          <span class="sub">After this, a request counts as failed.</span>
        </div>
      </div>
      <div class="row">
        <button class="btn primary" data-save="api">Save server</button>
        <button class="btn" type="button" id="btnTestApi">Test connection</button>
      </div>
    </div>
  </section>

  <!-- LED -->
  <section id="p-led" hidden>
    <div class="card">
      <h3>While the train may run</h3>
      <p class="hint">How the button looks as long as the server gives the go-ahead.</p>
      <div class="grid">
        <div class="f wide"><label>Pattern</label>
          <div class="seg" data-seg="readyMode">
            <label><input type="radio" name="readyMode" value="1">Solid</label>
            <label><input type="radio" name="readyMode" value="2">Breathe</label>
            <label><input type="radio" name="readyMode" value="3">Blink</label>
            <label><input type="radio" name="readyMode" value="0">Off</label>
          </div>
        </div>
        <div class="f"><label for="readyBright">Brightness</label>
          <div class="rangerow"><input type="range" id="readyBright" min="1" max="100"><output id="readyBrightOut">100 %</output></div>
        </div>
        <div class="f"><label for="readyMs">Blink / breathe rate</label>
          <div class="rangerow"><input type="range" id="readyMs" min="100" max="3000" step="50"><output id="readyMsOut">800 ms</output></div>
        </div>
      </div>
      <div class="row"><button class="btn" type="button" data-test="ready">Show on device</button></div>
    </div>

    <div class="card">
      <h3>While the cooldown runs</h3>
      <p class="hint">What the button shows while nobody may press.</p>
      <div class="grid">
        <div class="f wide"><label>Pattern</label>
          <div class="seg" data-seg="idleMode">
            <label><input type="radio" name="idleMode" value="0">Off</label>
            <label><input type="radio" name="idleMode" value="1">Solid</label>
            <label><input type="radio" name="idleMode" value="2">Breathe</label>
            <label><input type="radio" name="idleMode" value="3">Blink</label>
          </div>
        </div>
        <div class="f"><label for="idleBright">Brightness</label>
          <div class="rangerow"><input type="range" id="idleBright" min="0" max="100"><output id="idleBrightOut">8 %</output></div>
        </div>
        <div class="f"><label for="idleMs">Rate</label>
          <div class="rangerow"><input type="range" id="idleMs" min="200" max="6000" step="100"><output id="idleMsOut">3000 ms</output></div>
        </div>
      </div>
      <div class="row"><button class="btn" type="button" data-test="idle">Show on device</button></div>
    </div>

    <div class="card">
      <h3>After a press</h3>
      <p class="hint">The confirmation that the train was started.</p>
      <div class="grid">
        <div class="f"><label for="pressSec">Duration</label>
          <div class="rangerow"><input type="range" id="pressSec" min="5" max="600" step="5"><output id="pressSecOut">120 s</output></div>
        </div>
        <div class="f"><label for="pressMs">Blink rate</label>
          <div class="rangerow"><input type="range" id="pressMs" min="60" max="2000" step="20"><output id="pressMsOut">500 ms</output></div>
        </div>
        <div class="f"><label for="pressBright">Brightness</label>
          <div class="rangerow"><input type="range" id="pressBright" min="1" max="100"><output id="pressBrightOut">100 %</output></div>
        </div>
      </div>
      <div class="row"><button class="btn" type="button" data-test="press">Show on device</button></div>
    </div>

    <div class="card">
      <h3>On error</h3>
      <p class="hint">Very fast blinking when someone presses while the train is locked, or when the server does not answer.</p>
      <div class="grid">
        <div class="f"><label for="errSec">Duration</label>
          <div class="rangerow"><input type="range" id="errSec" min="1" max="60" step="1"><output id="errSecOut">3 s</output></div>
        </div>
        <div class="f"><label for="errMs">Blink rate</label>
          <div class="rangerow"><input type="range" id="errMs" min="40" max="500" step="10"><output id="errMsOut">80 ms</output></div>
        </div>
        <div class="f"><label for="errBright">Brightness</label>
          <div class="rangerow"><input type="range" id="errBright" min="1" max="100"><output id="errBrightOut">100 %</output></div>
        </div>
        <div class="f wide">
          <label class="sw"><input type="checkbox" id="instant"><i></i><span>Blink immediately instead of waiting for the server</span></label>
          <span class="sub">The device already knows the lock state from its last poll, so it reacts without delay.</span>
        </div>
      </div>
      <div class="row"><button class="btn" type="button" data-test="error">Show on device</button></div>
    </div>

    <div class="row"><button class="btn primary" data-save="led">Save LED settings</button></div>
  </section>

  <!-- Hardware -->
  <section id="p-hw" hidden>
    <div class="card">
      <h3>Wiring</h3>
      <p class="hint">Default for the ESP32-S3 Mini: switch on GPIO 4 and GND, LED on GPIO 5 through a series resistor to GND.</p>
      <div class="grid">
        <div class="f"><label for="btnPin">Switch pin</label><input type="number" id="btnPin" min="0" max="48"></div>
        <div class="f"><label for="ledPin">LED pin</label><input type="number" id="ledPin" min="0" max="48"></div>
        <div class="f"><label for="debounce">Debounce</label>
          <div class="rangerow"><input type="range" id="debounce" min="10" max="200" step="5"><output id="debounceOut">40 ms</output></div>
        </div>
        <div class="f"><label for="holdSec">Hold to reopen setup</label>
          <div class="rangerow"><input type="range" id="holdSec" min="3" max="20" step="1"><output id="holdSecOut">5 s</output></div>
          <span class="sub">Hold the button this long to clear Wi-Fi and open the setup access point.</span>
        </div>
        <div class="f wide"><label class="sw"><input type="checkbox" id="btnPullup"><i></i><span>Switch shorts to GND (internal pull-up)</span></label></div>
        <div class="f wide"><label class="sw"><input type="checkbox" id="ledInvert"><i></i><span>LED is inverted (lights up on LOW)</span></label></div>
        <div class="f wide"><label class="sw"><input type="checkbox" id="rgbOn"><i></i><span>Use the board's own RGB LED as a status light</span></label>
          <span class="sub">This is the tiny WS2812 soldered on the ESP32 board, not the button's LED. Leave it off if your board does not have one.</span>
        </div>
        <div class="f"><label for="rgbPin">Board RGB LED pin</label><input type="number" id="rgbPin" min="0" max="48">
          <span class="sub">Usually 48 on the ESP32-S3 Mini, 21 on some boards.</span>
        </div>
      </div>
      <div class="row"><button class="btn primary" data-save="hw">Save hardware</button>
        <span class="note">The device re-initialises its pins right away — no restart needed.</span></div>
    </div>

    <div class="card">
      <h3>Reset button</h3>
      <p class="hint">An optional second push button. Holding it clears the Wi-Fi network and password, the token and the server address — LED patterns and pin assignments stay. Wire it exactly like the main switch: one leg to its GPIO, the other to GND.</p>
      <div class="grid">
        <div class="f wide"><label class="sw"><input type="checkbox" id="resetOn"><i></i><span>A reset button is connected</span></label></div>
        <div class="f"><label for="resetPin">Reset button pin</label><input type="number" id="resetPin" min="0" max="48">
          <span class="sub">Must differ from the switch and LED pin.</span>
        </div>
        <div class="f"><label for="wipeSec">Hold to reset</label>
          <div class="rangerow"><input type="range" id="wipeSec" min="2" max="30" step="1"><output id="wipeSecOut">8 s</output></div>
          <span class="sub">The LED strobes while the reset runs, then the device restarts into setup mode.</span>
        </div>
      </div>
      <div class="row"><button class="btn primary" data-save="reset">Save reset button</button></div>
    </div>
  </section>

  <!-- System -->
  <section id="p-sys" hidden>
    <div class="card">
      <h3>Device</h3>
      <div class="grid">
        <div class="f"><label>Firmware</label><div class="tag" id="iFw">—</div></div>
        <div class="f"><label>Chip</label><div class="tag" id="iChip">—</div></div>
        <div class="f"><label>MAC</label><div class="tag" id="iMac">—</div></div>
        <div class="f"><label>Uptime</label><div class="tag" id="iUp">—</div></div>
      </div>
      <div class="row">
        <button class="btn" type="button" id="btnReboot">Restart</button>
        <button class="btn danger" type="button" id="btnResetCreds">Reset Wi-Fi, token and server</button>
        <button class="btn danger" type="button" id="btnReset">Factory reset</button>
        <span class="note">The first keeps your LED and pin settings. A factory reset erases everything.</span>
      </div>
    </div>
  </section>
</div>

<div class="toast" id="toast"></div>

<script>
const $ = s => document.querySelector(s);
const api = async (path, opt) => {
  const r = await fetch(path, Object.assign({headers:{'Content-Type':'application/json'}}, opt||{}));
  const t = await r.text();
  let j = null; try { j = t ? JSON.parse(t) : null; } catch(e){}
  if(!r.ok) throw new Error((j && (j.error||j.message)) || ('HTTP ' + r.status));
  return j;
};
let toastTimer;
function toast(msg, kind){
  const el = $('#toast'); el.textContent = msg;
  el.className = 'toast show ' + (kind||'');
  clearTimeout(toastTimer); toastTimer = setTimeout(() => el.className = 'toast ' + (kind||''), 3400);
}
const pad = n => String(n).padStart(2,'0');
function dur(s){
  s = Math.max(0, Math.round(s));
  if(s < 60) return s + ' s';
  if(s < 3600) return Math.floor(s/60) + ':' + pad(s%60) + ' min';
  return Math.floor(s/3600) + ' h ' + pad(Math.floor((s%3600)/60)) + ' min';
}
function ago(s){
  if(s == null || s < 0) return '—';
  if(s < 60) return Math.round(s) + ' s ago';
  if(s < 3600) return Math.floor(s/60) + ' min ago';
  if(s < 86400) return Math.floor(s/3600) + ' h ago';
  return Math.floor(s/86400) + ' d ago';
}

/* ---- tabs ---- */
$('#tabs').addEventListener('click', e => {
  const b = e.target.closest('button[data-t]'); if(!b) return;
  document.querySelectorAll('#tabs button').forEach(x => x.setAttribute('aria-selected', String(x === b)));
  ['start','api','led','hw','sys'].forEach(t => $('#p-'+t).hidden = (t !== b.dataset.t));
});

/* ---- range labels ---- */
const RANGES = {
  poll:' s', timeout:' s', readyBright:' %', idleBright:' %', pressBright:' %', errBright:' %',
  readyMs:' ms', idleMs:' ms', pressMs:' ms', errMs:' ms', pressSec:' s', errSec:' s',
  debounce:' ms', holdSec:' s', wipeSec:' s'
};
Object.keys(RANGES).forEach(id => {
  const el = $('#'+id); if(!el) return;
  const out = $('#'+id+'Out');
  el._upd = () => { if(out) out.textContent = el.value + RANGES[id]; };
  el.addEventListener('input', el._upd);
});

/* ---- segmented radios (no :has() needed) ---- */
function paintSegs(){
  document.querySelectorAll('.seg label').forEach(l => {
    const i = l.querySelector('input');
    l.classList.toggle('on', !!(i && i.checked));
  });
}
document.querySelectorAll('.seg input').forEach(i => i.addEventListener('change', paintSegs));

/* ---- config ---- */
let cfg = {};
async function loadConfig(){
  cfg = await api('/api/config');
  $('#ssid').value = cfg.ssid || '';
  $('#host').value = cfg.host || 'trainbutton';
  $('#hostPreview').textContent = cfg.host || 'trainbutton';
  $('#base').value = cfg.base || '';
  $('#uiUser').value = cfg.uiUser || '';
  $('#authState').textContent = cfg.uiLocked ? 'Password protection is on.' : 'No password set.';
  $('#tokState').textContent = cfg.hasToken
    ? 'Token stored (' + (cfg.tokenHint || '••••') + ')'
    : 'No token stored yet.';
  const set = (id, v) => { const e = $('#'+id); if(!e || v === undefined) return; e.value = v; if(e._upd) e._upd(); };
  set('poll', cfg.pollSec); set('timeout', cfg.timeoutSec);
  set('readyBright', cfg.readyBright); set('readyMs', cfg.readyMs);
  set('idleBright', cfg.idleBright); set('idleMs', cfg.idleMs);
  set('pressSec', cfg.pressSec); set('pressMs', cfg.pressMs); set('pressBright', cfg.pressBright);
  set('errSec', cfg.errSec); set('errMs', cfg.errMs); set('errBright', cfg.errBright);
  set('btnPin', cfg.btnPin); set('ledPin', cfg.ledPin); set('rgbPin', cfg.rgbPin);
  set('debounce', cfg.debounceMs); set('holdSec', cfg.holdSec); set('wipeSec', cfg.wipeSec);
  set('resetPin', cfg.resetPin);
  $('#resetOn').checked = !!cfg.resetOn;
  const radio = (name, v) => {
    const r = document.querySelector('input[name="'+name+'"][value="'+v+'"]'); if(r) r.checked = true;
  };
  radio('readyMode', cfg.readyMode); radio('idleMode', cfg.idleMode);
  $('#instant').checked = !!cfg.instantError;
  $('#btnPullup').checked = !!cfg.btnPullup;
  $('#ledInvert').checked = !!cfg.ledInvert;
  $('#rgbOn').checked = !!cfg.rgbOn;
  paintSegs();
}
$('#host').addEventListener('input', e => $('#hostPreview').textContent = e.target.value || 'trainbutton');

function collect(group){
  const num = id => Number($('#'+id).value);
  const radio = n => Number((document.querySelector('input[name="'+n+'"]:checked') || {}).value || 0);
  if(group === 'device'){
    const o = { host:$('#host').value.trim(), uiUser:$('#uiUser').value.trim() };
    if($('#uiPass').value) o.uiPass = $('#uiPass').value;
    return o;
  }
  if(group === 'wifi'){
    const o = { ssid:$('#ssid').value.trim() };
    if($('#pass').value) o.pass = $('#pass').value;
    return o;
  }
  if(group === 'api'){
    const o = { base:$('#base').value.trim(), pollSec:num('poll'), timeoutSec:num('timeout') };
    if($('#token').value) o.token = $('#token').value.trim();
    return o;
  }
  if(group === 'led') return {
    readyMode:radio('readyMode'), readyBright:num('readyBright'), readyMs:num('readyMs'),
    idleMode:radio('idleMode'), idleBright:num('idleBright'), idleMs:num('idleMs'),
    pressSec:num('pressSec'), pressMs:num('pressMs'), pressBright:num('pressBright'),
    errSec:num('errSec'), errMs:num('errMs'), errBright:num('errBright'),
    instantError:$('#instant').checked
  };
  if(group === 'hw') return {
    btnPin:num('btnPin'), ledPin:num('ledPin'), rgbPin:num('rgbPin'),
    debounceMs:num('debounce'), holdSec:num('holdSec'),
    btnPullup:$('#btnPullup').checked, ledInvert:$('#ledInvert').checked, rgbOn:$('#rgbOn').checked
  };
  if(group === 'reset') return {
    resetOn:$('#resetOn').checked, resetPin:num('resetPin'), wipeSec:num('wipeSec')
  };

  return {};
}
document.addEventListener('click', async e => {
  const b = e.target.closest('[data-save]'); if(!b) return;
  b.disabled = true;
  try{
    const res = await api('/api/config', {method:'POST', body:JSON.stringify(collect(b.dataset.save))});
    $('#pass').value = ''; $('#token').value = ''; $('#uiPass').value = '';
    if(res && res.reboot){
      toast('Saved. The device restarts and joins the network.', 'ok');
    } else {
      await loadConfig();
      toast('Saved.', 'ok');
    }
  }catch(err){ toast('Not saved: ' + err.message, 'err'); }
  b.disabled = false;
});

$('#btnEye').addEventListener('click', () => {
  const t = $('#token'), show = t.type === 'password';
  t.type = show ? 'text' : 'password';
  $('#btnEye').textContent = show ? 'Hide' : 'Show';
});

/* ---- Wi-Fi scan ---- */
$('#btnScan').addEventListener('click', async () => {
  const b = $('#btnScan'); b.disabled = true; b.textContent = 'Scanning…';
  try{
    const r = await api('/api/scan');
    const box = $('#nets'); box.innerHTML = ''; box.hidden = false;
    if(!r.networks || !r.networks.length){
      const empty = document.createElement('button');
      empty.type = 'button'; empty.disabled = true;
      empty.textContent = 'No networks found';
      box.appendChild(empty);
    } else {
      r.networks.forEach(n => {
        const lv = n.rssi > -55 ? 4 : n.rssi > -67 ? 3 : n.rssi > -78 ? 2 : 1;
        const el = document.createElement('button'); el.type = 'button';
        el.innerHTML = '<span class="bars">' +
          [4,7,10,13].map((h,i) => '<i class="' + (i < lv ? 'a' : '') + '" style="height:' + h + 'px"></i>').join('') +
          '</span><span class="ss"></span><span class="rs">' + (n.lock ? '\u{1F512} ' : '') + n.rssi + ' dBm</span>';
        el.querySelector('.ss').textContent = n.ssid;
        el.addEventListener('click', () => { $('#ssid').value = n.ssid; $('#pass').focus(); box.hidden = true; });
        box.appendChild(el);
      });
    }
  }catch(err){ toast('Scan failed: ' + err.message, 'err'); }
  b.disabled = false; b.textContent = 'Scan';
});

/* ---- actions ---- */
$('#btnTestApi').addEventListener('click', async () => {
  const b = $('#btnTestApi'); b.disabled = true;
  try{
    const r = await api('/api/test-connection', {method:'POST'});
    toast(r.ok ? ('Connected as ' + (r.username || 'unknown') + '.')
               : ('Server did not answer as expected: ' + (r.error || r.code)), r.ok ? 'ok' : 'err');
  }catch(err){ toast('Test failed: ' + err.message, 'err'); }
  b.disabled = false;
});
document.addEventListener('click', async e => {
  const b = e.target.closest('[data-test]'); if(!b) return;
  try{
    await api('/api/led-test', {method:'POST', body:JSON.stringify(Object.assign({pattern:b.dataset.test}, collect('led')))});
    toast('Playing the pattern on the device.', 'ok');
  }catch(err){ toast('Not possible: ' + err.message, 'err'); }
});

// The device answers a press asynchronously: it queues the request and bumps
// press.seq once the server replied, so we watch that counter.
let pressSeq = 0, awaiting = false;
async function doPress(){
  if(awaiting) return;
  awaiting = true;
  const from = pressSeq;
  try{
    await api('/api/press', {method:'POST'});
    toast('Asking the server…');
    const t0 = Date.now();
    while(Date.now() - t0 < 15000){
      await new Promise(r => setTimeout(r, 350));
      const s = await api('/api/state');
      render(s);
      if(s.press && s.press.seq !== from){
        const p = s.press;
        if(p.result === 'ok') toast('The train is running.', 'ok');
        else if(p.result === 'locked') toast('Locked — ' + dur(p.secondsRemaining) + ' to go.', 'err');
        else toast(p.error || 'The server refused.', 'err');
        awaiting = false;
        return;
      }
    }
    toast('No answer from the server.', 'err');
  }catch(err){ toast('Not sent: ' + err.message, 'err'); }
  awaiting = false;
}
$('#btnPress').addEventListener('click', doPress);
$('#capBtn').addEventListener('click', doPress);

$('#btnReboot').addEventListener('click', async () => {
  if(!confirm('Restart the device now?')) return;
  try{ await api('/api/reboot', {method:'POST'}); toast('Restarting…', 'ok'); }
  catch(err){ toast('Failed: ' + err.message, 'err'); }
});
$('#btnResetCreds').addEventListener('click', async () => {
  if(!confirm('Clear Wi-Fi, password, token and server address? LED and pin settings stay.')) return;
  try{ await api('/api/reset-credentials', {method:'POST'}); toast('Cleared. The device restarts into setup mode.', 'ok'); }
  catch(err){ toast('Failed: ' + err.message, 'err'); }
});
$('#btnReset').addEventListener('click', async () => {
  if(!confirm('Erase all settings? Wi-Fi and token have to be entered again.')) return;
  try{ await api('/api/factory-reset', {method:'POST'}); toast('Reset. The device restarts.', 'ok'); }
  catch(err){ toast('Failed: ' + err.message, 'err'); }
});
$('#btnNoAuth').addEventListener('click', async () => {
  if(!confirm('Remove the password protection for this page?')) return;
  try{
    await api('/api/config', {method:'POST', body:JSON.stringify({clearUiAuth:true})});
    $('#uiPass').value = ''; await loadConfig(); toast('Password removed.', 'ok');
  }catch(err){ toast('Failed: ' + err.message, 'err'); }
});

/* ---- live state ---- */
const RING_LEN = 653.45;
let cooldownFrom = 0;
function setRing(frac, color){
  const r = $('#ring');
  r.style.strokeDashoffset = RING_LEN * (1 - Math.max(0, Math.min(1, frac)));
  r.style.stroke = color;
}
function render(s){
  const w = s.wifi || {}, a = s.api || {}, sy = s.sys || {}, L = s.led || {};
  pressSeq = (s.press && s.press.seq) || 0;

  $('#netDot').className = 'dot live ' + (s.setup ? 'warn' : (w.connected ? (a.ok ? 'on' : 'warn') : 'err'));
  $('#netText').textContent = s.setup ? 'setup mode'
    : !w.connected ? 'no Wi-Fi'
    : a.ok ? (w.ssid || 'connected')
    : 'server offline';
  $('#hostLabel').textContent = s.setup ? (w.ip || '192.168.4.1') : (w.host || 'trainbutton') + '.local';

  $('#mIp').textContent = w.ip || '—';
  $('#mRssi').textContent = w.connected ? (w.rssi + ' dBm') : '—';
  $('#mPoll').textContent = a.age >= 0 ? ago(a.age) + (a.latency ? ' · ' + a.latency + ' ms' : '') : '—';
  $('#mLast').textContent = a.lastUsername ? (a.lastUsername + (a.lastAgo >= 0 ? ' · ' + ago(a.lastAgo) : '')) : '—';
  $('#mUp').textContent = dur(sy.up || 0);
  $('#mHeap').textContent = sy.heap ? Math.round(sy.heap/1024) + ' kB' : '—';
  $('#iFw').textContent = sy.fw || '—';
  $('#iChip').textContent = sy.chip || '—';
  $('#iMac').textContent = sy.mac || '—';
  $('#iUp').textContent = dur(sy.up || 0);

  const knob = $('#knob');
  knob.dataset.vis = L.visual || 'off';
  knob.dataset.sem = L.state || 'offline';
  knob.style.setProperty('--period', (L.periodMs || 800) + 'ms');
  $('#knobCap').textContent = {
    ready:'LED says: ready',
    press:'confirming · ' + dur(L.remaining || 0) + ' left',
    error:'error blink',
    idle:'LED says: locked',
    offline:'no connection',
    setup:'setup mode'
  }[L.state] || 'LED mirror';
  $('#capText').textContent = L.state === 'press' ? 'go!' : '';

  let accent = 'var(--purple)', eye = 'Status', head = '', sub = '';
  if(s.setup){
    accent = 'var(--amber)'; eye = 'Setup';
    head = 'Connect the button to your <em>Wi-Fi</em>';
    sub = 'Enter your network under Wi-Fi and your token under Server. The device then restarts and joins your network.';
    setRing(0, 'var(--amber)');
  } else if(!w.connected){
    accent = 'var(--red)'; eye = 'Offline';
    head = 'No <em>Wi-Fi</em>';
    sub = 'The device keeps trying to reach "' + (w.ssid || '—') + '".';
    setRing(0, 'var(--red)');
  } else if(!a.configured){
    accent = 'var(--amber)'; eye = 'Token missing';
    head = 'No <em>token</em> stored';
    sub = 'Without a token the device cannot ask whether the train may run. Add it under Server.';
    setRing(0, 'var(--amber)');
  } else if(!a.ok){
    accent = 'var(--red)'; eye = 'Server';
    head = 'The server is <em>not answering</em>';
    sub = (a.error || ('HTTP ' + a.code)) + '. Retrying every ' + (cfg.pollSec || 5) + ' seconds.';
    setRing(0, 'var(--red)');
  } else if(a.canPress){
    accent = 'var(--cyan)'; eye = 'Ready';
    head = 'The train can <em>run</em>';
    sub = 'Signed in as <b>' + (a.username || '—') + '</b>. One press starts it.';
    setRing(1, 'var(--cyan)');
    cooldownFrom = 0;
  } else {
    accent = 'var(--pink)'; eye = 'Locked';
    head = '<em>' + dur(a.secondsUntilReady) + '</em> to go';
    sub = a.lastUsername ? ('<b>' + a.lastUsername + '</b> started the last run.') : 'The cooldown is running.';
    if(a.secondsUntilReady > cooldownFrom) cooldownFrom = a.secondsUntilReady;
    setRing(cooldownFrom ? 1 - a.secondsUntilReady / cooldownFrom : 0, 'var(--pink)');
  }
  document.documentElement.style.setProperty('--accent', accent);
  $('#eyebrow').textContent = eye;
  $('#headline').innerHTML = head;
  $('#subline').innerHTML = sub;

  const canAct = !s.setup && w.connected && a.configured;
  $('#btnPress').disabled = !canAct;
  $('#pressNote').textContent = !canAct ? 'Set up Wi-Fi and a token first.'
    : a.canPress ? 'Does exactly what the physical button does.'
    : 'The server refuses right now — a good way to see the error blink.';
}
let failCount = 0;
async function refresh(){
  if(awaiting) return;
  try{ render(await api('/api/state')); failCount = 0; }
  catch(e){ if(++failCount === 3) $('#netText').textContent = 'device unreachable'; }
}
(async () => {
  try{ await loadConfig(); }catch(e){ toast('Could not load the settings.', 'err'); }
  refresh();
  setInterval(refresh, 1000);
})();
</script>
</body>
</html>
)HTMLPAGE";
