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
      <div class="zone-card ${zone.active ? 'active-zone' : ''}">
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
        <p class="last-run">Last run: <span>--</span></p>
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
    `;
  
    // Safe now that #next-program exists
    this.querySelector('#next-program').textContent = 'Morning Watering';
    this.querySelector('#next-start-time').textContent = '06:00';
  
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
      });
    });
  }
  
} 
const programEl = document.getElementById('next-program');
if (programEl) programEl.textContent = 'Morning Watering';

const timeEl = document.getElementById('next-start-time');
if (timeEl) timeEl.textContent = '06:00';


customElements.define('sprinkler-dashboard', Dashboard);
