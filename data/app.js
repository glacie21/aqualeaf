// Mirrors PumpState in include/Constants.h (numeric values are part of the API)
const PUMP_STATE_LABELS = { 0: 'OFF', 1: 'ON', 2: 'COOLDOWN', 4: 'FAULT' };
const PUMP_STATE_ON = 1;
const OPERATING_MODE_AUTO = 0;
const ALL_HOURS_MASK = 0xffffff;
const STATUS_INTERVAL_MS = 3000;
const HISTORY_INTERVAL_MS = 60000;
const SVG_NS = 'http://www.w3.org/2000/svg';

const state = {
    moisture: 0,
    moistureRaw: 0,
    pumpState: 0,
    autoMode: true,
    manualOverride: false,
    thresholdLow: 30,
    thresholdHigh: 60,
    allowedHoursMask: ALL_HOURS_MASK,
    sensorFault: false,
    wateringLockout: false,
    dailyLimitReached: false,
    pumpOnTodayS: 0,
    overrideRemainingS: 0,
    weakWebPassword: false,
    rawDry: 0,
    rawWet: 0,
    timeSynced: false,
    localTime: '',
    scheduleAllowsNow: true,
    wifiSsid: '',
    wifiIp: '',
    apActive: false,
    hostname: '',
    firmware: '',
};

// Hours currently shown in the chip editor (may differ from state while dirty).
let editedHoursMask = ALL_HOURS_MASK;

// True while the user has unsaved changes in the config form,
// so periodic polling does not overwrite the sliders.
let configDirty = false;

let history = null;

const elements = {
    statusPill: document.getElementById('statusPill'),
    alerts: document.getElementById('alerts'),
    moisture: document.getElementById('moisture'),
    pumpState: document.getElementById('pumpState'),
    pumpHint: document.getElementById('pumpHint'),
    mode: document.getElementById('mode'),
    modeHint: document.getElementById('modeHint'),
    pumpButton: document.getElementById('pumpButton'),
    thresholdLow: document.getElementById('thresholdLow'),
    thresholdHigh: document.getElementById('thresholdHigh'),
    thresholdLowValue: document.getElementById('thresholdLowValue'),
    thresholdHighValue: document.getElementById('thresholdHighValue'),
    autoMode: document.getElementById('autoMode'),
    hourChips: document.getElementById('hourChips'),
    allHours: document.getElementById('allHours'),
    clockHint: document.getElementById('clockHint'),
    configMessage: document.getElementById('configMessage'),
    saveConfig: document.getElementById('saveConfig'),
    historyChart: document.getElementById('historyChart'),
    chartTooltip: document.getElementById('chartTooltip'),
    historyDetails: document.getElementById('historyDetails'),
    historyTable: document.getElementById('historyTable'),
    rawNow: document.getElementById('rawNow'),
    rawDry: document.getElementById('rawDry'),
    rawWet: document.getElementById('rawWet'),
    calibrateDry: document.getElementById('calibrateDry'),
    calibrateWet: document.getElementById('calibrateWet'),
    calibrateReset: document.getElementById('calibrateReset'),
    calibrationMessage: document.getElementById('calibrationMessage'),
    wifiSsid: document.getElementById('wifiSsid'),
    wifiAddress: document.getElementById('wifiAddress'),
    firmware: document.getElementById('firmware'),
    wifiForm: document.getElementById('wifiForm'),
    wifiSsidInput: document.getElementById('wifiSsidInput'),
    wifiPasswordInput: document.getElementById('wifiPasswordInput'),
    wifiMessage: document.getElementById('wifiMessage'),
};

// ── Networking ──────────────────────────────────────────────

function setOnline(online) {
    elements.statusPill.textContent = online ? 'System Online' : 'Device Offline';
    elements.statusPill.classList.toggle('offline', !online);
}

function showMessage(element, text, isError = false) {
    element.textContent = text;
    element.classList.toggle('error', isError);
    element.hidden = !text;
}

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) {
            setOnline(false);
            return;
        }
        applyStatus(await response.json());
        setOnline(true);
    } catch (error) {
        console.error('Status error', error);
        setOnline(false);
    }
}

/**
 * POST JSON. Returns { ok, data, error }. By default a successful response is a
 * status object and is applied to the page.
 */
