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
      <div class="zone-card ${zone.active ? 'active' : ''}">
        <h3>${zone.name}</h3>
        <p>GPIO: ${zone.gpioPin}</p>
        <button data-action="start" data-name="${zone.name}">Start</button>
        <button data-action="stop" data-name="${zone.name}">Stop</button>
      </div>
    `).join('');

    this.innerHTML = `
      <section>
        <h2>Sprinkler Dashboard</h2>
        <div class="zone-grid">${list}</div>
      </section>
    `;

    this.querySelectorAll('button').forEach(btn => {
      btn.addEventListener('click', async () => {
        const name = btn.dataset.name;
        const action = btn.dataset.action;
        await apiPost(`/api/v1/zones/${name}/${action}`, {});
        location.reload(); // simple refresh for now
      });
    });
  }
}

customElements.define('sprinkler-dashboard', Dashboard);
