// =====================================================================
//  config.js  —  Digital Ruler: pengaturan koneksi broker untuk website
//  Broker LOKAL (Mosquitto), plain WebSocket (ws).
//  PENTING: buka via http lokal (mis. http://192.168.4.180:8000), BUKAN
//  https/GitHub Pages (https tak boleh konek ws:// — diblokir browser).
// =====================================================================
window.RULER_CONFIG = {
  protocol: "ws",        // "ws" (lokal) atau "wss" (broker cloud)
  host: "192.168.4.180", // IP broker lokal (sama dgn MQTT_HOST firmware)
  port: 9001,            // listener websockets Mosquitto
  path: "",              // root
  username: "",          // broker lokal: kosong (anonim)
  password: "",

  // Samakan PERSIS dengan TOPIC_NS di firmware config.h
  namespace: "r2c-rul-6e8cb4",

  deviceCount: 10,       // alat 01..10
  scaleMax: 100,         // skala bar mistar (cm)
  unit: "cm",
};