async function postJson(url, body, { applyResult = true } = {}) {
    let response;
    try {
        response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body),
        });
    } catch (error) {
        console.error(`Request to ${url} failed`, error);
        setOnline(false);
        return { ok: false, error: 'Device unreachable' };
    }

    setOnline(true);
    const data = await response.json().catch(() => ({}));
    if (!response.ok) {
        return { ok: false, error: data.error || `HTTP ${response.status}` };
    }
    if (applyResult) {
        applyStatus(data);
    }
    return { ok: true, data };
}

// ── Status ──────────────────────────────────────────────────

function applyStatus(data) {
    state.moisture = data.moisture_pct;
    state.moistureRaw = data.moisture_raw;
    state.pumpState = data.pump_state;
    state.autoMode = data.auto_mode;
    state.manualOverride = data.operating_mode !== OPERATING_MODE_AUTO || data.manual_override;
    state.thresholdLow = data.threshold_low;
    state.thresholdHigh = data.threshold_high;
    state.allowedHoursMask = data.allowed_hours_mask;
    state.sensorFault = data.sensor_fault;
    state.wateringLockout = data.watering_lockout;
    state.dailyLimitReached = data.daily_limit_reached;
    state.pumpOnTodayS = data.pump_on_today_s;
    state.overrideRemainingS = data.override_remaining_s;
    state.weakWebPassword = data.weak_web_password;
    state.rawDry = data.raw_dry;
    state.rawWet = data.raw_wet;
    state.timeSynced = data.time_synced;
    state.localTime = data.local_time || '';
    state.scheduleAllowsNow = data.schedule_allows_now;
    state.wifiSsid = data.wifi_ssid;
    state.wifiIp = data.wifi_ip;
    state.apActive = data.ap_active;
    state.hostname = data.hostname;
    state.firmware = data.firmware;
    render();
}

function renderAlerts() {
    const messages = [];
    if (state.sensorFault) {
        messages.push('Sensor fault: moisture reading out of range. Automatic watering is paused — check the sensor wiring.');
    }
    if (state.wateringLockout) {
        messages.push('Automatic watering locked: moisture did not rise after several cycles. Check the water tank and hose, then save the configuration to reset.');
    }
    if (state.dailyLimitReached) {
        messages.push('Daily pump run-time limit reached. The pump stays off until the 24-hour window resets.');
    }
    if (state.weakWebPassword) {
        messages.push('Security: the dashboard still uses the default (or a short) password. Change WEB_ADMIN_PASS in Secrets.h and re-flash.');
    }

    elements.alerts.replaceChildren(...messages.map((text) => {
        const item = document.createElement('p');
        item.className = 'alert';
        item.textContent = text;
        return item;
    }));
    elements.alerts.hidden = messages.length === 0;
}

function renderThresholdLabels() {
    elements.thresholdLowValue.textContent = elements.thresholdLow.value;
    elements.thresholdHighValue.textContent = elements.thresholdHigh.value;
}

function render() {
    const pumpOn = state.pumpState === PUMP_STATE_ON;
    const isAuto = state.autoMode && !state.manualOverride;

    elements.moisture.textContent = state.sensorFault ? '-- %' : `${state.moisture.toFixed(1)} %`;
    elements.pumpState.textContent = PUMP_STATE_LABELS[state.pumpState] || '--';
    elements.pumpState.classList.toggle('status-on', pumpOn);
    elements.pumpState.classList.toggle('status-off', !pumpOn);
    elements.pumpHint.textContent = `Ran ${formatDuration(state.pumpOnTodayS)} in the last 24 h`;
    elements.pumpButton.textContent = pumpOn ? 'Turn Pump Off' : 'Turn Pump On';

    elements.mode.textContent = isAuto ? 'AUTO' : 'MANUAL';
    elements.mode.classList.toggle('status-on', isAuto);
    elements.mode.classList.toggle('status-off', !isAuto);
    if (isAuto) {
        elements.modeHint.textContent = state.scheduleAllowsNow
            ? 'Automatic control'
            : 'Automatic — outside watering hours';
    } else if (state.autoMode && state.overrideRemainingS > 0) {
        const minutes = Math.ceil(state.overrideRemainingS / 60);
        elements.modeHint.textContent = `Manual override — auto resumes in ${minutes} min`;
    } else {
        elements.modeHint.textContent = 'Automatic mode disabled in configuration';
    }
    renderAlerts();

    if (!configDirty) {
        elements.thresholdLow.value = state.thresholdLow;
        elements.thresholdHigh.value = state.thresholdHigh;
        elements.autoMode.checked = state.autoMode;
        editedHoursMask = state.allowedHoursMask;
        renderThresholdLabels();
        renderHourChips();
    }
    elements.clockHint.textContent = state.timeSynced
        ? `Automatic watering only starts during the selected hours. Device clock: ${state.localTime}.`
        : 'Device clock not synced yet (needs internet) — watering is allowed at any hour until it is.';

    elements.rawNow.textContent = state.moistureRaw;
    elements.rawDry.textContent = state.rawDry;
    elements.rawWet.textContent = state.rawWet;

    elements.wifiSsid.textContent = state.apActive && !state.wifiIp
        ? 'Setup access point active'
        : (state.wifiSsid || 'Not configured');
    elements.wifiAddress.textContent = state.wifiIp
        ? `${state.hostname} · ${state.wifiIp}`
        : '192.168.4.1 (setup AP)';
    elements.firmware.textContent = state.firmware;
}

