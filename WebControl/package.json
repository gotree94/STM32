/**
 * STM32 RC Car Web Controller - Node.js Version
 * 
 * 설치: npm install express serialport
 * 실행: node server.js
 * 접속: http://localhost:3000
 * 
 * 시리얼 포트 설정 방법:
 *   1. config.json 파일에서 설정
 *   2. 웹 UI 설정 페이지에서 변경
 *   3. 명령줄 인자: node server.js --port COM3
 */

const express = require('express');
const { SerialPort } = require('serialport');
const fs = require('fs');
const path = require('path');
const os = require('os');

const app = express();
app.use(express.json());

// ===== 설정 로드 =====
const CONFIG_PATH = path.join(__dirname, 'config.json');

let config = {
    serial: { port: 'auto', baudRate: 115200 },
    server: { port: 3000 }
};

// 설정 파일 로드
function loadConfig() {
    try {
        if (fs.existsSync(CONFIG_PATH)) {
            const data = fs.readFileSync(CONFIG_PATH, 'utf8');
            config = JSON.parse(data);
            console.log('📁 설정 파일 로드: config.json');
        }
    } catch (err) {
        console.log('⚠️  설정 파일 로드 실패, 기본값 사용');
    }
}

// 설정 파일 저장
function saveConfig() {
    try {
        fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
        console.log('💾 설정 저장됨: config.json');
    } catch (err) {
        console.error('설정 저장 실패:', err);
    }
}

// 명령줄 인자 처리
function parseArgs() {
    const args = process.argv.slice(2);
    for (let i = 0; i < args.length; i++) {
        if ((args[i] === '--port' || args[i] === '-p') && args[i + 1]) {
            config.serial.port = args[i + 1];
            console.log(`📌 명령줄 시리얼 포트: ${config.serial.port}`);
        }
        if ((args[i] === '--baud' || args[i] === '-b') && args[i + 1]) {
            config.serial.baudRate = parseInt(args[i + 1]);
        }
        if ((args[i] === '--server-port' || args[i] === '-s') && args[i + 1]) {
            config.server.port = parseInt(args[i + 1]);
        }
    }
}

loadConfig();
parseArgs();

// ===== 상태 관리 =====
let serialPort = null;
let currentStatus = {
    direction: 'STOP',
    speed: 7,
    connected: false,
    port: '',
    baudRate: config.serial.baudRate
};

// ===== 시리얼 포트 함수 =====

async function listPorts() {
    try {
        return await SerialPort.list();
    } catch (err) {
        console.error('포트 목록 조회 실패:', err);
        return [];
    }
}

async function findSerialPort() {
    const ports = await listPorts();
    if (ports.length === 0) return null;
    
    // 적합한 포트 찾기
    const target = ports.find(p => {
        const info = ((p.manufacturer || '') + (p.friendlyName || '')).toLowerCase();
        return ['serial', 'uart', 'stm', 'st-link', 'ch340', 'cp210', 'ftdi', 'usb']
            .some(key => info.includes(key));
    });
    
    return target ? target.path : ports[0].path;
}

async function connectSerial(portPath, baudRate) {
    // 기존 연결 종료
    if (serialPort && serialPort.isOpen) {
        try {
            serialPort.close();
        } catch (e) {}
    }
    
    // auto인 경우 자동 감지
    if (!portPath || portPath === 'auto') {
        portPath = await findSerialPort();
        if (!portPath) {
            console.log('⚠️  사용 가능한 시리얼 포트가 없습니다.');
            currentStatus.connected = false;
            return false;
        }
    }
    
    baudRate = baudRate || config.serial.baudRate;
    
    try {
        serialPort = new SerialPort({
            path: portPath,
            baudRate: baudRate
        });
        
        serialPort.on('open', () => {
            console.log(`✅ 시리얼 연결: ${portPath} @ ${baudRate}bps`);
            currentStatus.connected = true;
            currentStatus.port = portPath;
            currentStatus.baudRate = baudRate;
            
            // 설정 저장
            config.serial.port = portPath;
            config.serial.baudRate = baudRate;
            saveConfig();
        });
        
        serialPort.on('error', (err) => {
            console.error('시리얼 오류:', err.message);
            currentStatus.connected = false;
        });
        
        serialPort.on('close', () => {
            console.log('시리얼 연결 종료');
            currentStatus.connected = false;
        });
        
        serialPort.on('data', (data) => {
            console.log('STM32:', data.toString().trim());
        });
        
        return true;
    } catch (err) {
        console.error('❌ 시리얼 연결 실패:', err.message);
        currentStatus.connected = false;
        return false;
    }
}

