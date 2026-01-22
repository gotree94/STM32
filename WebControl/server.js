/**
 * STM32 RC Car Web Controller - Node.js Version
 * 
 * 설치: npm install express serialport
 * 실행: node server.js
 * 접속: http://localhost:3000
 */

const express = require('express');
const { SerialPort } = require('serialport');
const path = require('path');

const app = express();
const PORT = 3000;
const BAUD_RATE = 115200;

// 상태 관리
let serialPort = null;
let currentStatus = {
    direction: 'STOP',
    speed: 7,
    connected: false,
    port: ''
};

// ===== 시리얼 포트 함수 =====

// 사용 가능한 포트 목록
async function listPorts() {
    try {
        const ports = await SerialPort.list();
        return ports;
    } catch (err) {
        console.error('포트 목록 조회 실패:', err);
        return [];
    }
}

// 시리얼 포트 자동 감지 및 연결
async function initSerial() {
    const ports = await listPorts();
    
    if (ports.length === 0) {
        console.log('⚠️  사용 가능한 시리얼 포트가 없습니다.');
        return false;
    }
    
    console.log('\n📡 사용 가능한 포트:');
    ports.forEach(p => {
        console.log(`   - ${p.path}: ${p.manufacturer || p.friendlyName || 'Unknown'}`);
    });
    
    // 적합한 포트 찾기 (STM32, USB-Serial 등)
    let targetPort = ports.find(p => {
        const info = (p.manufacturer || '' + p.friendlyName || '').toLowerCase();
        return info.includes('serial') || info.includes('uart') || 
               info.includes('stm') || info.includes('st-link') ||
               info.includes('ch340') || info.includes('cp210') || 
               info.includes('ftdi') || info.includes('usb');
    });
    
    if (!targetPort) {
        targetPort = ports[0]; // 첫 번째 포트 사용
    }
    
    try {
        serialPort = new SerialPort({
            path: targetPort.path,
            baudRate: BAUD_RATE
        });
        
        serialPort.on('open', () => {
            console.log(`✅ 시리얼 연결 성공: ${targetPort.path}`);
            currentStatus.connected = true;
            currentStatus.port = targetPort.path;
        });
        
        serialPort.on('error', (err) => {
            console.error('시리얼 오류:', err.message);
            currentStatus.connected = false;
        });
        
        serialPort.on('close', () => {
            console.log('시리얼 연결 종료');
            currentStatus.connected = false;
        });
        
        // STM32로부터 데이터 수신 (디버그용)
        serialPort.on('data', (data) => {
            console.log('STM32:', data.toString().trim());
        });
        
        return true;
    } catch (err) {
        console.error('❌ 시리얼 연결 실패:', err.message);
        return false;
    }
}

// 명령 전송
function sendCommand(cmd) {
    if (!serialPort || !serialPort.isOpen) {
        console.log('시리얼 포트가 열려있지 않습니다.');
        currentStatus.connected = false;
        return false;
    }
    
    try {
        serialPort.write(cmd);
        currentStatus.connected = true;
        
        // 상태 업데이트
        const directions = {
            'W': 'FORWARD', 'w': 'FORWARD',
            'S': 'BACKWARD', 's': 'BACKWARD',
            'A': 'LEFT', 'a': 'LEFT',
            'D': 'RIGHT', 'd': 'RIGHT',
            'X': 'STOP', 'x': 'STOP'
        };
        
        if (directions[cmd]) {
            currentStatus.direction = directions[cmd];
        } else if (cmd >= '0' && cmd <= '9') {
            currentStatus.speed = parseInt(cmd);
        }
        
        return true;
    } catch (err) {
        console.error('전송 오류:', err);
        currentStatus.connected = false;
        return false;
    }
}