function formatDuration(totalSeconds) {
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = totalSeconds % 60;
    return minutes > 0 ? `${minutes} min ${seconds} s` : `${seconds} s`;
}

// ── Configuration ───────────────────────────────────────────

function renderHourChips() {
    if (elements.hourChips.children.length === 0) {
        for (let hour = 0; hour < 24; hour += 1) {
            const chip = document.createElement('button');
            chip.type = 'button';
            chip.className = 'hour-chip';
            chip.textContent = String(hour).padStart(2, '0');
            chip.setAttribute('aria-label', `${String(hour).padStart(2, '0')}:00 to ${String(hour).padStart(2, '0')}:59`);
            chip.addEventListener('click', () => {
                editedHoursMask ^= (1 << hour);
                markConfigDirty();
                renderHourChips();
            });
            elements.hourChips.appendChild(chip);
        }
    }
    [...elements.hourChips.children].forEach((chip, hour) => {
        chip.setAttribute('aria-pressed', String((editedHoursMask & (1 << hour)) !== 0));
    });
}

async function saveConfig() {
    if ((editedHoursMask & ALL_HOURS_MASK) === 0) {
        showMessage(elements.configMessage, 'Select at least one watering hour.', true);
        return;
    }
    const result = await postJson('/api/config', {
        threshold_low: parseInt(elements.thresholdLow.value, 10),
        threshold_high: parseInt(elements.thresholdHigh.value, 10),
        auto_mode: elements.autoMode.checked,
        allowed_hours_mask: editedHoursMask,
    });
    if (result.ok) {
        configDirty = false;
        showMessage(elements.configMessage, 'Configuration saved.');
        render();
    } else {
        showMessage(elements.configMessage, result.error, true);
    }
}

function markConfigDirty() {
    configDirty = true;
    showMessage(elements.configMessage, '');
    renderThresholdLabels();
}

async function sendPumpRequest() {
    await postJson('/api/pump', { pump_on: state.pumpState !== PUMP_STATE_ON });
}

// ── Calibration ─────────────────────────────────────────────

async function calibrate(body, confirmText) {
    if (confirmText && !window.confirm(confirmText)) {
        return;
    }
    const result = await postJson('/api/calibration', body);
    showMessage(elements.calibrationMessage,
        result.ok ? 'Calibration saved.' : result.error, !result.ok);
}

// ── WiFi ────────────────────────────────────────────────────

async function connectWifi(event) {
    event.preventDefault();
    const ssid = elements.wifiSsidInput.value.trim();
    const password = elements.wifiPasswordInput.value;
    const result = await postJson('/api/wifi', { ssid, password }, { applyResult: false });
    if (!result.ok) {
        showMessage(elements.wifiMessage, result.error, true);
        return;
    }
    elements.wifiPasswordInput.value = '';
    showMessage(elements.wifiMessage,
        `Connecting to “${ssid}”. If this page stops responding, join that network and open http://${state.hostname || 'aqualeaf.local'}.`);
}

// ── History chart ───────────────────────────────────────────

async function fetchHistory() {
    try {
        const response = await fetch('/api/history');
        if (!response.ok) {
            return;
        }
        const data = await response.json();
        // Convert device-relative offsets to wall-clock times using the browser clock.
        const fetchedAt = Date.now();
        const count = data.moisture.length;
        history = {
            intervalS: data.interval_s,
            points: data.moisture.map((value, index) => ({
                time: fetchedAt - ((count - 1 - index) * data.interval_s + data.age_s) * 1000,
                moisture: value,
                watered: data.watered[index] === '1',
            })),
        };
        renderHistory();
        if (elements.historyDetails.open) {
            renderHistoryTable();
        }
    } catch (error) {
        console.error('History error', error);
    }
}

