using System.Drawing.Drawing2D;
using System.Text.Json;

namespace SlamViewer;

public partial class MainForm : Form
{
    // ── Data ──────────────────────────────────────────────────────
    private string? _filePath;
    private SimulationData? _simData;
    private List<FrameData> _frames = [];
    private int _frameCount;
    private int _gridW, _gridH;
    private float _envW, _envH;

    // ── Precomputed occupancy grids (log-odds) per frame ──────────
    private List<float[,]> _frameGrids = [];

    // ── Animation state ────────────────────────────────────────────
    private int _currentIdx;
    private bool _isPlaying;
    private double _speed = 1.0;
    private readonly System.Windows.Forms.Timer _timer;

    // ── Drawing caches ─────────────────────────────────────────────
    private Bitmap? _gridBitmap;
    private readonly List<PointF> _trajectory = [];

    // ── Controls ───────────────────────────────────────────────────
    private readonly ToolStrip _toolbar;
    private readonly ToolStripButton _btnOpen;
    private readonly ToolStripButton _btnPlay;
    private readonly ToolStripButton _btnPrev;
    private readonly ToolStripButton _btnNext;
    private readonly ToolStripLabel _lblFrame;
    private readonly ToolStripButton _btnSpeedDown;
    private readonly ToolStripLabel _lblSpeed;
    private readonly ToolStripButton _btnSpeedUp;
    private readonly SplitContainer _split;
    private readonly Panel _envPanel;
    private readonly Panel _gridPanel;
    private readonly StatusStrip _statusBar;
    private readonly ToolStripStatusLabel _lblStatus;

    // ── Coordinate mapping (env panel) ─────────────────────────────
    private float _envScale = 1f;
    private float _envOffsetX = 0f;
    private float _envOffsetY = 0f;

    // ═══════════════════════════════════════════════════════════════
    public MainForm(string? jsonPath)
    {
        // ── Set up form ────────────────────────────────────────
        Text = "SLAM Data Viewer — C#";
        Size = new Size(1280, 720);
        MinimumSize = new Size(800, 500);
        StartPosition = FormStartPosition.CenterScreen;
        BackColor = Color.FromArgb(30, 30, 30);
        DoubleBuffered = true;

        // ── Toolbar ────────────────────────────────────────────
        _toolbar = new ToolStrip { BackColor = Color.FromArgb(45, 45, 45), ForeColor = Color.White };

        _btnOpen = new ToolStripButton("Open", null, (_, _) => OpenFile()) { ForeColor = Color.White };
        _btnPlay = new ToolStripButton("▶", null, (_, _) => TogglePlay()) { ForeColor = Color.White };
        _btnPrev = new ToolStripButton("◀", null, (_, _) => StepFrame(-1)) { ForeColor = Color.White };
        _btnNext = new ToolStripButton("▶", null, (_, _) => StepFrame(+1)) { ForeColor = Color.White };
        _lblFrame = new ToolStripLabel("  0 / 0") { ForeColor = Color.White };

        _toolbar.Items.AddRange([
            _btnOpen, new ToolStripSeparator(),
            _btnPlay, new ToolStripSeparator(),
            _btnPrev, _btnNext, new ToolStripSeparator(),
            new ToolStripLabel(" Speed:") { ForeColor = Color.White },
            _btnSpeedDown = new ToolStripButton("−", null, (_, _) => ChangeSpeed(-0.5)) { ForeColor = Color.White },
            _lblSpeed = new ToolStripLabel("1.0×") { ForeColor = Color.White },
            _btnSpeedUp = new ToolStripButton("+", null, (_, _) => ChangeSpeed(+0.5)) { ForeColor = Color.White },
            new ToolStripSeparator(),
            _lblFrame
        ]);

        // ── Split container ────────────────────────────────────
        _split = new SplitContainer
        {
            Dock = DockStyle.Fill,
            SplitterDistance = Width / 2,
            BackColor = Color.FromArgb(30, 30, 30)
        };

        _envPanel = new Panel { Dock = DockStyle.Fill, BackColor = Color.FromArgb(15, 15, 15) };
        _envPanel.Paint += EnvPanel_Paint;
        _envPanel.Resize += (_, _) => _envPanel.Invalidate();
        _split.Panel1.Controls.Add(_envPanel);

        _gridPanel = new Panel { Dock = DockStyle.Fill, BackColor = Color.FromArgb(15, 15, 15) };
        _gridPanel.Paint += GridPanel_Paint;
        _gridPanel.Resize += (_, _) => { _gridPanel.Invalidate(); RebuildGridBitmap(); };
        _split.Panel2.Controls.Add(_gridPanel);

        // ── Status bar ─────────────────────────────────────────
        _statusBar = new StatusStrip { BackColor = Color.FromArgb(45, 45, 45), ForeColor = Color.White };
        _lblStatus = new ToolStripStatusLabel("Ready") { ForeColor = Color.White };
        _statusBar.Items.Add(_lblStatus);

        // ── Layout ─────────────────────────────────────────────
        Controls.Add(_split);
        Controls.Add(_toolbar);
        Controls.Add(_statusBar);
        _timer = new System.Windows.Forms.Timer { Interval = 50 };
        _timer.Tick += (_, _) => { if (_isPlaying) StepFrame(+1); };

        // ── Load initial file ──────────────────────────────────
        if (jsonPath != null || File.Exists("sensor_data.json"))
            LoadFile(jsonPath ?? "sensor_data.json");

        Shown += (_, _) =>
        {
            if (_simData != null) { RenderFrame(0); UpdateUI(); }
        };
    }