// ===== HTML 페이지 =====
const HTML_PAGE = `
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>RC Car Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
            user-select: none;
        }
        
        :root {
            --bg: #0f172a;
            --card: #1e293b;
            --border: #334155;
            --text: #f1f5f9;
            --dim: #64748b;
            --blue: #3b82f6;
            --green: #22c55e;
            --red: #ef4444;
            --purple: #8b5cf6;
            --cyan: #06b6d4;
        }
        
        html, body {
            height: 100%;
            overflow: hidden;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg);
            color: var(--text);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 20px;
            gap: 20px;
        }
        
        h1 {
            font-size: 1.8rem;
            background: linear-gradient(135deg, #60a5fa, #a78bfa);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .status {
            display: flex;
            gap: 30px;
            background: var(--card);
            padding: 12px 25px;
            border-radius: 12px;
            border: 1px solid var(--border);
        }
        
        .status-item { text-align: center; }
        
        .status-label {
            font-size: 0.7rem;
            color: var(--dim);
            text-transform: uppercase;
            margin-bottom: 4px;
        }
        
        .status-value {
            font-size: 1.1rem;
            font-weight: 600;
        }
        
        #direction { color: #60a5fa; }
        #speed { color: #34d399; }
        #connection { color: #f59e0b; }
        #connection.on { color: var(--green); }
        #connection.off { color: var(--red); }
        
        .controller {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 10px;
        }
        
        .row {
            display: flex;
            gap: 10px;
        }
        
        .btn {
            width: 85px;
            height: 85px;
            border: none;
            border-radius: 16px;
            font-size: 2rem;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            background: var(--card);
            color: var(--text);
            border: 2px solid var(--border);
            transition: transform 0.1s;
        }
        
        .btn:active, .btn.active { transform: scale(0.95); }
        
        .btn-up {
            background: linear-gradient(145deg, #3b82f6, #2563eb);
            border-color: #3b82f6;
        }
        
        .btn-down {
            background: linear-gradient(145deg, #8b5cf6, #7c3aed);
            border-color: #8b5cf6;
        }
        
        .btn-left, .btn-right {
            background: linear-gradient(145deg, #06b6d4, #0891b2);
            border-color: #06b6d4;
        }
        
        .btn-stop {
            background: linear-gradient(145deg, #ef4444, #dc2626);
            border-color: #ef4444;
        }
        
        .spacer { width: 85px; height: 85px; }
        
        .speed-control {
            width: 100%;
            max-width: 300px;
            background: var(--card);
            padding: 15px 20px;
            border-radius: 12px;
            border: 1px solid var(--border);
        }
        
        .speed-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }
        
        .speed-title { color: var(--dim); font-size: 0.85rem; }
        .speed-value { color: var(--green); font-size: 1.3rem; font-weight: bold; }
        
        input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: var(--border);
            outline: none;
            -webkit-appearance: none;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: var(--green);
            cursor: pointer;
            border: 3px solid white;
        }
        
        .hint {
            color: var(--dim);
            font-size: 0.75rem;
            text-align: center;
        }
        
        .key {
            display: inline-block;
            background: var(--card);
            padding: 2px 8px;
            border-radius: 4px;
            font-family: monospace;
            margin: 0 2px;
            border: 1px solid var(--border);
        }
        
        @media (min-width: 600px) {
            h1 { font-size: 2.2rem; }
            .btn { width: 100px; height: 100px; font-size: 2.5rem; }
            .spacer { width: 100px; height: 100px; }
            .row { gap: 15px; }
            .controller { gap: 15px; }
        }
        
        @media (max-height: 500px) and (orientation: landscape) {
            body {
                flex-direction: row;
                justify-content: space-around;
                padding: 10px 30px;
            }
            h1 { display: none; }
            .status {
                position: fixed;
                top: 10px;
                left: 50%;
                transform: translateX(-50%);
                padding: 8px 20px;
                gap: 20px;
            }
            .status-label { display: none; }
            .controller { margin-top: 30px; }
            .speed-control {
                position: fixed;
                bottom: 10px;
                left: 50%;
                transform: translateX(-50%);
                width: 250px;
                padding: 10px 15px;
            }
            .hint { display: none; }
            .btn { width: 70px; height: 70px; font-size: 1.8rem; }
            .spacer { width: 70px; height: 70px; }
        }
    </style>
</head>
<body>
    <h1>🚗 RC Car Controller</h1>
    
    <div class="status">
        <div class="status-item">
            <div class="status-label">Direction</div>
            <div class="status-value" id="direction">STOP</div>
        </div>
        <div class="status-item">
            <div class="status-label">Speed</div>
            <div class="status-value" id="speed">7</div>
        </div>
        <div class="status-item">
            <div class="status-label">Status</div>
            <div class="status-value" id="connection">●</div>
        </div>
    </div>
    
    <div class="controller">
        <div class="row">
            <div class="spacer"></div>
            <button class="btn btn-up" id="btnW" data-cmd="W">▲</button>
            <div class="spacer"></div>
        </div>
        <div class="row">
            <button class="btn btn-left" id="btnA" data-cmd="A">◀</button>
            <button class="btn btn-stop" id="btnX" data-cmd="X">✕</button>
            <button class="btn btn-right" id="btnD" data-cmd="D">▶</button>
        </div>
        <div class="row">
            <div class="spacer"></div>
            <button class="btn btn-down" id="btnS" data-cmd="S">▼</button>
            <div class="spacer"></div>
        </div>
    </div>
    
    <div class="speed-control">
        <div class="speed-header">
            <span class="speed-title">SPEED</span>
            <span class="speed-value" id="speedDisplay">7</span>
        </div>
        <input type="range" id="speedSlider" min="0" max="9" value="7">
    </div>
    
    <div class="hint">
        <span class="key">W</span> 전진
        <span class="key">S</span> 후진
        <span class="key">A</span> 좌회전
        <span class="key">D</span> 우회전
        <span class="key">X</span> 정지
    </div>

    <script>
        function sendCmd(cmd) {
            fetch('/cmd?c=' + cmd)
                .then(r => r.json())
                .then(data => {
                    document.getElementById('direction').textContent = data.direction;
                    document.getElementById('speed').textContent = data.speed;
                    updateConnection(data.connected);
                })
                .catch(() => updateConnection(false));
        }
        
        function updateConnection(connected) {
            const el = document.getElementById('connection');
            el.textContent = connected ? '●' : '○';
            el.className = 'status-value ' + (connected ? 'on' : 'off');
        }
        
        document.querySelectorAll('.btn').forEach(btn => {
            const cmd = btn.dataset.cmd;
            
            btn.addEventListener('touchstart', e => {
                e.preventDefault();
                btn.classList.add('active');
                sendCmd(cmd);
            });
            
            btn.addEventListener('touchend', e => {
                e.preventDefault();
                btn.classList.remove('active');
                if (cmd !== 'X') sendCmd('X');
            });
            
            btn.addEventListener('mousedown', () => {
                btn.classList.add('active');
                sendCmd(cmd);
            });
            
            btn.addEventListener('mouseup', () => {
                btn.classList.remove('active');
                if (cmd !== 'X') sendCmd('X');
            });
            
            btn.addEventListener('mouseleave', () => {
                if (btn.classList.contains('active')) {
                    btn.classList.remove('active');
                    if (cmd !== 'X') sendCmd('X');
                }
            });
        });
        
        document.getElementById('speedSlider').addEventListener('input', function() {
            document.getElementById('speedDisplay').textContent = this.value;
            sendCmd(this.value);
        });
        
        const keyMap = {
            'w': 'W', 'W': 'W', 'ArrowUp': 'W',
            's': 'S', 'S': 'S', 'ArrowDown': 'S',
            'a': 'A', 'A': 'A', 'ArrowLeft': 'A',
            'd': 'D', 'D': 'D', 'ArrowRight': 'D',
            'x': 'X', 'X': 'X', ' ': 'X', 'Escape': 'X'
        };
        
        const pressed = new Set();
        
        document.addEventListener('keydown', e => {
            if (keyMap[e.key] && !pressed.has(e.key)) {
                e.preventDefault();
                pressed.add(e.key);
                const cmd = keyMap[e.key];
                const btn = document.getElementById('btn' + cmd);
                if (btn) btn.classList.add('active');
                sendCmd(cmd);
            }
            if (e.key >= '0' && e.key <= '9') {
                document.getElementById('speedSlider').value = e.key;
                document.getElementById('speedDisplay').textContent = e.key;
                sendCmd(e.key);
            }
        });
        
        document.addEventListener('keyup', e => {
            if (keyMap[e.key]) {
                e.preventDefault();
                pressed.delete(e.key);
                const cmd = keyMap[e.key];
                const btn = document.getElementById('btn' + cmd);
                if (btn) btn.classList.remove('active');
                if (cmd !== 'X' && pressed.size === 0) sendCmd('X');
            }
        });
        
        setInterval(() => {
            fetch('/status').then(r => r.json())
                .then(data => updateConnection(data.connected))
                .catch(() => updateConnection(false));
        }, 3000);
    </script>
</body>
</html>
`;