function svgElement(name, attributes) {
    const node = document.createElementNS(SVG_NS, name);
    Object.entries(attributes).forEach(([key, value]) => node.setAttribute(key, value));
    return node;
}

function formatClock(time) {
    return new Date(time).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
}

function renderHistory() {
    const chart = elements.historyChart;
    if (!history || history.points.length === 0) {
        return;
    }

    const width = Math.max(chart.clientWidth, 280);
    const height = 220;
    const pad = { top: 12, right: 12, bottom: 26, left: 40 };
    const plotW = width - pad.left - pad.right;
    const plotH = height - pad.top - pad.bottom;

    // Fixed 24 h window ending now, so a fresh device shows data on the right.
    const end = Date.now();
    const start = end - 24 * 3600 * 1000;
    const x = (time) => pad.left + ((time - start) / (end - start)) * plotW;
    const y = (pct) => pad.top + (1 - pct / 100) * plotH;

    const svg = svgElement('svg', { viewBox: `0 0 ${width} ${height}`, 'aria-hidden': 'true' });

    // Recessive grid and y-axis ticks.
    [0, 25, 50, 75, 100].forEach((tick) => {
        svg.appendChild(svgElement('line', {
            class: 'grid-line', x1: pad.left, x2: width - pad.right, y1: y(tick), y2: y(tick),
        }));
        const label = svgElement('text', {
            class: 'axis-label', x: pad.left - 8, y: y(tick) + 4, 'text-anchor': 'end',
        });
        label.textContent = `${tick}%`;
        svg.appendChild(label);
    });

    // X-axis labels every 6 hours, relative to now.
    [24, 18, 12, 6, 0].forEach((hoursAgo) => {
        const label = svgElement('text', {
            class: 'axis-label',
            x: x(end - hoursAgo * 3600 * 1000),
            y: height - 6,
            'text-anchor': hoursAgo === 24 ? 'start' : (hoursAgo === 0 ? 'end' : 'middle'),
        });
        label.textContent = hoursAgo === 0 ? 'now' : `−${hoursAgo} h`;
        svg.appendChild(label);
    });

    // Threshold reference lines with direct labels.
    [['Low', state.thresholdLow], ['High', state.thresholdHigh]].forEach(([name, value]) => {
        svg.appendChild(svgElement('line', {
            class: 'threshold-line', x1: pad.left, x2: width - pad.right, y1: y(value), y2: y(value),
        }));
        const label = svgElement('text', { class: 'axis-label', x: pad.left + 4, y: y(value) - 4 });
        label.textContent = `${name} ${value}%`;
        svg.appendChild(label);
    });

    // Moisture line + area wash; gaps (sensor fault) break the line.
    const visible = history.points.filter((point) => point.time >= start);
    let segment = [];
    const segments = [];
    visible.forEach((point) => {
        if (point.moisture === null) {
            if (segment.length) segments.push(segment);
            segment = [];
        } else {
            segment.push(point);
        }
    });
    if (segment.length) segments.push(segment);

    segments.forEach((points) => {
        const line = points.map((p, i) => `${i ? 'L' : 'M'}${x(p.time).toFixed(1)},${y(p.moisture).toFixed(1)}`).join('');
        const first = points[0];
        const last = points[points.length - 1];
        svg.appendChild(svgElement('path', {
            class: 'series-area',
            d: `${line}L${x(last.time).toFixed(1)},${y(0)}L${x(first.time).toFixed(1)},${y(0)}Z`,
        }));
        svg.appendChild(svgElement('path', { class: 'series-line', d: line }));
    });

    // Pump activity ticks along the baseline.
    visible.filter((point) => point.watered).forEach((point) => {
        svg.appendChild(svgElement('rect', {
            class: 'pump-mark', x: x(point.time) - 1.5, y: y(0) - 10, width: 3, height: 10, rx: 1,
        }));
    });

    // Hover layer: crosshair + focus dot, positioned on demand.
    const crosshair = svgElement('line', { class: 'crosshair', y1: pad.top, y2: y(0), visibility: 'hidden' });
    const dot = svgElement('circle', { class: 'focus-dot', r: 4, visibility: 'hidden' });
    svg.appendChild(crosshair);
    svg.appendChild(dot);

    chart.replaceChildren(svg);
    chart.setAttribute('aria-label',
        `Soil moisture over the last 24 hours, ${visible.length} samples. Latest ${describePoint(visible[visible.length - 1])}.`);

    let focusIndex = visible.length - 1;

    const showPoint = (index) => {
        const point = visible[index];
        if (!point) return;
        focusIndex = index;
        const px = x(point.time);
        crosshair.setAttribute('x1', px);
        crosshair.setAttribute('x2', px);
        crosshair.setAttribute('visibility', 'visible');
        if (point.moisture !== null) {
            dot.setAttribute('cx', px);
            dot.setAttribute('cy', y(point.moisture));
            dot.setAttribute('visibility', 'visible');
        } else {
            dot.setAttribute('visibility', 'hidden');
        }

        const tooltip = elements.chartTooltip;
        tooltip.replaceChildren();
        const value = document.createElement('strong');
        value.textContent = point.moisture === null ? 'No reading' : `${point.moisture.toFixed(1)} %`;
        const detail = document.createElement('span');
        detail.textContent = `${formatClock(point.time)}${point.watered ? ' · pump ran' : ''}`;
        tooltip.append(value, detail);
        tooltip.hidden = false;

        const scale = chart.clientWidth / width;
        const left = chart.offsetLeft + px * scale;
        const flip = left + tooltip.offsetWidth + 12 > chart.offsetLeft + chart.clientWidth;
        tooltip.style.left = `${flip ? left - tooltip.offsetWidth - 12 : left + 12}px`;
        tooltip.style.top = `${chart.offsetTop + pad.top}px`;
    };

    const hidePoint = () => {
        crosshair.setAttribute('visibility', 'hidden');
        dot.setAttribute('visibility', 'hidden');
        elements.chartTooltip.hidden = true;
    };

    // Snap to the nearest sample in X — readers aim at a time, not at a 2px line.
    chart.onpointermove = (event) => {
        const rect = chart.getBoundingClientRect();
        const time = start + ((event.clientX - rect.left) * (width / rect.width) - pad.left) / plotW * (end - start);
        let nearest = 0;
        visible.forEach((point, index) => {
            if (Math.abs(point.time - time) < Math.abs(visible[nearest].time - time)) nearest = index;
        });
        showPoint(nearest);
    };
    chart.onpointerleave = hidePoint;
    chart.onfocus = () => showPoint(focusIndex);
    chart.onblur = hidePoint;
    chart.onkeydown = (event) => {
        if (event.key === 'ArrowLeft') showPoint(Math.max(0, focusIndex - 1));
        else if (event.key === 'ArrowRight') showPoint(Math.min(visible.length - 1, focusIndex + 1));
        else return;
        event.preventDefault();
    };
}

