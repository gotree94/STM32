const express = require('express');
const { SerialPort } = require('serialport');
const path = require('path');
const https = require('https');
const http = require('http');
const fs = require('fs');
const selfsigned = require('selfsigned');

const app = express();
const HTTP_PORT = 3000;
const HTTPS_PORT = 3443;
let currentPort = null;

// ─── Self-signed HTTPS certificate ──────────────────────────────
const CERT_DIR = path.join(__dirname, '.cert');
const CERT_FILE = path.join(CERT_DIR, 'cert.pem');
const KEY_FILE = path.join(CERT_DIR, 'key.pem');

async function ensureCert() {
  if (fs.existsSync(CERT_FILE) && fs.existsSync(KEY_FILE)) {
    return {
      cert: fs.readFileSync(CERT_FILE),
      key: fs.readFileSync(KEY_FILE),
    };
  }
  console.log('Generating self-signed certificate for HTTPS...');
  if (!fs.existsSync(CERT_DIR)) fs.mkdirSync(CERT_DIR, { recursive: true });
  const attrs = [{ name: 'commonName', value: 'localhost' }];
  const pem = await selfsigned.generate(attrs, { days: 365 });
  fs.writeFileSync(CERT_FILE, pem.cert);
  fs.writeFileSync(KEY_FILE, pem.private);
  return { cert: pem.cert, key: pem.private };
}

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

app.get('/api/ports', async (req, res) => {
  try {
    const ports = await SerialPort.list();
    res.json(ports.map(p => ({
      path: p.path,
      manufacturer: p.manufacturer || ''
    })));
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.post('/api/connect', (req, res) => {
  const { path: portPath } = req.body;
  if (!portPath) return res.status(400).json({ error: 'Port path is required' });

  if (currentPort && currentPort.isOpen) {
    currentPort.close();
    currentPort = null;
  }

  currentPort = new SerialPort({ path: portPath, baudRate: 9600 });

  currentPort.on('open', () => {
    res.json({ success: true, message: `Connected to ${portPath}` });
  });

  currentPort.on('error', (err) => {
    if (!res.headersSent) {
      res.status(500).json({ error: err.message });
    }
  });
});

app.post('/api/disconnect', (req, res) => {
  if (currentPort && currentPort.isOpen) {
    currentPort.close();
    currentPort = null;
    res.json({ success: true, message: 'Disconnected' });
  } else {
    res.json({ success: true, message: 'No port connected' });
  }
});

app.get('/api/status', (req, res) => {
  if (currentPort && currentPort.isOpen) {
    res.json({ connected: true, path: currentPort.path });
  } else {
    res.json({ connected: false });
  }
});

app.post('/api/command', (req, res) => {
  const { cmd } = req.body;
  if (!cmd) return res.status(400).json({ error: 'Command is required' });
  if (!currentPort || !currentPort.isOpen) {
    return res.status(400).json({ error: 'Port is not connected' });
  }

  console.log(`[TX] ${cmd}`);
  currentPort.write(cmd, (err) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json({ success: true, cmd });
  });
});

// ─── Start Servers ─────────────────────────────────────────────
(async () => {
  const creds = await ensureCert();

  http.createServer(app).listen(HTTP_PORT, () => {
    console.log(`[HTTP]  http://localhost:${HTTP_PORT}`);
  });

  https.createServer(creds, app).listen(HTTPS_PORT, () => {
    const ip = Object.values(require('os').networkInterfaces())
      .flat().find(i => i.family === 'IPv4' && !i.internal);
    const addr = ip ? ip.address : 'localhost';
    console.log(`[HTTPS] https://${addr}:${HTTPS_PORT}`);
    console.log(`iPhone에서 위 HTTPS 주소로 접속하세요 (접속 불가 시 "고급→[IP]로 이동" 또는 브라우저에 https://${addr}:${HTTPS_PORT} 직접 입력)`);
  });
})();
