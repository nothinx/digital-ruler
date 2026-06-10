// =====================================================================
//  app.js  —  Digital Ruler (MQTT.js over ws lokal)
// =====================================================================
(function () {
  "use strict";
  const CFG = window.RULER_CONFIG;

  // ---------- Elemen ----------
  const brokerStatus = document.getElementById("brokerStatus");
  const deviceSelect = document.getElementById("deviceSelect");
  const deviceOnline = document.getElementById("deviceOnline");
  const container = document.querySelector(".container");
  const readout = document.querySelector(".readout");
  const distVal = document.getElementById("distVal");
  const distUnit = document.getElementById("distUnit");
  const rulerFill = document.getElementById("rulerFill");
  const rulerMarker = document.getElementById("rulerMarker");
  const rulerScale = document.getElementById("rulerScale");
  const statLast = document.getElementById("statLast");
  const statMin = document.getElementById("statMin");
  const statMax = document.getElementById("statMax");
  const resetBtn = document.getElementById("resetBtn");

  // ---------- State ----------
  let client = null;
  let currentDevice = pad(1);
  let session = { last: null, min: null, max: null };

  function pad(n) { return String(n).padStart(2, "0"); }
  function base(dev) { return `${CFG.namespace}/ruler-${dev}`; }
  function fmt(v) { return v == null ? "—" : v.toFixed(1); }

  // ---------- Build UI ----------
  function buildDeviceOptions() {
    for (let i = 1; i <= CFG.deviceCount; i++) {
      const opt = document.createElement("option");
      opt.value = pad(i);
      opt.textContent = `Alat ${pad(i)}`;
      deviceSelect.appendChild(opt);
    }
  }

  function buildScale() {
    distUnit.textContent = CFG.unit;
    const steps = 4;
    rulerScale.innerHTML = "";
    for (let i = 0; i <= steps; i++) {
      const span = document.createElement("span");
      span.textContent = Math.round((CFG.scaleMax / steps) * i);
      rulerScale.appendChild(span);
    }
  }

  // ---------- MQTT ----------
  function connect() {
    const proto = CFG.protocol || "ws";
    const url = `${proto}://${CFG.host}:${CFG.port}${CFG.path || ""}`;
    setBroker("connecting", "Menyambung…");
    const opts = {
      clientId: "rulerweb-" + Math.random().toString(16).slice(2, 10),
      reconnectPeriod: 3000,
      clean: true,
    };
    if (CFG.username) opts.username = CFG.username;
    if (CFG.password) opts.password = CFG.password;
    client = mqtt.connect(url, opts);

    client.on("connect", () => {
      setBroker("connected", "Tersambung");
      subscribeDevice(currentDevice);
    });
    client.on("reconnect", () => setBroker("connecting", "Menyambung ulang…"));
    client.on("error", () => setBroker("error", "Error koneksi"));
    client.on("close", () => setBroker("error", "Terputus"));
    client.on("message", onMessage);
  }

  function subscribeDevice(dev) { client.subscribe(`${base(dev)}/#`, { qos: 1 }); }
  function unsubscribeDevice(dev) { client.unsubscribe(`${base(dev)}/#`); }

  function onMessage(topic, payloadBuf) {
    const prefix = base(currentDevice) + "/";
    if (!topic.startsWith(prefix)) return;
    const sub = topic.slice(prefix.length);
    const payload = payloadBuf.toString();

    if (sub === "status") {
      setDeviceOnline(payload === "online");
    } else if (sub === "distance") {
      const v = parseFloat(payload);
      if (!isNaN(v)) updateDistance(v);
    }
  }

  // ---------- UI update ----------
  function updateDistance(v) {
    session.last = v;
    session.min = session.min == null ? v : Math.min(session.min, v);
    session.max = session.max == null ? v : Math.max(session.max, v);

    distVal.textContent = fmt(v);
    statLast.textContent = fmt(session.last);
    statMin.textContent = fmt(session.min);
    statMax.textContent = fmt(session.max);

    const pct = Math.max(0, Math.min(100, (v / CFG.scaleMax) * 100));
    rulerFill.style.width = pct + "%";
    rulerMarker.style.left = pct + "%";

    readout.classList.remove("flash");
    void readout.offsetWidth;          // retrigger animasi
    readout.classList.add("flash");
  }

  function resetReadings() {
    session = { last: null, min: null, max: null };
    distVal.textContent = "—";
    statLast.textContent = statMin.textContent = statMax.textContent = "—";
    rulerFill.style.width = "0%";
    rulerMarker.style.left = "0%";
  }

  function setBroker(cls, text) {
    brokerStatus.className = "broker-status " + cls;
    brokerStatus.querySelector(".label").textContent = text;
    container.classList.toggle("disconnected", cls !== "connected");
  }

  function setDeviceOnline(online) {
    deviceOnline.className = "device-online " + (online ? "online" : "offline");
    deviceOnline.querySelector(".label").textContent = online ? "Online" : "Offline";
  }

  // ---------- Events ----------
  deviceSelect.addEventListener("change", () => {
    if (client && client.connected) unsubscribeDevice(currentDevice);
    currentDevice = deviceSelect.value;
    resetReadings();
    setDeviceOnline(false);
    if (client && client.connected) subscribeDevice(currentDevice);
  });

  resetBtn.addEventListener("click", () => {
    // Hanya reset min/maks; pertahankan nilai terakhir
    session.min = session.last;
    session.max = session.last;
    statMin.textContent = fmt(session.min);
    statMax.textContent = fmt(session.max);
  });

  // ---------- Init ----------
  buildDeviceOptions();
  buildScale();
  connect();
})();
