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
};

// True while the user has unsaved changes in the config form,
// so periodic polling does not overwrite the sliders.
let configDirty = false;

const elements = {
    statusPill: document.getElementById('statusPill'),
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
    render();
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
    elements.modeHint.textContent = isAuto
        ? 'Automatic control'
        : 'Save configuration to resume automatic mode';

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
