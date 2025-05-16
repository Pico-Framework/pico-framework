import { apiGet, apiPost } from '../utils/api.js';

class Dashboard extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `<section><h2>Sprinkler Dashboard</h2><p>Loading zones...</p></section>`;
  
    setTimeout(async () => {
      try {
        const zones = await apiGet('/api/v1/zones');
        this.renderZones(zones);
      } catch (err) {
        this.innerHTML = `<section><h2>Sprinkler Dashboard</h2><p>Error loading zones.</p></section>`;
      }
    }, 100);
  }

  renderZones(zones) {
    const list = zones.map(zone => `
      <div class="zone-card ${zone.active ? 'active-zone' : ''}" data-name="${zone.name}">
        <div class="zone-image">
          ${zone.image ? `
            <img 
              src="/uploads/${encodeURIComponent(zone.image)}" 
              alt="${zone.name}" 
              class="zone-preview"
              onerror="this.style.display='none'"
            />
          ` : `
            <span class="zone-name-placeholder">${zone.name || 'Unnamed Zone'}</span>
          `}
        </div>
        <p class="last-status">Last event: <span>--</span></p>
        <p>GPIO: ${zone.gpioPin}</p>

        <div class="zone">
          <span class="led ${zone.running ? 'led-on' : 'led-off'}"></span>
          <span class="zone-name">${zone.name || 'Unnamed Zone'}</span>
          <button data-name="${zone.name}" class="zone-toggle">
            ${zone.running ? 'Stop' : 'Start'}
          </button>
        </div>
      </div>
    `).join('');
  
    this.innerHTML = `
      <section class="dashboard-summary">
        <h2>Sprinkler Dashboard</h2>
        <div class="next-run">
          <strong>Next Scheduled:</strong> <span id="next-program">None</span> at <span id="next-start-time">--:--</span>
        </div>
      </section>
      <section>
        <div class="zone-grid">${list}</div>
      </section>
      <button id="test-run-btn">Run Test Program</button>
    `;

    this.querySelector('#test-run-btn')?.addEventListener('click', async () => {
      try {
        await apiPost('/api/v1/test-program', { name: 'TestRun' });
        alert('Test program queued successfully.');
        await this.fetchNextSchedule(); // Refresh display
      } catch (err) {
        console.error('Test run failed:', err);
        alert(err.message); // Will show "POST /api/v1/test-program failed"
      }
    });
    
    
    this.querySelectorAll('button.zone-toggle').forEach(btn => {
      btn.addEventListener('click', async () => {
        const name = btn.dataset.name;
        const action = btn.textContent.trim().toLowerCase();
        try {
          await apiPost(`/api/v1/zones/${name}/${action}`);
          const running = action === 'start';
    
          const card = btn.closest('.zone-card');
          const led = card.querySelector('.led');
          const toggle = card.querySelector('.zone-toggle');
    
          if (led) led.className = `led ${running ? 'led-on' : 'led-off'}`;
          if (toggle) toggle.textContent = running ? 'Stop' : 'Start';
    
        } catch (err) {
          alert(`Failed to ${action} zone`);
        }
        try {
          const logSummary = await apiGet('/api/v1/logs/summaryJson'); 
        
          Object.entries(logSummary.zones || {}).forEach(([zoneName, data]) => {
            const card = this.querySelector(`.zone-card[data-name="${zoneName}"]`);
            if (!card) return;
          
            const statusEl = card.querySelector('.last-status span'); 
          
            if (statusEl && data.time && data.status) {
              const ts = new Date(data.time);
              const formatted = `${data.status} at ${ts.getFullYear()}-${(ts.getMonth() + 1).toString().padStart(2, '0')}-${ts.getDate().toString().padStart(2, '0')} ${ts.getHours().toString().padStart(2, '0')}:${ts.getMinutes().toString().padStart(2, '0')}`;
              statusEl.textContent = formatted;
            }
          });
        } catch (err) {
          console.warn('Failed to fetch log summary:', err);
        }
      });
    });
    this.fetchNextSchedule();
    // Start periodic log summary refresh
    this._logRefreshTimer = setInterval(() => {
      this.updateZoneStatusFromLog();
    }, 10000); // every 10 seconds
  }

  async updateZoneStatusFromLog() {
    try {
      const logSummary = await apiGet('/api/v1/logs/summaryJson');
  
      Object.entries(logSummary.zones || {}).forEach(([zoneName, data]) => {
        const card = this.querySelector(`.zone-card[data-name="${zoneName}"]`);
        if (!card) return;
  
        const statusEl = card.querySelector('.last-status span');
  
        if (statusEl && data.time && data.status) {
          const ts = new Date(data.time);
          const formatted = `${data.status} at ${ts.getFullYear()}-${(ts.getMonth() + 1).toString().padStart(2, '0')}-${ts.getDate().toString().padStart(2, '0')} ${ts.getHours().toString().padStart(2, '0')}:${ts.getMinutes().toString().padStart(2, '0')}`;
          statusEl.textContent = formatted;
        }
      });
    } catch (err) {
      console.warn('Failed to refresh log summary:', err);
    }
  }
  
  async fetchNextSchedule() {
    try {
      const programEl = this.querySelector('#next-program');
      const timeEl = this.querySelector('#next-start-time');
  
      const data = await apiGet('/api/v1/next-schedule');
      if (data.status === 'scheduled') {
        const ts = new Date(data.time);
        const timeString = ts.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
        programEl.textContent = data.name;
        timeEl.textContent = timeString;
      } else {
        programEl.textContent = 'None';
        timeEl.textContent = '--:--';
      }
    } catch (err) {
      console.error('Failed to fetch next schedule:', err);
      const programEl = this.querySelector('#next-program');
      const timeEl = this.querySelector('#next-start-time');
      if (programEl) programEl.textContent = 'Error';
      if (timeEl) timeEl.textContent = '--:--';
    }
  }
  
} 

customElements.define('sprinkler-dashboard', Dashboard);
