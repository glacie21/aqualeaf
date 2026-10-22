// Mirrors PumpState in include/Constants.h
const PUMP_STATE_LABELS = ['OFF', 'ON', 'COOLDOWN', 'MANUAL', 'FAULT'];
const PUMP_STATE_ON = 1;
const OPERATING_MODE_AUTO = 0;

const state = {
    moisture: 0,
    pumpState: 0,
    autoMode: true,
    manualOverride: false,
    thresholdLow: 30,
    thresholdHigh: 60,
    sensorFault: false,
    wateringLockout: false,
    dailyLimitReached: false,
    overrideRemainingS: 0,
    weakWebPassword: false,
};

// True while the user has unsaved changes in the config form,
// so periodic polling does not overwrite the sliders.
let configDirty = false;

const elements = {
    statusPill: document.getElementById('statusPill'),
    alerts: document.getElementById('alerts'),
    moisture: document.getElementById('moisture'),
    pumpState: document.getElementById('pumpState'),
    mode: document.getElementById('mode'),
    modeHint: document.getElementById('modeHint'),
    pumpButton: document.getElementById('pumpButton'),
    thresholdLow: document.getElementById('thresholdLow'),
    thresholdHigh: document.getElementById('thresholdHigh'),
    thresholdLowValue: document.getElementById('thresholdLowValue'),
    thresholdHighValue: document.getElementById('thresholdHighValue'),
    autoMode: document.getElementById('autoMode'),
    saveConfig: document.getElementById('saveConfig'),
};

function setOnline(online) {
    elements.statusPill.textContent = online ? 'System Online' : 'Device Offline';
    elements.statusPill.classList.toggle('offline', !online);
}

function applyStatus(data) {
    state.moisture = data.moisture_pct;
    state.pumpState = data.pump_state;
    state.autoMode = data.auto_mode;
    state.manualOverride = data.operating_mode !== OPERATING_MODE_AUTO || data.manual_override;
    state.thresholdLow = data.threshold_low;
    state.thresholdHigh = data.threshold_high;
    state.sensorFault = data.sensor_fault;
    state.wateringLockout = data.watering_lockout;
    state.dailyLimitReached = data.daily_limit_reached;
    state.overrideRemainingS = data.override_remaining_s;
    state.weakWebPassword = data.weak_web_password;
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

async function postJson(url, body) {
    try {
        const response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body),
        });
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        applyStatus(await response.json());
        setOnline(true);
        return true;
    } catch (error) {
        console.error(`Request to ${url} failed`, error);
        setOnline(false);
        return false;
    }
}

function renderThresholdLabels() {
    elements.thresholdLowValue.textContent = elements.thresholdLow.value;
    elements.thresholdHighValue.textContent = elements.thresholdHigh.value;
}

function render() {
    const pumpOn = state.pumpState === PUMP_STATE_ON;
    const isAuto = state.autoMode && !state.manualOverride;

    elements.moisture.textContent = `${state.moisture.toFixed(1)} %`;
    elements.pumpState.textContent = PUMP_STATE_LABELS[state.pumpState] || '--';
    elements.pumpState.classList.toggle('status-on', pumpOn);
    elements.pumpState.classList.toggle('status-off', !pumpOn);
    elements.pumpButton.textContent = pumpOn ? 'Turn Pump Off' : 'Turn Pump On';

    elements.mode.textContent = isAuto ? 'AUTO' : 'MANUAL';
    elements.mode.classList.toggle('status-on', isAuto);
    elements.mode.classList.toggle('status-off', !isAuto);
    if (isAuto) {
        elements.modeHint.textContent = 'Automatic control';
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
        renderThresholdLabels();
    }
}

async function sendPumpRequest() {
    await postJson('/api/pump', { pump_on: state.pumpState !== PUMP_STATE_ON });
}

async function saveConfig() {
    const saved = await postJson('/api/config', {
        threshold_low: parseInt(elements.thresholdLow.value, 10),
        threshold_high: parseInt(elements.thresholdHigh.value, 10),
        auto_mode: elements.autoMode.checked,
    });
    if (saved) {
        configDirty = false;
        render();
    }
}

function markConfigDirty() {
    configDirty = true;
    renderThresholdLabels();
}

elements.pumpButton.addEventListener('click', sendPumpRequest);
elements.saveConfig.addEventListener('click', saveConfig);
elements.thresholdLow.addEventListener('input', markConfigDirty);
elements.thresholdHigh.addEventListener('input', markConfigDirty);
elements.autoMode.addEventListener('change', markConfigDirty);

setInterval(fetchStatus, 3000);
fetchStatus();
