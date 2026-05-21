const state = {
    moisture: 0,
    pumpOn: false,
    autoMode: true,
    thresholdLow: 30,
    thresholdHigh: 60,
};

const elements = {
    moisture: document.getElementById('moisture'),
    pumpState: document.getElementById('pumpState'),
    mode: document.getElementById('mode'),
    pumpButton: document.getElementById('pumpButton'),
    thresholdLow: document.getElementById('thresholdLow'),
    thresholdHigh: document.getElementById('thresholdHigh'),
    thresholdLowValue: document.getElementById('thresholdLowValue'),
    thresholdHighValue: document.getElementById('thresholdHighValue'),
    autoMode: document.getElementById('autoMode'),
    saveConfig: document.getElementById('saveConfig'),
};

async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (!response.ok) {
            return;
        }
        const data = await response.json();
        state.moisture = data.moisture_pct;
        state.pumpOn = data.pump_state === 1;
        state.autoMode = data.auto_mode;
        state.thresholdLow = data.threshold_low;
        state.thresholdHigh = data.threshold_high;
        render();
    } catch (error) {
        console.error('Status error', error);
    }
}

function render() {
    elements.moisture.textContent = `${state.moisture.toFixed(1)} %`;
    elements.pumpState.textContent = state.pumpOn ? 'ON' : 'OFF';
    elements.mode.textContent = state.autoMode ? 'AUTO' : 'MANUAL';
    elements.thresholdLow.value = state.thresholdLow;
    elements.thresholdHigh.value = state.thresholdHigh;
    elements.thresholdLowValue.textContent = state.thresholdLow;
    elements.thresholdHighValue.textContent = state.thresholdHigh;
    elements.autoMode.checked = state.autoMode;
}

async function sendPumpRequest() {
    await fetch('/api/pump', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ pump_on: !state.pumpOn }),
    });
    await fetchStatus();
}

async function saveConfig() {
    await fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            threshold_low: parseInt(elements.thresholdLow.value, 10),
            threshold_high: parseInt(elements.thresholdHigh.value, 10),
            auto_mode: elements.autoMode.checked,
        }),
    });
    await fetchStatus();
}

elements.pumpButton.addEventListener('click', sendPumpRequest);
elements.saveConfig.addEventListener('click', saveConfig);

setInterval(fetchStatus, 3000);
fetchStatus();