function disconnectSerial() {
    if (serialPort && serialPort.isOpen) {
        serialPort.close();
        currentStatus.connected = false;
        currentStatus.port = '';
        return true;
    }
    return false;
}

function sendCommand(cmd) {
    if (!serialPort || !serialPort.isOpen) {
        currentStatus.connected = false;
        return false;
    }
    
    try {
        serialPort.write(cmd);
        
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
        * { margin: 0; padding: 0; box-sizing: border-box; user-select: none; -webkit-tap-highlight-color: transparent; }
        :root {
            --bg: #0f172a; --card: #1e293b; --border: #334155;
            --text: #f1f5f9; --dim: #64748b;
            --blue: #3b82f6; --green: #22c55e; --red: #ef4444;
            --purple: #8b5cf6; --cyan: #06b6d4; --orange: #f97316;
        }
        html, body { height: 100%; overflow: hidden; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: var(--bg); color: var(--text);
            display: flex; flex-direction: column; align-items: center;
            justify-content: center; padding: 15px; gap: 15px;
        }
        
        /* 헤더 */
        .header { display: flex; align-items: center; gap: 15px; }
        h1 {
            font-size: 1.6rem;
            background: linear-gradient(135deg, #60a5fa, #a78bfa);
            -webkit-background-clip: text; -webkit-text-fill-color: transparent;
        }
        .settings-btn {
            background: var(--card); border: 1px solid var(--border);
            color: var(--text); padding: 8px 12px; border-radius: 8px;
            cursor: pointer; font-size: 1.2rem;
        }
        .settings-btn:hover { background: var(--border); }
        
        /* 상태바 */
        .status {
            display: flex; gap: 25px; background: var(--card);
            padding: 10px 20px; border-radius: 10px; border: 1px solid var(--border);
        }
        .status-item { text-align: center; }
        .status-label { font-size: 0.65rem; color: var(--dim); text-transform: uppercase; }
        .status-value { font-size: 1rem; font-weight: 600; }
        #direction { color: #60a5fa; }
        #speed { color: #34d399; }
        #connection.on { color: var(--green); }
        #connection.off { color: var(--red); }
        #portName { color: var(--orange); font-size: 0.85rem; }
        
        /* 컨트롤러 */
        .controller { display: flex; flex-direction: column; align-items: center; gap: 8px; }
        .row { display: flex; gap: 8px; }
        .btn {
            width: 80px; height: 80px; border: none; border-radius: 14px;
            font-size: 1.8rem; cursor: pointer;
            display: flex; align-items: center; justify-content: center;
            background: var(--card); color: var(--text);
            border: 2px solid var(--border); transition: transform 0.1s;
        }
        .btn:active, .btn.active { transform: scale(0.95); }
        .btn-up { background: linear-gradient(145deg, #3b82f6, #2563eb); border-color: #3b82f6; }
        .btn-down { background: linear-gradient(145deg, #8b5cf6, #7c3aed); border-color: #8b5cf6; }
        .btn-left, .btn-right { background: linear-gradient(145deg, #06b6d4, #0891b2); border-color: #06b6d4; }
        .btn-stop { background: linear-gradient(145deg, #ef4444, #dc2626); border-color: #ef4444; }
        .spacer { width: 80px; height: 80px; }
        
        /* 속도 */
        .speed-control {
            width: 100%; max-width: 280px; background: var(--card);
            padding: 12px 16px; border-radius: 10px; border: 1px solid var(--border);
        }
        .speed-header { display: flex; justify-content: space-between; margin-bottom: 8px; }
        .speed-title { color: var(--dim); font-size: 0.8rem; }
        .speed-value { color: var(--green); font-size: 1.2rem; font-weight: bold; }
        input[type="range"] {
            width: 100%; height: 6px; border-radius: 3px;
            background: var(--border); outline: none; -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none; width: 22px; height: 22px;
            border-radius: 50%; background: var(--green);
            cursor: pointer; border: 3px solid white;
        }
        
        /* 힌트 */
        .hint { color: var(--dim); font-size: 0.7rem; text-align: center; }
        .key {
            display: inline-block; background: var(--card);
            padding: 2px 6px; border-radius: 3px; font-family: monospace;
            border: 1px solid var(--border); margin: 0 1px;
        }
        
        /* 설정 모달 */
        .modal {
            display: none; position: fixed; top: 0; left: 0;
            width: 100%; height: 100%; background: rgba(0,0,0,0.7);
            justify-content: center; align-items: center; z-index: 100;
        }
        .modal.show { display: flex; }
        .modal-content {
            background: var(--card); border-radius: 16px;
            padding: 24px; width: 90%; max-width: 400px;
            border: 1px solid var(--border);
        }
        .modal-header {
            display: flex; justify-content: space-between;
            align-items: center; margin-bottom: 20px;
        }
        .modal-title { font-size: 1.3rem; font-weight: 600; }
        .close-btn {
            background: none; border: none; color: var(--dim);
            font-size: 1.5rem; cursor: pointer;
        }
        .form-group { margin-bottom: 16px; }
        .form-label { display: block; color: var(--dim); font-size: 0.85rem; margin-bottom: 6px; }
        .form-select, .form-input {
            width: 100%; padding: 10px 12px; border-radius: 8px;
            border: 1px solid var(--border); background: var(--bg);
            color: var(--text); font-size: 1rem;
        }
        .form-select:focus, .form-input:focus { outline: none; border-color: var(--blue); }
        .btn-row { display: flex; gap: 10px; margin-top: 20px; }
        .btn-primary {
            flex: 1; padding: 12px; border: none; border-radius: 8px;
            background: var(--blue); color: white; font-size: 1rem;
            cursor: pointer; font-weight: 500;
        }
        .btn-primary:hover { background: #2563eb; }
        .btn-secondary {
            flex: 1; padding: 12px; border: 1px solid var(--border);
            border-radius: 8px; background: var(--card); color: var(--text);
            font-size: 1rem; cursor: pointer;
        }
        .btn-danger { background: var(--red); border-color: var(--red); }
        .btn-danger:hover { background: #dc2626; }
        .port-list { max-height: 150px; overflow-y: auto; margin-bottom: 10px; }
        .port-item {
            padding: 10px; background: var(--bg); border-radius: 6px;
            margin-bottom: 6px; cursor: pointer; border: 1px solid var(--border);
        }
        .port-item:hover { border-color: var(--blue); }
        .port-item.selected { border-color: var(--green); background: rgba(34,197,94,0.1); }
        .port-name { font-weight: 500; }
        .port-desc { font-size: 0.8rem; color: var(--dim); }
        .refresh-btn {
            background: var(--card); border: 1px solid var(--border);
            color: var(--text); padding: 6px 12px; border-radius: 6px;
            cursor: pointer; font-size: 0.85rem;
        }
        .status-badge {
            display: inline-block; padding: 4px 10px; border-radius: 12px;
            font-size: 0.8rem; font-weight: 500;
        }
        .status-badge.connected { background: rgba(34,197,94,0.2); color: var(--green); }
        .status-badge.disconnected { background: rgba(239,68,68,0.2); color: var(--red); }
        
        /* 반응형 */
        @media (min-width: 600px) {
            h1 { font-size: 2rem; }
            .btn { width: 95px; height: 95px; font-size: 2.2rem; }
            .spacer { width: 95px; height: 95px; }
            .row { gap: 12px; }
            .controller { gap: 12px; }
        }
        @media (max-height: 500px) and (orientation: landscape) {
            body { flex-direction: row; justify-content: space-around; padding: 10px 30px; }
            .header { display: none; }
            .status {
                position: fixed; top: 8px; left: 50%; transform: translateX(-50%);
                padding: 6px 15px; gap: 15px;
            }
            .status-label { display: none; }
            .controller { margin-top: 25px; }
            .speed-control {
                position: fixed; bottom: 8px; left: 50%; transform: translateX(-50%);
                width: 220px; padding: 8px 12px;
            }
            .hint { display: none; }
            .btn { width: 65px; height: 65px; font-size: 1.6rem; }
            .spacer { width: 65px; height: 65px; }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🚗 RC Car</h1>
        <button class="settings-btn" onclick="openSettings()">⚙️</button>
    </div>
    
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
            <div class="status-label">Port</div>
            <div class="status-value" id="portName">-</div>
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
        <span class="key">W</span><span class="key">A</span><span class="key">S</span><span class="key">D</span> 이동
        <span class="key">X</span> 정지
        <span class="key">0-9</span> 속도
    </div>

    <!-- 설정 모달 -->
    <div class="modal" id="settingsModal">
        <div class="modal-content">
            <div class="modal-header">
                <span class="modal-title">⚙️ 시리얼 포트 설정</span>
                <button class="close-btn" onclick="closeSettings()">×</button>
            </div>
            
            <div class="form-group">
                <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
                    <label class="form-label" style="margin:0;">사용 가능한 포트</label>
                    <button class="refresh-btn" onclick="refreshPorts()">🔄 새로고침</button>
                </div>
                <div class="port-list" id="portList">
                    <div style="color:var(--dim); text-align:center; padding:20px;">로딩 중...</div>
                </div>
            </div>
            
            <div class="form-group">
                <label class="form-label">Baud Rate</label>
                <select class="form-select" id="baudRate">
                    <option value="9600">9600</option>
                    <option value="19200">19200</option>
                    <option value="38400">38400</option>
                    <option value="57600">57600</option>
                    <option value="115200" selected>115200</option>
                    <option value="230400">230400</option>
                    <option value="460800">460800</option>
                </select>
            </div>
            
            <div class="form-group">
                <label class="form-label">현재 상태</label>
                <div>
                    <span class="status-badge" id="modalStatus">연결 안됨</span>
                    <span id="modalPort" style="margin-left:10px; color:var(--dim);"></span>
                </div>
            </div>
            
            <div class="btn-row">
                <button class="btn-primary" onclick="connectPort()">연결</button>
                <button class="btn-secondary btn-danger" onclick="disconnectPort()">연결 해제</button>
            </div>
        </div>
    </div>

    <script>
        let selectedPort = null;
        
        // 명령 전송
        function sendCmd(cmd) {
            fetch('/api/cmd?c=' + cmd)
                .then(r => r.json())
                .then(updateStatus)
                .catch(() => updateConnection(false));
        }
        
        // 상태 업데이트
        function updateStatus(data) {
            document.getElementById('direction').textContent = data.direction;
            document.getElementById('speed').textContent = data.speed;
            document.getElementById('portName').textContent = data.port ? data.port.split('/').pop() : '-';
            updateConnection(data.connected);
        }
        
        function updateConnection(connected) {
            const el = document.getElementById('connection');
            el.textContent = connected ? '●' : '○';
            el.className = 'status-value ' + (connected ? 'on' : 'off');
        }
        
        // 버튼 이벤트
        document.querySelectorAll('.btn').forEach(btn => {
            const cmd = btn.dataset.cmd;
            
            ['touchstart', 'mousedown'].forEach(evt => {
                btn.addEventListener(evt, e => {
                    e.preventDefault();
                    btn.classList.add('active');
                    sendCmd(cmd);
                });
            });
            
            ['touchend', 'mouseup', 'mouseleave'].forEach(evt => {
                btn.addEventListener(evt, e => {
                    if (btn.classList.contains('active')) {
                        btn.classList.remove('active');
                        if (cmd !== 'X') sendCmd('X');
                    }
                });
            });
        });
        
        // 속도 슬라이더
        document.getElementById('speedSlider').addEventListener('input', function() {
            document.getElementById('speedDisplay').textContent = this.value;
            sendCmd(this.value);
        });
        
        // 키보드
        const keyMap = {
            'w':'W', 'W':'W', 'ArrowUp':'W',
            's':'S', 'S':'S', 'ArrowDown':'S',
            'a':'A', 'A':'A', 'ArrowLeft':'A',
            'd':'D', 'D':'D', 'ArrowRight':'D',
            'x':'X', 'X':'X', ' ':'X', 'Escape':'X'
        };
        const pressed = new Set();
        
        document.addEventListener('keydown', e => {
            if (keyMap[e.key] && !pressed.has(e.key)) {
                e.preventDefault();
                pressed.add(e.key);
                const cmd = keyMap[e.key];
                document.getElementById('btn' + cmd)?.classList.add('active');
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
                pressed.delete(e.key);
                const cmd = keyMap[e.key];
                document.getElementById('btn' + cmd)?.classList.remove('active');
                if (cmd !== 'X' && pressed.size === 0) sendCmd('X');
            }
        });
        
        // 설정 모달
        function openSettings() {
            document.getElementById('settingsModal').classList.add('show');
            refreshPorts();
            updateModalStatus();
        }
        
        function closeSettings() {
            document.getElementById('settingsModal').classList.remove('show');
        }
        
        function refreshPorts() {
            const list = document.getElementById('portList');
            list.innerHTML = '<div style="color:var(--dim);text-align:center;padding:20px;">로딩 중...</div>';
            
            fetch('/api/ports')
                .then(r => r.json())
                .then(ports => {
                    if (ports.length === 0) {
                        list.innerHTML = '<div style="color:var(--dim);text-align:center;padding:20px;">포트 없음</div>';
                        return;
                    }
                    list.innerHTML = ports.map(p => 
                        '<div class="port-item" onclick="selectPort(this, \\'' + p.path + '\\')">' +
                        '<div class="port-name">' + p.path + '</div>' +
                        '<div class="port-desc">' + (p.manufacturer || p.friendlyName || 'Unknown') + '</div>' +
                        '</div>'
                    ).join('');
                })
                .catch(() => {
                    list.innerHTML = '<div style="color:var(--red);text-align:center;padding:20px;">조회 실패</div>';
                });
        }
        
        function selectPort(el, port) {
            document.querySelectorAll('.port-item').forEach(i => i.classList.remove('selected'));
            el.classList.add('selected');
            selectedPort = port;
        }
        
        function connectPort() {
            if (!selectedPort) {
                alert('포트를 선택하세요.');
                return;
            }
            const baud = document.getElementById('baudRate').value;
            fetch('/api/connect', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ port: selectedPort, baudRate: parseInt(baud) })
            })
            .then(r => r.json())
            .then(data => {
                if (data.success) {
                    updateModalStatus();
                    alert('연결 성공: ' + data.port);
                } else {
                    alert('연결 실패: ' + data.error);
                }
            });
        }
        
        function disconnectPort() {
            fetch('/api/disconnect', { method: 'POST' })
                .then(r => r.json())
                .then(() => {
                    updateModalStatus();
                    alert('연결 해제됨');
                });
        }
        
        function updateModalStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    const badge = document.getElementById('modalStatus');
                    const portInfo = document.getElementById('modalPort');
                    
                    if (data.connected) {
                        badge.textContent = '연결됨';
                        badge.className = 'status-badge connected';
                        portInfo.textContent = data.port + ' @ ' + data.baudRate + 'bps';
                        document.getElementById('baudRate').value = data.baudRate;
                    } else {
                        badge.textContent = '연결 안됨';
                        badge.className = 'status-badge disconnected';
                        portInfo.textContent = '';
                    }
                    updateStatus(data);
                });
        }
        
        // 모달 외부 클릭 시 닫기
        document.getElementById('settingsModal').addEventListener('click', e => {
            if (e.target.id === 'settingsModal') closeSettings();
        });
        
        // 주기적 상태 확인
        setInterval(() => {
            fetch('/api/status').then(r => r.json()).then(updateStatus).catch(() => updateConnection(false));
        }, 3000);
        
        // 초기 상태 로드
        fetch('/api/status').then(r => r.json()).then(updateStatus);
    </script>
</body>
</html>
`;

// ===== Express 라우트 =====

app.get('/', (req, res) => res.send(HTML_PAGE));

app.get('/api/cmd', (req, res) => {
    const cmd = req.query.c || '';
    if (cmd) sendCommand(cmd);
    res.json(currentStatus);
});

app.get('/api/status', (req, res) => res.json(currentStatus));

app.get('/api/ports', async (req, res) => {
    const ports = await listPorts();
    res.json(ports);
});

app.get('/api/config', (req, res) => res.json(config));

app.post('/api/connect', async (req, res) => {
    const { port, baudRate } = req.body;
    const success = await connectSerial(port, baudRate);
    res.json({ 
        success, 
        port: currentStatus.port,
        error: success ? null : '연결 실패'
    });
});

app.post('/api/disconnect', (req, res) => {
    disconnectSerial();
    res.json({ success: true });
});

app.post('/api/config', (req, res) => {
    const { serial, server } = req.body;
    if (serial) {
        if (serial.port) config.serial.port = serial.port;
        if (serial.baudRate) config.serial.baudRate = serial.baudRate;
    }
    if (server && server.port) config.server.port = server.port;
    saveConfig();
    res.json({ success: true, config });
});

// ===== 서버 시작 =====
async function startServer() {
    console.log('='.repeat(50));
    console.log('   STM32 RC Car Web Controller (Node.js)');
    console.log('='.repeat(50));
    console.log();
    console.log('📋 설정:');
    console.log('   시리얼 포트:', config.serial.port);
    console.log('   Baud Rate:', config.serial.baudRate);
    console.log('   서버 포트:', config.server.port);
    console.log();
    
    // 시리얼 연결
    await connectSerial(config.serial.port, config.serial.baudRate);
    
    // 서버 시작
    app.listen(config.server.port, '0.0.0.0', () => {
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
        console.log('   PC:     http://localhost:' + config.server.port);
        console.log('   모바일: http://' + localIP + ':' + config.server.port);
        console.log();
        console.log('💡 명령줄 옵션:');
        console.log('   --port, -p <포트>      시리얼 포트 지정');
        console.log('   --baud, -b <속도>      Baud Rate 지정');
        console.log('   --server-port, -s <포트>  서버 포트 지정');
        console.log('='.repeat(50));
    });
}

startServer();