// ===== Express 라우트 =====

app.get('/', (req, res) => {
    res.send(HTML_PAGE);
});

app.get('/cmd', (req, res) => {
    const cmd = req.query.c || '';
    if (cmd) {
        sendCommand(cmd);
    }
    res.json(currentStatus);
});

app.get('/status', (req, res) => {
    res.json(currentStatus);
});

app.get('/ports', async (req, res) => {
    const ports = await listPorts();
    res.json(ports);
});

// 특정 포트로 연결
app.get('/connect', async (req, res) => {
    const portPath = req.query.port;
    if (portPath) {
        if (serialPort && serialPort.isOpen) {
            serialPort.close();
        }
        try {
            serialPort = new SerialPort({
                path: portPath,
                baudRate: BAUD_RATE
            });
            currentStatus.connected = true;
            currentStatus.port = portPath;
            res.json({ success: true, port: portPath });
        } catch (err) {
            res.json({ success: false, error: err.message });
        }
    } else {
        res.json({ success: false, error: 'No port specified' });
    }
});

// ===== 서버 시작 =====

async function startServer() {
    console.log('='.repeat(50));
    console.log('   STM32 RC Car Web Controller (Node.js)');
    console.log('='.repeat(50));
    
    // 시리얼 초기화
    await initSerial();
    
    // 서버 시작
    app.listen(PORT, '0.0.0.0', () => {
        // 로컬 IP 주소 가져오기
        const os = require('os');
        const interfaces = os.networkInterfaces();
        let localIP = 'localhost';
        
        for (const name of Object.keys(interfaces)) {
            for (const iface of interfaces[name]) {
                if (iface.family === 'IPv4' && !iface.internal) {
                    localIP = iface.address;
                    break;
                }
            }
        }
        
        console.log();
        console.log('🌐 웹 서버 시작!');
        console.log(`   PC 접속:      http://localhost:${PORT}`);
        console.log(`   모바일 접속:  http://${localIP}:${PORT}`);
        console.log();
        console.log('   (같은 WiFi 네트워크에서 접속하세요)');
        console.log('='.repeat(50));
    });
}

startServer();
