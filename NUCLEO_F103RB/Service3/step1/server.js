const express = require('express');
const { SerialPort } = require('serialport');
const path = require('path');

const app = express();
const HTTP_PORT = 3000;
let currentPort = null;

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

app.listen(HTTP_PORT, () => {
  console.log(`Serial Control Service running at http://localhost:${HTTP_PORT}`);
});