    private void OpenFile()
    {
        using var dlg = new OpenFileDialog
        {
            Filter = "JSON files (*.json)|*.json|All files (*.*)|*.*",
            Title = "Open SLAM sensor data",
            RestoreDirectory = true
        };
        if (dlg.ShowDialog() == DialogResult.OK)
            LoadFile(dlg.FileName);
    }

    private void LoadFile(string path)
    {
        if (!File.Exists(path))
        {
            MessageBox.Show($"File not found: {path}", "Error",
                            MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        try
        {
            _isPlaying = false;
            _timer.Stop();
            _btnPlay.Text = "▶";

            string json = File.ReadAllText(path);
            var data = JsonSerializer.Deserialize<SimulationData>(json);

            if (data?.Data == null || data.Data.Count == 0)
            {
                MessageBox.Show("No frame data found in file.", "Error",
                                MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            _filePath = path;
            _simData = data;
            _frames = data.Data;
            _frameCount = _frames.Count;
            _envW = (float)(data.Environment?.Width ?? 150);
            _envH = (float)(data.Environment?.Height ?? 150);
            _gridW = (int)Math.Ceiling(_envW);
            _gridH = (int)Math.Ceiling(_envH);
            _currentIdx = 0;
            _trajectory.Clear();
            _frameGrids.Clear();
            _gridBitmap?.Dispose();
            _gridBitmap = null;

            PrecomputeGrids();
            RenderFrame(0);
            UpdateUI();

            Text = $"SLAM Data Viewer — {Path.GetFileName(path)}";
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Failed to load file:\n{ex.Message}", "Error",
                            MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    // ═══════════════════════════════════════════════════════════════
    //  Grid pre‑computation
    // ═══════════════════════════════════════════════════════════════
    private void PrecomputeGrids()
    {
        float lOcc = 0.85f;
        float lFree = -0.4f;
        float[,] logOdds = new float[_gridH, _gridW];

        foreach (var frame in _frames)
        {
            var scan = frame.Scan;
            if (scan != null && scan.Count > 0)
            {
                var gt = frame.GroundTruth!;
                float px = (float)gt.X;
                float py = (float)gt.Y;
                int gx0 = (int)(px / 1.0);
                int gy0 = (int)(py / 1.0);
                gx0 = Math.Clamp(gx0, 0, _gridW - 1);
                gy0 = Math.Clamp(gy0, 0, _gridH - 1);

                foreach (var rd in scan)
                {
                    double aRad = rd.AngleWorld * Math.PI / 180.0;
                    float dist = (float)rd.Distance;
                    float ex = px + dist * (float)Math.Cos(aRad);
                    float ey = py + dist * (float)Math.Sin(aRad);
                    int gx1 = Math.Clamp((int)(ex / 1.0), 0, _gridW - 1);
                    int gy1 = Math.Clamp((int)(ey / 1.0), 0, _gridH - 1);

                    var cells = Bresenham(gx0, gy0, gx1, gy1);
                    if (cells.Count < 2) continue;

                    for (int i = 0; i < cells.Count - 1; i++)
                    {
                        int cx = cells[i].x, cy = cells[i].y;
                        if (cx >= 0 && cx < _gridW && cy >= 0 && cy < _gridH)
                            logOdds[cy, cx] += lFree;
                    }
                    int lx = cells[^1].x, ly = cells[^1].y;
                    if (lx >= 0 && lx < _gridW && ly >= 0 && ly < _gridH)
                        logOdds[ly, lx] += lOcc;
                }
            }
            _frameGrids.Add((float[,])logOdds.Clone());
        }
    }

    private static List<(int x, int y)> Bresenham(int x0, int y0, int x1, int y1)
    {
        var pts = new List<(int, int)>();
        int dx = Math.Abs(x1 - x0);
        int dy = -Math.Abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        int x = x0, y = y0;
        while (true)
        {
            pts.Add((x, y));
            if (x == x1 && y == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x += sx; }
            if (e2 <= dx) { err += dx; y += sy; }
        }
        return pts;
    }

    private static float LogOddsToProb(float lo)
    {
        lo = Math.Clamp(lo, -10f, 10f);
        return 1.0f - 1.0f / (1.0f + MathF.Exp(lo));
    }

    // ── Rebuild cached grid bitmap ─────────────────────────────
    private void RebuildGridBitmap()
    {
        if (_currentIdx < 0 || _currentIdx >= _frameCount) return;
        var lo = _frameGrids[_currentIdx];
        int pw = Math.Max(1, _gridPanel.ClientSize.Width);
        int ph = Math.Max(1, _gridPanel.ClientSize.Height);
        float scale = Math.Min(pw / (float)_gridW, ph / (float)_gridH) * 0.95f;
        int bw = (int)(_gridW * scale);
        int bh = (int)(_gridH * scale);
        if (bw < 1 || bh < 1) return;

        _gridBitmap?.Dispose();
        _gridBitmap = new Bitmap(bw, bh);

        using var g = Graphics.FromImage(_gridBitmap);
        for (int gy = 0; gy < _gridH; gy++)
        {
            for (int gx = 0; gx < _gridW; gx++)
            {
                float p = LogOddsToProb(lo[_gridH - 1 - gy, gx]);
                int val = (int)(p * 255);
                val = Math.Clamp(val, 0, 255);
                using var brush = new SolidBrush(Color.FromArgb(val, val, val));
                float x = gx * scale;
                float y = gy * scale;
                g.FillRectangle(brush, x, y, scale, scale);
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    //  Rendering — Environment Panel
    // ═══════════════════════════════════════════════════════════════
    private void EnvPanel_Paint(object? sender, PaintEventArgs e)
    {
        if (_simData == null) return;
        var g = e.Graphics;
        g.SmoothingMode = SmoothingMode.AntiAlias;
        g.Clear(Color.FromArgb(15, 15, 15));

        int pw = _envPanel.ClientSize.Width;
        int ph = _envPanel.ClientSize.Height;
        float margin = 20;
        _envScale = Math.Min((pw - margin * 2) / _envW, (ph - margin * 2) / _envH);
        _envOffsetX = (pw - _envW * _envScale) / 2f;
        _envOffsetY = (ph - _envH * _envScale) / 2f;

        g.TranslateTransform(_envOffsetX, _envOffsetY);
        g.ScaleTransform(_envScale, _envScale);

        using var wallPen = new Pen(Color.FromArgb(200, 200, 200), 2.0f / _envScale);
        using var thickWall = new Pen(Color.FromArgb(220, 220, 220), 3.0f / _envScale);

        // Walls
        g.DrawLine(thickWall, 0, 0, _envW, 0);
        g.DrawLine(thickWall, _envW, 0, _envW, _envH);
        g.DrawLine(thickWall, _envW, _envH, 0, _envH);
        g.DrawLine(thickWall, 0, _envH, 0, 0);

        // Obstacles (rectangles)
        var rects = _simData.Environment?.Obstacles;
        if (rects != null)
        {
            using var brush = new HatchBrush(HatchStyle.ForwardDiagonal,
                Color.FromArgb(160, 90, 90), Color.FromArgb(80, 40, 40));
            foreach (var r in rects)
            {
                if (r.Count >= 4)
                    g.FillRectangle(brush, (float)r[0], (float)r[1], (float)r[2], (float)r[3]);
            }
        }

        // Obstacles (circles)
        var circles = _simData.Environment?.Circles;
        if (circles != null)
        {
            using var brush = new HatchBrush(HatchStyle.ForwardDiagonal,
                Color.FromArgb(160, 90, 90), Color.FromArgb(80, 40, 40));
            foreach (var c in circles)
            {
                if (c.Count >= 3)
                    g.FillEllipse(brush, (float)(c[0] - c[2]), (float)(c[1] - c[2]),
                                  (float)(c[2] * 2), (float)(c[2] * 2));
            }
        }

        // Trajectory
        if (_trajectory.Count >= 2)
        {
            using var trajPen = new Pen(Color.FromArgb(80, 140, 255), 1.5f / _envScale);
            g.DrawLines(trajPen, _trajectory.ToArray());
        }

        // Scan rays (subsample)
        if (_currentIdx >= 0 && _currentIdx < _frameCount)
        {
            var scan = _frames[_currentIdx].Scan;
            if (scan != null)
            {
                var gt = _frames[_currentIdx].GroundTruth!;
                float px = (float)gt.X;
                float py = (float)gt.Y;
                int step = Math.Max(1, scan.Count / 30);
                using var rayPen0 = new Pen(Color.FromArgb(25, 160, 60, 60), 0.5f / _envScale);
                using var rayPen1 = new Pen(Color.FromArgb(25, 60, 160, 60), 1.0f / _envScale);

                for (int i = 0; i < scan.Count; i += step)
                {
                    var rd = scan[i];
                    double aRad = rd.AngleWorld * Math.PI / 180.0;
                    float dist = (float)rd.Distance;
                    float ex = px + dist * (float)Math.Cos(aRad);
                    float ey = py + dist * (float)Math.Sin(aRad);
                    var pen = rd.SensorId == 0 ? rayPen0 : rayPen1;
                    g.DrawLine(pen, px, py, ex, ey);
                }
            }
        }

        // Robot
        if (_currentIdx >= 0 && _currentIdx < _frameCount)
        {
            var gt = _frames[_currentIdx].GroundTruth!;
            float px = (float)gt.X;
            float py = (float)gt.Y;
            float heading = (float)gt.Theta;

            float robotR = 3.5f / _envScale;
            g.FillEllipse(Brushes.Red, px - robotR, py - robotR, robotR * 2, robotR * 2);

            float arrowLen = 8f / _envScale;
            float ax = px + arrowLen * MathF.Cos(heading * MathF.PI / 180f);
            float ay = py + arrowLen * MathF.Sin(heading * MathF.PI / 180f);
            using var dirPen = new Pen(Color.Red, 2.5f / _envScale);
            g.DrawLine(dirPen, px, py, ax, ay);

            g.ResetTransform();

            // Info text overlay
            string info = BuildInfoText();
            using var font = new Font("Consolas", 9);
            using var bgBrush = new SolidBrush(Color.FromArgb(200, 40, 40, 40));
            using var textBrush = new SolidBrush(Color.FromArgb(220, 220, 220));
            var sz = g.MeasureString(info, font);
            var rect = new RectangleF(8, 8, sz.Width + 12, sz.Height + 8);
            g.FillRectangle(bgBrush, rect);
            g.DrawString(info, font, textBrush, new PointF(14, 12));
        }
    }

    // ═══════════════════════════════════════════════════════════════
    //  Rendering — Grid Panel
    // ═══════════════════════════════════════════════════════════════
    private void GridPanel_Paint(object? sender, PaintEventArgs e)
    {
        if (_simData == null || _frameGrids.Count == 0) return;
        var g = e.Graphics;
        g.Clear(Color.FromArgb(15, 15, 15));

        if (_gridBitmap == null) return;

        int pw = _gridPanel.ClientSize.Width;
        int ph = _gridPanel.ClientSize.Height;
        int bw = _gridBitmap.Width;
        int bh = _gridBitmap.Height;
        float ox = (pw - bw) / 2f;
        float oy = (ph - bh) / 2f;

        g.DrawImage(_gridBitmap, ox, oy, bw, bh);

        // Grid info
        var lo = _frameGrids[_currentIdx];
        int known = 0, total = _gridW * _gridH;
        for (int y = 0; y < _gridH; y++)
            for (int x = 0; x < _gridW; x++)
            {
                float p = LogOddsToProb(lo[y, x]);
                if (p > 0.45f && p < 0.55f) known++;
            }
        int mapped = total - known;
        double pct = 100.0 * mapped / total;

        string info = $"Cell: {_gridW}×{_gridH}\nMapped: {mapped}/{total} ({pct:F1}%)";
        using var font = new Font("Consolas", 9);
        using var bgBrush = new SolidBrush(Color.FromArgb(200, 40, 40, 40));
        using var textBrush = new SolidBrush(Color.FromArgb(220, 220, 220));
        var sz = g.MeasureString(info, font);
        var rect = new RectangleF(8, 8, sz.Width + 12, sz.Height + 8);
        g.FillRectangle(bgBrush, rect);
        g.DrawString(info, font, textBrush, new PointF(14, 12));

        // Robot on grid
        var gt = _frames[_currentIdx].GroundTruth!;
        float gx = (float)gt.X;
        float gy = (float)gt.Y;
        float cellSize = bw / (float)_gridW;
        float rx = ox + gx * cellSize;
        float ry = oy + (_gridH - gy) * cellSize;
        g.FillEllipse(Brushes.Red, rx - 3, ry - 3, 6, 6);
    }

    // ═══════════════════════════════════════════════════════════════
    //  Helpers
    // ═══════════════════════════════════════════════════════════════
    private string BuildInfoText()
    {
        if (_currentIdx < 0 || _currentIdx >= _frameCount) return "";
        var f = _frames[_currentIdx];
        var gt = f.GroundTruth!;
        var odom = f.Odometry!;
        string status = f.IsTurn ? "TURNING" : "SCANNING";
        int scanCount = f.Scan?.Count ?? 0;
        return $"[{_currentIdx + 1}/{_frameCount}]  t={f.Timestamp:F1}s\n" +
               $"Pose: ({gt.X:F1}, {gt.Y:F1}, {gt.Theta:F1})\n" +
               $"Enc: L={odom.LeftEncoderTicks}  R={odom.RightEncoderTicks}\n" +
               $"Dist: {odom.DistanceCm:F0} cm\n" +
               $"Status: {status}  Rays: {scanCount}";
    }

    private void UpdateStatus()
    {
        var lo = _frameGrids[_currentIdx];
        int known = 0;
        int total = _gridW * _gridH;
        for (int y = 0; y < _gridH; y++)
            for (int x = 0; x < _gridW; x++)
            {
                float p = LogOddsToProb(lo[y, x]);
                if (p > 0.45f && p < 0.55f) known++;
            }
        int mapped = total - known;
        _lblStatus.Text = $"Frame: {_currentIdx + 1}/{_frameCount}  |  " +
                          $"Mapped: {mapped}/{total} ({100.0 * mapped / total:F1}%)  |  " +
                          $"Speed: {_speed:F1}×  |  " +
                          $"{(_isPlaying ? "▶ Playing" : "⏸ Paused")}";
    }

    private void UpdateUI()
    {
        _lblFrame.Text = $"Frame: {_currentIdx + 1} / {_frameCount}";
        _lblSpeed.Text = $"{_speed:F1}×";
        UpdateStatus();
    }

    // ═══════════════════════════════════════════════════════════════
    //  Controls
    // ═══════════════════════════════════════════════════════════════
    private void RenderFrame(int idx)
    {
        _currentIdx = Math.Clamp(idx, 0, _frameCount - 1);
        _trajectory.Clear();
        for (int i = 0; i <= _currentIdx; i++)
        {
            var gt = _frames[i].GroundTruth;
            if (gt != null)
                _trajectory.Add(new PointF((float)gt.X, (float)gt.Y));
        }
        RebuildGridBitmap();
        _envPanel.Invalidate();
        _gridPanel.Invalidate();
        UpdateUI();
    }

    private void StepFrame(int delta)
    {
        int next = Math.Clamp(_currentIdx + delta, 0, _frameCount - 1);
        if (next == _currentIdx && delta > 0) return; // end of animation
        RenderFrame(next);
    }

    private void TogglePlay()
    {
        _isPlaying = !_isPlaying;
        if (_isPlaying)
        {
            if (_currentIdx >= _frameCount - 1) RenderFrame(0);
            _timer.Start();
            _btnPlay.Text = "⏸";
        }
        else
        {
            _timer.Stop();
            _btnPlay.Text = "▶";
        }
        UpdateUI();
    }

    private void ChangeSpeed(double delta)
    {
        _speed = Math.Clamp(_speed + delta, 0.5, 5.0);
        _timer.Interval = (int)(50.0 / _speed);
        _lblSpeed.Text = $"{_speed:F1}×";
    }

    // ── Keyboard shortcuts ─────────────────────────────────
    protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
    {
        switch (keyData)
        {
            case Keys.Space:
                TogglePlay();
                return true;
            case Keys.Right:
                StepFrame(+1);
                return true;
            case Keys.Left:
                StepFrame(-1);
                return true;
            case Keys.Control | Keys.O:
                OpenFile();
                return true;
            case Keys.Oemplus or Keys.Add:
                ChangeSpeed(+0.5);
                return true;
            case Keys.OemMinus or Keys.Subtract:
                ChangeSpeed(-0.5);
                return true;
        }
        return base.ProcessCmdKey(ref msg, keyData);
    }
}
