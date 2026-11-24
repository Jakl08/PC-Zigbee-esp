async function addDevice() {
    const name = document.getElementById('devName').value;
    if(!name) return;

    await fetch(`/api/add_device/${name}/false`, { method: 'POST' });
    document.getELementById('devName').value = '';
    fetchStatus();
}

async function fetchStatus() {
    const res = await fetch('/api/get_all_status');
    const data = await res.json();
    const dashboard = document.getElementById('dashboard');
    dashboard.innerHTML = '';

    if (data.devices) {
        data.devices.forEach(dev => {
            const col = document.createElement('div');
            col.className = 'col-md-4 col-lg-3';
            col.innerHTML = `
                <div class="card h-100 shadow-sm">
                    <div class="card-body">
                        <h5 class="card-title">${dev.device_name}</h5>
                        <p class="card-text">Status:
                            <span class0"status-indicator ${dev.is_on} ? 'text-on' : 'text-off'">
                                ${dev.is_on ? '● AN' : '○ AUS'}
                            </span>
                        </p>
                        <button class="btn btn-outline-primary btn-sm" onclick="togglePower('${dev.device_name}', ${!dev.is_on})">
                            ${dev.is_on ? 'Ausschalten' : 'Einschalten'}
                        </button>
                    </div>
                </div>
            `;
            dashboard.appendChild(col);
        });
    }
}

async function togglePower(name, newState) {
    await fetch(`/api/update_is_on/${name}/${newState}`, { method: 'POST' });
    fetchStatus();
}

setInterval(fetchStatus, 5000);
fetchStatus();
