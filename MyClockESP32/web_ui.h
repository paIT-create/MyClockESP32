// --- KOD HTML W PAMIĘCI FLASH ---
const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Clock Config</title>

<style>
body{
  margin:0;padding:20px;background:#05060a;
  font-family:Segoe UI,Roboto,Arial,sans-serif;
  background:linear-gradient(135deg,#05060a,#0a0d14,#05060a);
  background-size:400% 400%;
  animation:bgmove 18s ease infinite;
  color:#d0d0d0;
}
@keyframes bgmove{
  0%{background-position:0% 50%;}
  50%{background-position:100% 50%;}
  100%{background-position:0% 50%;}
}
.card{
  background:#0f1117;padding:28px;border-radius:16px;
  max-width:500px;margin:25px auto;
  box-shadow:0 0 25px #0090ff55,0 0 60px #0050ff33,inset 0 0 20px #0030aa55;
}
h2{
  text-align:center;margin-top:0;color:#6ab8ff;font-size:26px;
  text-shadow:0 0 12px #0088ff,0 0 22px #0066ff;
}
label{
  display:block;margin-top:10px;font-weight:600;color:#9fc9ff;
  text-shadow:0 0 6px #0044aa;
}
.value{font-size:14px;color:#aaa;margin-top:6px;}
input[type=range]{
  width:100%;margin-top:12px;-webkit-appearance:none;height:6px;
  background:#222;border-radius:4px;outline:none;
  box-shadow:0 0 10px #0077ff88;
}
input[type=range]::-webkit-slider-thumb{
  -webkit-appearance:none;width:22px;height:22px;
  background:#00aaff;border-radius:50%;cursor:pointer;
  box-shadow:0 0 12px #00aaff,0 0 22px #0088ff;
}
input[type=checkbox]{transform:scale(1.5);margin-top:12px;cursor:pointer;}
.btn{
  margin-top:30px;width:100%;padding:14px;border:none;border-radius:12px;
  font-size:18px;cursor:pointer;font-weight:600;transition:0.25s;
  letter-spacing:0.5px;
}
.save{
  background:#0078ff;color:white;
  box-shadow:0 0 18px #0078ffcc,0 0 30px #0050ff88;
}
.save:hover{
  background:#0a8bff;
  box-shadow:0 0 25px #0a8bffdd,0 0 40px #0070ffaa;
}
.reset{
  background:#333;color:#ccc;margin-top:12px;
  box-shadow:0 0 12px #444;
}
.reset:hover{
  background:#444;color:white;box-shadow:0 0 18px #666;
}
.statusBox{
  margin-top:5px;padding:18px;background:#0a0c12;border-radius:12px;
  box-shadow:inset 0 0 18px #0070ffaa,inset 0 0 35px #0030aa55;
}
.titleSmall{
  color:#6ab8ff;font-size:17px;margin-bottom:12px;
  text-shadow:0 0 10px #0088ff;
}
.statusLine{
  margin:6px 0;font-size:14px;color:#c0c0c0;font-family:Consolas,monospace;
  text-shadow:0 0 6px #0040aa;
}
.bigClockBox{text-align:center;margin-top:10px;margin-bottom:25px;}
.bigClock{
  font-size:48px;font-weight:700;color:#6ab8ff;
  text-shadow:0 0 12px #0088ff,0 0 25px #0066ff,0 0 40px #0044aa;
  letter-spacing:2px;margin-bottom:10px;
}
.bigTemp{
  font-size:32px;font-weight:600;color:#ffdd88;
  text-shadow:0 0 12px #ffaa00,0 0 25px #ff8800,0 0 40px #cc6600;
}
.bigDate {
  font-size: 20px;
  font-weight: 500;
  color: #a0c4ff; /* Jasnobłękitny */
  text-shadow: 0 0 8px #0078ff, 0 0 15px #0050ff;
  margin-top: 10px;
  letter-spacing: 1px;
  font-family: 'Segoe UI', sans-serif;
}
.calendar-icon {
  margin-right: 8px;
  font-size: 18px;
  vertical-align: middle;
  opacity: 0.8;
}
#hdrID {
  margin-left: 10px;
  color: #6ab8ff;
  letter-spacing: 1px;
}
#nightIcon {
  animation: moonPulse 3s ease-in-out infinite;
}
@keyframes moonPulse {
  0%, 100% { opacity: 0.4; transform: scale(0.9); }
  50% { opacity: 1; transform: scale(1.1); }
}
@keyframes savePulse {
  0% { box-shadow: 0 0 15px #0078ffcc; transform: scale(1); }
  50% { box-shadow: 0 0 35px #0078ff, 0 0 50px #0050ffaa; transform: scale(1.02); }
  100% { box-shadow: 0 0 15px #0078ffcc; transform: scale(1); }
}
.pulse-active {
  animation: savePulse 2s infinite ease-in-out;
  border: 1px solid #6ab8ff !important;
}
@keyframes advPulse {
  0% { box-shadow: 0 0 15px #ffa600cc; transform: scale(1); }
  50% { box-shadow: 0 0 35px #ffaa00, 0 0 50px #ff880088; transform: scale(1.02); }
  100% { box-shadow: 0 0 15px #ffa600cc; transform: scale(1); }
}
.adv-pulse {
  animation: advPulse 2s infinite ease-in-out;
  border: 1px solid #ffdd88 !important;
}
#alTime:focus {
  border-color: #0088ff;
  box-shadow: 0 0 20px #0088ff, inset 0 0 10px #0088ff44;
}
/* Styl dla przełącznika (Toggle Switch) */
.switch {
  position: relative; display: inline-block;
  width: 50px; height: 26px; flex-shrink: 0;
}
.switch input { opacity: 0; width: 0; height: 0; }
.slider {
  position: absolute; cursor: pointer;
  top: 0; left: 0; right: 0; bottom: 0;
  background-color: #111; border: 1px solid #444;
  transition: .4s; border-radius: 34px;
}
.slider:before {
  position: absolute; content: "";
  height: 18px; width: 18px; left: 3px; bottom: 3px;
  background-color: #666; transition: .4s; border-radius: 50%;
}
input:checked + .slider { background-color: #0050ff33; border-color: #0070ff; box-shadow: 0 0 10px #0070ff66; }
input:checked + .slider:before { transform: translateX(24px); background-color: #6ab8ff; box-shadow: 0 0 8px #fff; }

/* Dodatkowy styl dla animacji ikony budzika */
@keyframes alarm-pulse {
  0% { transform: scale(1); filter: drop-shadow(0 0 5px #ff444466); }
  50% { transform: scale(1.1); filter: drop-shadow(0 0 15px #ff4444); }
  100% { transform: scale(1); filter: drop-shadow(0 0 5px #ff444466); }
}
.alarm-active-icon { animation: alarm-pulse 2s infinite ease-in-out; }
</style>

<script>
let hh="--", mm="--", ss="--";
let temp="--.-";
let firstStatus = true;
let debounceTimer; // Timer dla suwaka jasności
let localTime = new Date();
let lastSyncTime = 0;
let isEditing = false; // Flaga blokująca auto-odświeżanie pól podczas wpisywania
let currentAlarmDays = 127; // Domyślnie wszystkie dni
let nStart = "22";
let nEnd = "6";

// Funkcja Debounce: wysyła żądanie dopiero 150ms po zakończeniu ruchu suwakiem
function setBright(v) {
  if (document.getElementById('auto').checked) return;
  
  // Aktualizujemy etykietę natychmiast dla płynności interfejsu
  document.getElementById('brightVal').textContent = "Aktualnie: " + v;

  clearTimeout(debounceTimer);
  debounceTimer = setTimeout(() => {
    fetch('/set?bright=' + v)
      .then(() => console.log("Brightness updated:", v))
      .catch(err => console.error("Update failed", err));
  }, 150); 
}

function setAuto() {
  let a = document.getElementById('auto').checked ? 1 : 0;
  // Przy checkboxie wysyłamy od razu, bo to pojedyncze kliknięcie
  fetch('/set?auto=' + a)
    .then(() => {
      console.log("Auto brightness toggled:", a);
      // Jeśli włączono auto, suwak staje się nieaktywny
      document.getElementById('bright').disabled = (a === 1);
    });
}
    
function save(){
  // ZMIANA: Strzelamy do nowego endpointu /save, który faktycznie robi prefs.put
  fetch('/save').then(r => {
    if(r.ok) {
      alert('Ustawienia zapisane trwale w pamięci Flash');
      // wyłącz pulsowanie po udanym zapisie
      document.getElementById('saveBtn').classList.remove('pulse-active');
    } else {
      alert('Błąd zapisu');
    }
  });
}

function reset(){
  if(confirm('Czy na pewno przywrócić ustawienia fabryczne?')){
    fetch('/reset').then(()=>{
      alert('Reset zakończony. Strona zostanie odświeżona.');
      location.reload();
    });
  }
}

function reboot() {
  if (confirm('Czy na pewno zrestartować urządzenie?')) {
    fetch('/reboot').then(() => {
      alert('Zegar restartuje się...');
      setTimeout(() => location.reload(), 5000);
    });
  }
}

function updateClock() {
  // Jeśli od ostatniej synchronizacji minęło więcej niż 1s, dodaj sekundę lokalnie
  let now = new Date();
  if (now - lastSyncTime >= 1000) {
    localTime.setSeconds(localTime.getSeconds() + 1);
    lastSyncTime = now;
  }

  let hh = String(localTime.getHours()).padStart(2, '0');
  let mm = String(localTime.getMinutes()).padStart(2, '0');
  let ss = String(localTime.getSeconds()).padStart(2, '0');
  
  document.getElementById('bigClock').textContent = hh + ":" + mm + ":" + ss;
}

// Uruchom płynne odliczanie co 100ms dla idealnej płynności
setInterval(updateClock, 100);

function updateTemp(){
  document.getElementById('bigTemp').textContent = temp + " °C";
}

function stopAlarm() {
  fetch('/set?stopAlarm=1');
}

function toggleDay(day) {
  currentAlarmDays ^= (1 << day); // Przełącz bit (XOR)
  updateAlarm(); // Wyślij aktualizację do ESP
}

function updateAlarm() {
  let t = document.getElementById('alTime').value;
  let on = document.getElementById('alActive').checked ? 1 : 0;
  let mel = document.getElementById('alMel').value;
  let chime = document.getElementById('hChime').checked ? 1 : 0;
  // Wysyłamy maskę bitową dni tygodnia
  fetch(`/set?alTime=${t}&alOn=${on}&alDays=${currentAlarmDays}&alMel=${mel}&hChime=${chime}`);
}

function toggleAlarmIcon() {
  let isON = document.getElementById('alActive').checked;
  let icon = document.getElementById('alIcon');
  if(isON) icon.classList.add('alarm-active-icon');
  else icon.classList.remove('alarm-active-icon');
}

function updateMute() {
  let m = document.getElementById('mMute').checked ? 1 : 0;
  fetch(`/set?mMute=${m}`);
}

function setBuzzerVol(v) {
  document.getElementById('bzVolVal').textContent = "Poziom: " + v + "%";
  // Używamy debounce lub wysyłamy przy zmianie
  fetch(`/set?bzVol=${v}`);
}

function markUnsaved() {
  document.getElementById('saveBtn').classList.add('pulse-active');
}

function markAdvUnsaved() {
  document.getElementById('advBtn').classList.add('adv-pulse');
}

function loadStatus(){
  // Dodajemy timestamp, aby zapobiec cachowaniu odpowiedzi przez przeglądarkę
  fetch('/status?t=' + Date.now()).then(r=>r.text()).then(t=>{
    let lines=t.trim().split('\n');
    let box=document.getElementById('statusBox');
    box.innerHTML='';

    lines.forEach(l=>{
      // Pomocniczy podział linii na klucz i wartość
      let parts = l.split('=');
      if (parts.length < 2) return;
      let key = parts[0].trim();
      let val = parts[1].trim();

      // --- OBSŁUGA TRYBU NOCNEGO ---
      if(key === "nStart") nStart = val;
      if(key === "nEnd")   nEnd = val;

      if(key === "night"){
        let isNight = (val === "1");
        // Ikonka księżyca przy zegarze
        document.getElementById('nightIcon').style.display = isNight ? "inline-block" : "none";
        
        let bzVal = document.getElementById('bzVolVal');
        if (bzVal) {
          // Zawsze czyścimy tekst z poprzednich nawiasów (zapobiega puchnięciu napisu)
          let baseText = bzVal.textContent.split('(')[0].trim();
          if (isNight) {
            bzVal.innerHTML = baseText + " <span style='color:#ffaa00; text-shadow:0 0 8px #ff8800;'> (♫ Tryb nocny " + nStart + "-" + nEnd + ")</span>";
          } else {
            bzVal.textContent = baseText; // W dzień przywracamy czysty tekst
          }
        }
      }
      if(key === "nStart") {
        nStart = val;
        if(!isEditing) document.getElementById('inpNStart').value = val;
      }
      if(key === "nEnd") {
        nEnd = val;
        if(!isEditing) document.getElementById('inpNEnd').value = val;
      }

      if(l.startsWith("id=")){
        let deviceID = l.substring(3).trim();
        let hdr = document.getElementById('hdrID');
        if(hdr) hdr.textContent = "[ID: " + deviceID + "]";
      }
      let div=document.createElement('div');
      div.className='statusLine';
      div.textContent=l;
      box.appendChild(div);

      if(l.startsWith("time=")){
        let parts = l.substring(5).split(":");
        if(parts.length === 3) {
          // Synchronizacja lokalnego czasu z tym z ESP32
          localTime.setHours(parseInt(parts[0]));
          localTime.setMinutes(parseInt(parts[1]));
          localTime.setSeconds(parseInt(parts[2]));
          lastSyncTime = new Date(); // Reset licznika płynności
        }
      }

      if(l.startsWith("date=")){
        let d = l.substring(5).trim();
        // AKTUALIZUJ TYLKO JEŚLI DANE NIE SĄ PUSTE LUB KRESKAMI
        if(d.length > 5 && d !== "--.--.----") {
          let dayLine = lines.find(line => line.startsWith("day="));
          let dayName = dayLine ? dayLine.substring(4) : "";
          document.getElementById('dateText').textContent = dayName + ", " + d;
        }
      }

      if(l.startsWith("alDays=")){
        currentAlarmDays = parseInt(l.substring(7));
        for(let i=0; i<7; i++) {
          let btn = document.getElementById('day-'+i);
          if(btn) { // Sprawdzenie czy przycisk istnieje (dla wersji bez buzzera)
            if(currentAlarmDays & (1 << i)) btn.classList.add('active');
            else btn.classList.remove('active');
          }
        }
      }
      
      if(l.startsWith("mMute=")) {
        let isMuted = (l.substring(6) === "1");
        document.getElementById('mMute').checked = isMuted;
  
        // Efekt wizualny dla sekcji głośności
        let volInput = document.getElementById('bzVol');
        if(isMuted) {
          volInput.style.filter = "grayscale(1) opacity(0.3)";
          volInput.style.pointerEvents = "none"; // Blokada klikania
        } else {
          volInput.style.filter = "none";
          volInput.style.pointerEvents = "auto";
        }
      }

      if(l.startsWith("lastSync=")){
        document.getElementById('lastSync').textContent = "Synchronizacja: " + l.substring(9);
      }

      if(l.startsWith("tempC=")){
        let v = l.substring(6).trim();
        if (v === "nan" || parseFloat(v) < -80) {
          document.getElementById('bigTemp').textContent = "--.- °C";
        } else {
          document.getElementById('bigTemp').textContent = parseFloat(v).toFixed(1) + " °C";
        }
      }
    
      if(l.startsWith("brightness=")){
        let v = l.substring(11);
        const brightInput = document.getElementById('bright');
        
        // Aktualizujemy suwak tylko jeśli nie jest właśnie przesuwany przez użytkownika
        if (firstStatus || document.getElementById('auto').checked) {
          brightInput.value = v;
          document.getElementById('brightVal').textContent = "Aktualnie: " + v;
        }
      }

      if(l.startsWith("autoBrightness=")){
        let isAuto = (l.substring(15) === "1");
        document.getElementById('auto').checked = isAuto;
        document.getElementById('bright').disabled = isAuto; // Blokada suwaka
      }
      // Obsługa pól kalibracji (tylko jeśli użytkownik nie klika w nie teraz)
      if (!isEditing) {
        if(l.startsWith("tOff=")) document.getElementById('tOff').value = l.substring(5);
        if(l.startsWith("rDark=")) document.getElementById('rDark').value = l.substring(6);
        if(l.startsWith("rBright=")) document.getElementById('rBright').value = l.substring(8);
      }

      if(l.startsWith("rawLDR=")) {
        let val = l.substring(7);
        document.getElementById('liveLDR').textContent = val;
  
        // Opcjonalnie: zmiana koloru jeśli wartość zbliża się do progów
        let rd = parseInt(document.getElementById('rDark').value);
        let rb = parseInt(document.getElementById('rBright').value);
        if(val >= rd || val <= rb) {
          document.getElementById('liveLDR').style.color = "#ff4444"; // Alarm - poza zakresem
        } else {
          document.getElementById('liveLDR').style.color = "#00ffaa"; // OK
        }
      }

      // Obsługa Alarmu
      if(l.startsWith("isAlarming=")){
        let isAlarming = (l.substring(11) === "1");
        // Jeśli alarm gra, pokazujemy przycisk STOP, w przeciwnym razie ukrywamy
        document.getElementById('stopAl').style.display = isAlarming ? "block" : "none";
      }
      if(l.startsWith("alTime=")) {
        // Aktualizujemy pole czasu w panelu tylko raz (przy ładowaniu) 
        // lub gdy użytkownik nie edytuje właśnie pola
        if (!isEditing) document.getElementById('alTime').value = l.substring(7);
      }
      if(l.startsWith("alActive=")) {
        document.getElementById('alActive').checked = (l.substring(9) === "1");
        toggleAlarmIcon(); // Aktualizuje ikonę przy odświeżeniu statusu
      }
      if(l.startsWith("hasBuzzer=")){
        let hasBuzzer = (l.substring(10) === "1");
        document.getElementById('alarmSection').style.display = hasBuzzer ? "block" : "none";
      }
      if(l.startsWith("bzVol=")) {
        document.getElementById('bzVol').value = l.substring(6);
        document.getElementById('bzVolVal').textContent = "Poziom: " + l.substring(6) + "%";
      }
      // --- OBSŁUGA MELODII ("nowym sposobem" z key/val) ---
      if(key === "alMel") {
        let sel = document.getElementById('alMel');
        if (sel && document.activeElement !== sel) sel.value = val;
      }

      if(l.startsWith("hChime=")) {
        document.getElementById('hChime').checked = (l.substring(7) === "1");
      }

      // Firmware version
      if(l.startsWith("ver=")) {
        document.getElementById('fwVer').textContent = l.substring(4);
      }

    });
    firstStatus = false;
  }).catch(err => console.log("Status offline"));
}

// Obsługa flagi edycji - gdy użytkownik kliknie w pole, przestajemy nadpisywać mu tekst
function setEdit(state) {
  isEditing = state;
}

// Odświeżanie co 1 sekundę
setInterval(loadStatus, 1000);
window.onload = loadStatus;

// Uniwersalna obsługa klawisza Enter dla wszystkich pól formularza
document.addEventListener('keydown', function (e) {
  if (e.key === 'Enter') {
    let el = document.activeElement; // Sprawdzamy, w którym polu jest kursor
    if (el && (el.tagName === "INPUT")) {
      el.blur(); // "Zdejmuje" kursor z pola (to uruchamia onblur i setEdit(false))
      
      // Jeśli to pole kalibracji, od razu wysyłamy dane do zegara
      if (["tOff", "rDark", "rBright", "inpNStart", "inpNEnd"].includes(el.id)) {
        applyAdv();
      }
      
      // Jeśli to pole godziny budzika, odświeżamy ustawienia alarmu
      if (el.id === "alTime") {
        updateAlarm();
      }
    }
  }
});

// Zaktualizowana funkcja applyAdv (zdejmuje blokadę edycji po wysłaniu)
function applyAdv() {
  let to = document.getElementById('tOff').value;
  let rd = document.getElementById('rDark').value;
  let rb = document.getElementById('rBright').value;
  // Pobieramy i walidujemy godziny nocne
  let ns = parseInt(document.getElementById('inpNStart').value);
  let ne = parseInt(document.getElementById('inpNEnd').value);
  // Prosta korekta zakresu
  if (ns < 0) ns = 0; if (ns > 23) ns = 23;
  if (ne < 0) ne = 0; if (ne > 23) ne = 23;
  // Aktualizujemy pola w widoku, żeby użytkownik widział poprawkę
  document.getElementById('inpNStart').value = ns;
  document.getElementById('inpNEnd').value = ne;
  
  fetch(`/set?tOff=${to}&rDark=${rd}&rBright=${rb}&nStart=${ns}&nEnd=${ne}`)
    .then(() => {
      console.log("Parametry kalibracji i trybu nocnego wysłane");
      // KLUCZOWE: Pozwalamy skryptowi loadStatus ponownie nadpisywać pola danymi z ESP32
      isEditing = false;
      document.getElementById('advBtn').classList.remove('adv-pulse');
      markUnsaved();
    })
    .catch(err => {
      console.error("Błąd przesyłania kalibracji i trybu nocnego:", err);
      isEditing = false;
    });
}
</script>

</head>
<body>

<div class="card">
<h2>Ustawienia Zegara</h2>

<div class="bigClockBox">
  <div style="display:flex; justify-content:center; align-items:center; gap:10px;">
    <div id="nightIcon" style="font-size:24px; filter:drop-shadow(0 0 8px #ffaa00); display:none;">🌙</div>
    <div id="bigClock" class="bigClock">--:--:--</div>
  </div>
  <div id="bigTemp" class="bigTemp">--.- °C</div>
  <div id="bigDate" class="bigDate"><span class="calendar-icon">📅</span><span id="dateText">-- --- ----</span></div>
  <div id="lastSync" style="font-size:16px; color:#555; margin-top:10px;">Ostatnia synch: --:--</div>
</div>

<div id="alarmSection" style="display:none; border-top:1px solid #333; margin-top:20px; padding-top:10px;">
  
  <!-- NOWY ACTION BAR: Ikona - Godzina - Switch -->
  <div style="display:flex; justify-content:space-between; align-items:center; gap:10px; margin-bottom:15px;">
    
    <!-- Duża ikona z ID dla animacji -->
    <div id="alIcon" style="font-size:32px; transition: 0.3s;">⏰</div>

    <!-- Wybór godziny (Środek) -->
    <div style="flex-grow:1; text-align:center;">
      <input type="time" id="alTime" onfocus="setEdit(true)" onblur="setEdit(false)" onchange="updateAlarm(); markUnsaved()" 
           style="width:115px; background:#05060a; color:#fff; border:1px solid #444; padding:8px; border-radius:12px; font-size:22px; text-align:center; box-shadow:0 0 15px #0070ff66; outline:none;">
    </div>

    <!-- Przełącznik ON/OFF (Prawa) -->
    <label class="switch">
      <input type="checkbox" id="alActive" onchange="updateAlarm(); markUnsaved(); toggleAlarmIcon()">
      <span class="slider round"></span>
    </label>
  </div>

  <!-- Dni tygodnia -->
  <div style="display:flex; justify-content:space-between; margin:15px 0;">
    <!-- Dni tygodnia jako małe neonowe kafelki -->
    <style>
      .day-btn { font-size:10px; padding:5px; border:1px solid #444; border-radius:5px; cursor:pointer; background:#111; color:#555; transition:0.3s; }
      .day-btn.active { border-color:#6ab8ff; color:#6ab8ff; box-shadow:0 0 8px #0070ff; }
    </style>
    <div id="day-1" class="day-btn" onclick="toggleDay(1); markUnsaved()">Pn</div>
    <div id="day-2" class="day-btn" onclick="toggleDay(2); markUnsaved()">Wt</div>
    <div id="day-3" class="day-btn" onclick="toggleDay(3); markUnsaved()">Śr</div>
    <div id="day-4" class="day-btn" onclick="toggleDay(4); markUnsaved()">Cz</div>
    <div id="day-5" class="day-btn" onclick="toggleDay(5); markUnsaved()">Pt</div>
    <div id="day-6" class="day-btn" onclick="toggleDay(6); markUnsaved()">So</div>
    <div id="day-0" class="day-btn" onclick="toggleDay(0); markUnsaved()" style="color:#ff9f9f;">Nd</div>
  </div>

  <select id="alMel" onfocus="setEdit(true)" onblur="setEdit(false)" onchange="updateAlarm(); markUnsaved()" style="width:100%; margin-top:10px; background:#111; color:#fff; border:1px solid #444; padding:8px; border-radius:8px;">
    <option value="0">🎼Melodia: Klasyczna</option>
    <option value="1">🎼Melodia: Radosna</option>
    <option value="2">🎼Melodia: Syrena</option>
  </select>

  <label style="margin-top:15px; font-size:13px; color:#aaa;">🔊 Głośność</label>
  <input type="range" id="bzVol" min="0" max="100" oninput="setBuzzerVol(this.value); markUnsaved()" style="margin-top:5px;">
  <div id="bzVolVal" style="margin-top:8px; font-size:11px; color:#666;">Poziom: --%</div>

  <button id="stopAl" class="btn reset" style="display:none; background:#ff4444; color:#fff; margin-top:15px;" onclick="stopAlarm()">WYŁĄCZ ALARM</button>
  
  <div style="margin-top:20px; border-top:1px solid #333; padding-top:15px; display:flex; justify-content:space-between; align-items:center;">
    <!-- Master Mute -->
    <div style="display:flex; align-items:center; gap:8px;">
      <input type="checkbox" id="mMute" onchange="updateMute(); markUnsaved()" style="margin:0; cursor:pointer;">
      <label for="mMute" style="margin:0; color:#ff4444; font-size:14px; text-shadow:0 0 8px #ff4444; cursor:pointer;">🔇 Master Mute</label>
    </div>
    <!-- Chime -->
    <div style="display:flex; align-items:center; gap:8px;">
      <input type="checkbox" id="hChime" onchange="updateAlarm(); markUnsaved()" style="margin:0; cursor:pointer;">
      <label for="hChime" style="margin:0; color:#9fc9ff; font-size:14px; text-shadow:0 0 8px #0070ff; cursor:pointer;">🔔 Chime</label>
    </div>
  </div>
</div>

<div style="display:flex; justify-content:space-between; align-items:center; margin-top:25px;">
  <label style="margin:0;">🔆 Jasność</label>
  <div style="display:flex; align-items:center; gap:8px;">
    <input type="checkbox" id="auto" onchange="setAuto(); markUnsaved()" style="margin:0; cursor:pointer;">
    <label for="auto" style="margin:0; font-size:14px; color:#9fc9ff; text-shadow:0 0 6px #0044aa; cursor:pointer;">Auto</label>
  </div>
</div>
<input type="range" id="bright" min="0" max="255" oninput="setBright(this.value)">
<div class="value" id="brightVal">Aktualnie: --</div>

<button id="saveBtn" class="btn save" onclick="save()">💾 Zapisz</button>
<details style="margin-top:10px; text-align:left; color:#6ab8ff;">
  <summary style="cursor:pointer; font-weight:bold; padding:10px;">⚙️ Zaawansowane</summary>
  <div style="padding:15px; background:#0a0c12; border-radius:12px; margin-top:5px; border:1px solid #0070ff44;">
    <!-- Korekta Temp -->
    <label style="font-size:13px; display:block;">🌡️ Korekta Temp (°C)</label>
    <input type="number" id="tOff" oninput="markAdvUnsaved()" step="0.1" onfocus="setEdit(true)" onblur="setEdit(false)" 
           style="width:70px; background:#05060a; color:#ffdd88; border:1px solid #333; padding:8px; margin-top:5px; margin-bottom:10px; border-radius:6px; display:block;">
    <!-- LDR Live View -->
    <div style="margin-bottom:15px; padding:10px; background: #1a1d26; border-radius: 8px; text-align: center; border: 1px solid #0070ff22;">
      <span style="font-size: 11px; color: #888; text-transform: uppercase;">Aktualny odczyt sensora LDR:</span>
      <div id="liveLDR" style="font-size: 20px; color: #00ffaa; font-weight: bold; text-shadow: 0 0 10px #00ffaa66;">----</div>
    </div>
    <!-- LDR Dark i Bright -->
    <div style="display:flex; justify-content:space-between; gap:15px; margin-bottom:10px;">
      <div style="flex:1;">
        <label style="font-size:11px; color:#666; display:block;">🕯️ LDR Dark (Ciemno)</label>
        <input type="number" id="rDark" oninput="markAdvUnsaved()" onfocus="setEdit(true)" onblur="setEdit(false)"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#6ab8ff; border:1px solid #333; padding:8px; margin-top:5px; border-radius:6px;">
      </div>
      <div style="flex:1;">
        <label style="font-size:11px; color:#666; display:block;">💡 LDR Bright (Jasno)</label>
        <input type="number" id="rBright" oninput="markAdvUnsaved()" onfocus="setEdit(true)" onblur="setEdit(false)"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#6ab8ff; border:1px solid #333; padding:8px; margin-top:5px; border-radius:6px;">
      </div>
    </div>
    <!-- Godziny Nocne -->
    <div style="display:flex; justify-content:space-between; gap:15px; margin-bottom:10px;">
      <div style="flex:1;">
        <label style="font-size:11px; color:#ffaa00; display:block;">🌙 Cisza Od (h)</label>
        <input type="number" id="inpNStart" min="0" max="23" step="1" onfocus="setEdit(true)" onblur="setEdit(false)" oninput="markAdvUnsaved()" 
               style="width:100%; box-sizing:border-box; background:#05060a; color:#fff; border:1px solid #444; padding:8px; margin-top:5px; border-radius:6px;">
      </div>
      <div style="flex:1;">
        <label style="font-size:11px; color:#ffaa00; display:block;">☀️ Cisza Do (h)</label>
        <input type="number" id="inpNEnd" min="0" max="23" step="1" onfocus="setEdit(true)" onblur="setEdit(false)" oninput="markAdvUnsaved()" 
               style="width:100%; box-sizing:border-box; background:#05060a; color:#fff; border:1px solid #444; padding:8px; margin-top:5px; border-radius:6px;">
      </div>
    </div>
    <button id="advBtn" class="btn save" style="width:100%; padding:10px; font-size:14px; margin-top:10px;" onclick="applyAdv()">⚡ Zastosuj zmiany</button>
    <div style="font-size:10px; color:#444; margin-top:12px; text-align:center; opacity:0.7;">Zmiany będą aktywne TYLKO do restartu, chyba że klikniesz główny przycisk ZAPISZ.</div>
  </div>
</details>

<button class="btn reset" style="width:100%; margin-top:10px; font-size:16px; background:#222; border:1px solid #0070ff44;" onclick="location.href='/_ac'">🌐 Portal WiFi (AutoConnect)</button>

<div style="font-size:10px; color:#333; text-align:center; margin-top:15px; letter-spacing:1px;">
  MyClock ESP32 | <span id="fwVer">v1.x</span>
</div>
<details class="statusBox" style="cursor:pointer;">
  <summary class="titleSmall" style="outline:none; list-style:none; display:flex; align-items:center;">
    📊 Status <span id="hdrID" style="margin-left:10px; opacity:0.8; font-size:17px; font-family:monospace; color:#6ab8ff;">[ID: ----]</span>
  </summary>
  <div id="statusBox" style="margin-top:10px;">Ładowanie...</div>
</details>
<button class="btn reset" style="width:100%; margin-top:15px; font-size:16px;" onclick="reset()">🔀 Przywróć fabryczne</button>
<button class="btn reset" style="width:100%; margin-top:15px; font-size:16px; color:#ff4444; border:1px solid #ff444444;" onclick="reboot()">🔄 Restart Systemu</button>

</div>
</body>
</html>
)rawliteral";