function describePoint(point) {
    if (!point) return 'none';
    const value = point.moisture === null ? 'no reading' : `${point.moisture.toFixed(1)} percent`;
    return `${value} at ${formatClock(point.time)}`;
}

function renderHistoryTable() {
    if (!history) return;
    const rows = [...history.points].reverse().map((point) => {
        const row = document.createElement('tr');
        [formatClock(point.time),
         point.moisture === null ? 'No reading' : `${point.moisture.toFixed(1)} %`,
         point.watered ? 'Yes' : '—'].forEach((text) => {
            const cell = document.createElement('td');
            cell.textContent = text;
            row.appendChild(cell);
        });
        return row;
    });
    elements.historyTable.replaceChildren(...rows);
}

// ── Wiring ──────────────────────────────────────────────────

elements.pumpButton.addEventListener('click', sendPumpRequest);
elements.saveConfig.addEventListener('click', saveConfig);
elements.thresholdLow.addEventListener('input', markConfigDirty);
elements.thresholdHigh.addEventListener('input', markConfigDirty);
elements.autoMode.addEventListener('change', markConfigDirty);
elements.allHours.addEventListener('click', () => {
    editedHoursMask = ALL_HOURS_MASK;
    markConfigDirty();
    renderHourChips();
});
elements.calibrateDry.addEventListener('click', () =>
    calibrate({ point: 'dry' }, 'Is the sensor dry and held in the air?'));
elements.calibrateWet.addEventListener('click', () =>
    calibrate({ point: 'wet' }, 'Is the sensor submerged in water?'));
elements.calibrateReset.addEventListener('click', () =>
    calibrate({ reset: true }, 'Reset calibration to the firmware defaults?'));
elements.wifiForm.addEventListener('submit', connectWifi);
elements.historyDetails.addEventListener('toggle', () => {
    if (elements.historyDetails.open) renderHistoryTable();
});
window.addEventListener('resize', renderHistory);

renderHourChips();
setInterval(fetchStatus, STATUS_INTERVAL_MS);
setInterval(fetchHistory, HISTORY_INTERVAL_MS);
fetchStatus().then(fetchHistory);
