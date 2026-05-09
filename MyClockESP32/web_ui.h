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
/* Przyspieszenie reakcji na dotyk dla wszystkich przycisków */
button, .day-btn, .switch {
  touch-action: manipulation;
}
/* Styl dla paska suwaka temperatury */
#tOff {
  -webkit-appearance: none; /* Ukrywa standardowy wygląd */
  width: 100%;
  height: 8px;
  background: #1a1d26;
  border-radius: 5px;
  outline: none;
  border: 1px solid #333;
  cursor: pointer;
}
/* Styl dla gałki (uchwytu) - Chrome, Safari, Edge, iOS */
#tOff::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 32px; /* Jeszcze ciut większa pod kciuk */
  height: 32px;
  cursor: pointer;
  border-radius: 50%;
  border: 3px solid #0a0c12;
  transition: background 0.2s, box-shadow 0.2s; /* Płynne przejście kolorów */
}
/* Styl dla gałki - Firefox (też powiększamy dla spójności) */
#tOff::-moz-range-thumb {
  width: 28px;
  height: 28px;
  cursor: pointer;
  border-radius: 50%;
  border: 3px solid #0a0c12;
}
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
let currentID = "----"; 
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
    lines.forEach(l => {
      // 1. Uniwersalne rozbicie linii na klucz i wartość
      let p = l.split('=');
      if (p.length < 2) return;
      let key = p[0].trim();
      let val = p[1].trim();
      // 1a. Mapowanie nazw (Zegar -> Strona WWW)
      const keyToId = {
        "tOff": "tOff",
        "rDark": "rDark",
        "rBright": "rBright",
        "nStart": "inpNStart",
        "nEnd": "inpNEnd",
        "alMel": "alMel",
        "bzVol": "bzVol",
        "alTime": "alTime"
      };
      // 1b. Automatyczne wypełnianie pól formularza
      let targetId = keyToId[key];
      if (targetId) {
        let el = document.getElementById(targetId);
        // Jeśli pole istnieje i NIE jest edytowane (ani przez flagę, ani przez focus)
        if (el && !isEditing && document.activeElement !== el) {
          el.value = val;
        }
        // Pomocniczo aktualizujemy zmienne globalne dla trybu nocnego
        if (key === "nStart" || key === "nEnd") {
          if (key === "nStart") nStart = val;
          if (key === "nEnd") nEnd = val;
          updateNightFieldsStyle();
        }
      }
      // 2. Obsługa parametrów specjalnych (Nagłówek i zmienne pomocnicze)
      if (key === "id") {
        let h = document.getElementById('hdrID');
        if(h) {
          currentID = val; // Zapamiętujemy ID na później
          h.textContent = `[ID: ${val}]`;
          // Aktualizacja tytułu karty w przeglądarce
          document.title = "[" + val + "] Clock Config";
        }
      }
      if (key === "ver") document.getElementById('fwVer').textContent = val;
      // 3. Obsługa RSSI (Ładna linia + kropki)
      if (key === "rssi") {
        let v = parseInt(val), q, c, d;
        if(v >= -50) { q="Doskonały"; c="#00ff99"; d="●●●"; }
        else if(v >= -72) { q="Dobry"; c="#6ab8ff"; d="●●○"; }
        else if(v >= -85) { q="Dostateczny"; c="#ffaa00"; d="●○○"; }
        else { q="Słaby"; c="#ff4444"; d="◦◦◦"; }
        let ri = document.getElementById('rssiIcon');
        if(ri) { ri.textContent = d; ri.style.color = c; }
        let div = document.createElement('div');
        div.className = 'statusLine';
        div.innerHTML = `📶 Sygnał: <span style="color:${c}; font-weight:bold;">${v} dBm</span> (${q})`;
        box.appendChild(div);
      }
      // 4. GŁÓWNY SWITCH LOGIKI
      switch (key) {
        case "time":
          let t = val.split(":");
          if(t.length === 3) {
            localTime.setHours(parseInt(t[0]));
            localTime.setMinutes(parseInt(t[1]));
            localTime.setSeconds(parseInt(t[2]));
            lastSyncTime = new Date();
          }
          break;
        case "date":
          let dLine = lines.find(line => line.startsWith("day="));
          let dName = dLine ? dLine.split('=')[1] : "";
          document.getElementById('dateText').textContent = `${dName}, ${val}`;
          break;
        case "tempC":
          let tv = parseFloat(val);
          document.getElementById('bigTemp').textContent = (val === "nan" || tv < -80) ? "--.- °C" : tv.toFixed(1) + " °C";
          break;
        case "brightness":
          if (firstStatus || document.getElementById('auto').checked) {
            document.getElementById('bright').value = val;
            document.getElementById('brightVal').textContent = "Aktualnie: " + val;
          }
          break;
        case "autoBrightness":
          let isA = (val === "1");
          document.getElementById('auto').checked = isA;
          let bInput = document.getElementById('bright');
          if (bInput) {
            bInput.disabled = isA; 
            // To sprawi, że suwak wizualnie "zgaśnie" (stanie się szary/półprzezroczysty)
            bInput.style.opacity = isA ? "0.3" : "1";
            bInput.style.filter = isA ? "grayscale(1)" : "none";
          }
          break;
        case "rawLDR":
          let ldrEl = document.getElementById('liveLDR');
          ldrEl.textContent = val;
          let rd = parseInt(document.getElementById('rDark').value), rb = parseInt(document.getElementById('rBright').value);
          ldrEl.style.color = (parseInt(val) >= rd || parseInt(val) <= rb) ? "#ff4444" : "#00ffaa";
          break;
        case "alDays":
          currentAlarmDays = parseInt(val);
          for(let i=0; i<7; i++) {
            let btn = document.getElementById('day-'+i);
          if(btn) btn.classList.toggle('active', !!(currentAlarmDays & (1 << i)));
          }
          break;
        case "alTime":
          let elTime = document.getElementById('alTime');
          // Sprawdzamy czy pole istnieje i czy NIE ma focusu
          if (elTime && document.activeElement !== elTime) {
            elTime.value = val;
          }
          break;
        case "alActive":
          document.getElementById('alActive').checked = (val === "1");
          if(typeof toggleAlarmIcon === "function") toggleAlarmIcon();
          break;
        case "isAlarming":
          document.getElementById('stopAl').style.display = (val === "1") ? "block" : "none";
          break;
        case "bzVol":
          document.getElementById('bzVol').value = val;
          document.getElementById('bzVolVal').textContent = `Poziom: ${val}%`;
          break;
        case "alMel":
          let melSel = document.getElementById('alMel');
          if (melSel && document.activeElement !== melSel) melSel.value = val;
          break;
        case "hChime":
          document.getElementById('hChime').checked = (val === "1");
          break;
        case "mMute":
          let isM = (val === "1");
          document.getElementById('mMute').checked = isM;
          let vIn = document.getElementById('bzVol');
          vIn.style.filter = isM ? "grayscale(1) opacity(0.3)" : "none";
          vIn.style.pointerEvents = isM ? "none" : "auto";
          break;
        case "night":
          let isN = (val === "1");
          document.getElementById('nightIcon').style.display = isN ? "inline-block" : "none";
  
          // Dynamiczna zmiana tytułu karty (opcjonalnie)
          document.title = (isN ? "🌙 [" : "🕒 [") + currentID + (isN ? "] Clock Config - Night Mode" : "] Clock Config");

          let vLabel = document.getElementById('volLabel');
          if (vLabel) {
            // Czyścimy etykietę do stanu bazowego
            let baseLabel = "🔊 Głośność";
            if (isN) {
              vLabel.innerHTML = baseLabel + " <span style='color:#ffaa00; text-shadow:0 0 8px #ff8800; font-size:13px;'> (♫ Tryb nocny " + nStart + "-" + nEnd + ")</span>";
            } else {
              vLabel.textContent = baseLabel;
            }
          }
          break;
        case "lastSync":
          document.getElementById('lastSync').textContent = "Synchronizacja: " + val;
          break;
        case "hasBuzzer":
          document.getElementById('alarmSection').style.display = (val === "1") ? "block" : "none";
          break;
        case "tOff":
          if (!isEditing) {
            document.getElementById('tOff').value = val;
            document.getElementById('tOffDisp').textContent = parseFloat(val).toFixed(1);
            updateThumbColor(val);
          }
          break;
        case "nLedOff":
          document.getElementById('nLedOff').checked = (val === "1");
          break;
      }
      // 5. WYŚWIETLANIE SUROWYCH DANYCH (Z filtrem)
      const hide = ["id", "rssi", "ver", "nStart", "nEnd", "night", "day"];
      if (!hide.includes(key)) {
        let div = document.createElement('div');
        div.className = 'statusLine';
        div.textContent = `${key}=${val}`;
        box.appendChild(div);
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
  if (document.activeElement) document.activeElement.blur(); //wymusza zakończenie edycji dowolnego pola, które jest aktualnie aktywne (nawet tego zmienianego strzałkami)
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
function updateNightFieldsStyle() {
  let s = document.getElementById('inpNStart');
  let e = document.getElementById('inpNEnd');
  if (s && e) {
    // Jeśli godziny są równe, ustaw mniejszą przezroczystość i szary kolor
    let isDisabled = (s.value === e.value);
    s.style.opacity = e.style.opacity = isDisabled ? "0.4" : "1";
    s.style.color = e.style.color = isDisabled ? "#888" : "#fff";
  }
}
function sendTempCorrection() {
  let slider = document.getElementById('tOff');
  let v = parseFloat(slider.value);
  // Krytyczna poprawka dla iOS: jeśli jesteśmy w strefie przyciągania, 
  // wymuszamy 0 na suwaku ZANIM go odczytamy do wysyłki
  if (Math.abs(v) < 0.3) {
    v = 0;
    slider.value = 0; // To fizycznie ustawia suwak na 0 w przeglądarce
    document.getElementById('tOffDisp').textContent = "0.0";
    updateThumbColor(0);
  }
  // Dla iOS wymuszamy blokadę na wypadek, gdyby oninput nie zaskoczył
  setEdit(true);
  // Wysyłamy nową wartość do ESP
  fetch(`/set?tOff=${v}`)
    .then(() => {
      console.log("Korekta wysłana: " + v);
      // Krótkie wibracje na potwierdzenie wysłania
      if(window.navigator.vibrate) navigator.vibrate([15, 30, 15]);
      markUnsaved(); // Żeby nie zapomnieć o trwałym zapisie do NVS
      // Po wysłaniu trzymamy blokadę jeszcze przez 2 sekundy, 
      // aby ESP zdążyło zaktualizować status
      setTimeout(() => { setEdit(false); }, 2000);
    })
    .catch(err => {
      setEdit(false);
      console.error("Błąd wysyłania korekty:", err);
    });
}
function updateThumbColor(val) {
  let v = parseFloat(val);
  let color = "#ffaa00"; // Złoty środek (0.0)
  
  if (v < 0) {
    // ZIMNO: Głęboki błękit
    // Zmieniona matematyka, by przy -9.0 uzyskać mocny niebieski
    let b = Math.round(255 - (Math.abs(v) * 25)); 
    if (b < 30) b = 30; // Nie pozwalamy, by był zbyt czarny
    color = `rgb(0, ${b}, 255)`;
  } else if (v > 0) {
    // CIEPŁO: Intensywna czerwień
    let g = Math.round(170 - (v * 18));
    if (g < 0) g = 0;
    color = `rgb(255, ${g}, 0)`;
  }

  // Aplikujemy kolory i efekty
  let disp = document.getElementById('tOffDisp');
  disp.style.color = color;
  // Dodajemy mocniejszy glow, gdy jesteśmy na "złotym środku"
  disp.style.textShadow = (v === 0) ? "0 0 15px #ffaa00" : `0 0 10px ${color}66`;

  // TWÓRZYMY STYL DLA GAŁKI (Dla Firefoxa i Chrome/Safari)
  let styleId = "dynamicThumbStyle";
  let styleEl = document.getElementById(styleId);
  if (!styleEl) {
    styleEl = document.createElement('style');
    styleEl.id = styleId;
    document.head.appendChild(styleEl);
  }
  // Wpisujemy reguły CSS dla obu typów gałek
  styleEl.innerHTML = `
    #tOff::-webkit-slider-thumb { background: ${color} !important; box-shadow: 0 0 20px ${color}aa !important; }
    #tOff::-moz-range-thumb { background: ${color} !important; box-shadow: 0 0 20px ${color}aa !important; }
  `;
}
// Uniwersalny "strażnik" edycji dla wszystkich pól
document.querySelectorAll('input, select').forEach(el => {
  el.addEventListener('focus', () => isEditing = true);
  el.addEventListener('blur', () => isEditing = false);
});
function updateNightLed() {
  let val = document.getElementById('nLedOff').checked ? 1 : 0;
  fetch(`/set?nLedOff=${val}`);
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
      <input type="time" id="alTime" onchange="updateAlarm(); markUnsaved()" 
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
  <select id="alMel" onchange="updateAlarm(); markUnsaved()" style="width:100%; margin-top:10px; background:#111; color:#fff; border:1px solid #444; padding:8px; border-radius:8px;">
    <option value="0">🎼Melodia: Klasyczna</option>
    <option value="1">🎼Melodia: Radosna</option>
    <option value="2">🎼Melodia: Syrena</option>
  </select>
  <label id="volLabel" for="bzVol" style="margin-top:15px; font-size:13px; color:#aaa;">🔊 Głośność</label>
  <input type="range" id="bzVol" min="0" max="100" oninput="setBuzzerVol(this.value); markUnsaved()" style="margin-top:5px;">
  <div id="bzVolVal" style="margin-top:8px; font-size:11px; color:#666;">Poziom: --%</div>
  <button id="stopAl" class="btn reset" style="display:none; background:#ff4444; color:#fff; margin-top:15px;" onclick="stopAlarm()">WYŁĄCZ ALARM</button>
  
  <div style="margin-top:20px; display:flex; justify-content:space-between; align-items:center;">
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
<!-- SYMETRYCZNA LINIA ODDZIELAJĄCA -->
<div style="margin: 25px 0; border-top: 1px solid #333;"></div>
<div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:10px;">
  <label for="bright" style="margin:0;">🔆 Jasność</label>
  <div style="display:flex; align-items:center; gap:15px;">
    <!-- Checkbox: Wygaszanie LED -->
    <div style="display:flex; align-items:center; gap:5px;">
      <input type="checkbox" id="nLedOff" onchange="updateNightLed(); markUnsaved()" style="margin:0; cursor:pointer;">
      <label for="nLedOff" style="margin:0; font-size:12px; color:#ffaa00; text-shadow:0 0 6px #ffaa0066; cursor:pointer;">🌙 WYŁ. </label>
    </div>
    <!-- Checkbox: Auto Jasność -->
    <div style="display:flex; align-items:center; gap:5px;">
      <input type="checkbox" id="auto" onchange="setAuto(); markUnsaved()" style="margin:0; cursor:pointer;">
      <label for="auto" style="margin:0; font-size:14px; color:#9fc9ff; text-shadow:0 0 6px #0044aa; cursor:pointer;">Auto</label>
    </div>
  </div>
</div>
<input type="range" id="bright" min="0" max="255" oninput="setBright(this.value)">
<div class="value" id="brightVal">Aktualnie: --</div>
<button id="saveBtn" class="btn save" onclick="save()">💾 Zapisz</button>
<details style="margin-top:10px; text-align:left; color:#6ab8ff;">
  <summary style="cursor:pointer; font-weight:bold; padding:10px;">⚙️ Zaawansowane</summary>
  <div style="padding:15px; background:#0a0c12; border-radius:12px; margin-top:5px; border:1px solid #0070ff44;">
    <!-- Korekta Temp -->
    <label for="tOff" style="font-size:14px; display:block; color:#888; margin-bottom:10px;">🌡️ Korekta Temp: <span id="tOffDisp" style="font-size:22px; font-weight:bold; margin-left:5px;">--.-</span>°C</label>
    <div style="padding: 15px 0; margin-bottom: 10px;"> <!-- Kontener dający oddech -->
      <input type="range" id="tOff" min="-9.0" max="9.0" step="0.1"
            oninput="setEdit(true); document.getElementById('tOffDisp').textContent=parseFloat(this.value).toFixed(1); updateThumbColor(this.value); if(window.navigator.vibrate)navigator.vibrate(10);"
            onchange="sendTempCorrection()"
            style="width:100%; margin:0;">
    </div>
    <!-- LDR Live View -->
    <div style="margin-bottom:15px; padding:10px; background: #1a1d26; border-radius: 8px; text-align: center; border: 1px solid #0070ff22;">
      <span style="font-size: 11px; color: #888; text-transform: uppercase;">Aktualny odczyt sensora LDR:</span>
      <div id="liveLDR" style="font-size: 20px; color: #00ffaa; font-weight: bold; text-shadow: 0 0 10px #00ffaa66;">----</div>
    </div>
    <!-- LDR Dark i Bright -->
    <div style="display:flex; justify-content:space-between; gap:15px; margin-bottom:10px;">
      <div style="flex:1;">
        <label for="rDark" style="font-size:11px; color:#666; display:block; text-align: center;">🕯️ LDR Dark (Ciemno)</label>
        <input type="number" id="rDark" oninput="markAdvUnsaved()" onchange="markAdvUnsaved()"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#6ab8ff; border:1px solid #333; padding:8px; margin-top:5px; border-radius:6px; text-align: center;">
      </div>
      <div style="flex:1;">
        <label for="rBright" style="font-size:11px; color:#666; display:block; text-align: center;">💡 LDR Bright (Jasno)</label>
        <input type="number" id="rBright" oninput="markAdvUnsaved()" onchange="markAdvUnsaved()"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#6ab8ff; border:1px solid #333; padding:8px; margin-top:5px; border-radius:6px; text-align: center;">
      </div>
    </div>
    <!-- Godziny Nocne -->
    <div style="display:flex; justify-content:space-between; gap:15px; margin-bottom:10px;">
      <div style="flex:1;">
        <label for="inpNStart" style="font-size:11px; color:#ffaa00; display:block; text-align: center;">🌙 Tryb nocny Od (h)</label>
        <input type="number" id="inpNStart" min="0" max="23" step="1" oninput="markAdvUnsaved(); updateNightFieldsStyle()" onchange="markAdvUnsaved(); updateNightFieldsStyle()"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#fff; border:1px solid #444; padding:8px; margin-top:5px; border-radius:6px; text-align: center;">
      </div>
      <div style="flex:1;">
        <label for="inpNEnd" style="font-size:11px; color:#ffaa00; display:block; text-align: center;"> ... Do (h) ☀️</label>
        <input type="number" id="inpNEnd" min="0" max="23" step="1" oninput="markAdvUnsaved(); updateNightFieldsStyle()" onchange="markAdvUnsaved(); updateNightFieldsStyle()"
               style="width:100%; box-sizing:border-box; background:#05060a; color:#fff; border:1px solid #444; padding:8px; margin-top:5px; border-radius:6px; text-align: center;">
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
    <span id="rssiIcon" style="margin-left:10px; font-size:12px; letter-spacing:1px;"></span>
  </summary>
  <div id="statusBox" style="margin-top:10px;">Ładowanie...</div>
</details>
<button class="btn reset" style="width:100%; margin-top:15px; font-size:16px;" onclick="reset()">🔀 Przywróć fabryczne</button>
<button class="btn reset" style="width:100%; margin-top:15px; font-size:16px; color:#ff4444; border:1px solid #ff444444;" onclick="reboot()">🔄 Restart Systemu</button>
</div>
</body>
</html>
)rawliteral";