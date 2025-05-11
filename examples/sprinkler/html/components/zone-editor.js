import { apiGet, apiPut } from '../utils/api.js';

class ZoneEditor extends HTMLElement {
  async connectedCallback() {
    this.innerHTML = `<section><h2>Zone Editor</h2><p>Loading zones...</p></section>`;
    try {
      const zones = await apiGet('/api/v1/zones');
      this.renderZones(zones);
    } catch (err) {
      this.innerHTML = `<section><h2>Zone Editor</h2><p>Error loading zones.</p></section>`;
    }
  }

  renderZones(zones) {
    const html = zones.map(zone => `
      <form class="zone-editor-form" data-original-name="${zone.name}">
        <label>Name</label>
        <input type="text" name="name" value="${zone.name}" />
        <label>GPIO Pin</label>
        <input type="text" name="gpioPin" value="${zone.gpioPin}" disabled />
        <label>Active</label>
        <input type="checkbox" name="active" ${zone.active ? 'checked' : ''} disabled />
        <button type="submit">Save</button>
      </form>
    `).join('');

    this.innerHTML = `
      <section>
        <h2>Zone Editor</h2>
        <div class="zone-editor-list">${html}</div>
      </section>
    `;

    this.querySelectorAll('.zone-editor-form').forEach(form => {
      form.addEventListener('submit', async e => {
        e.preventDefault();
        const formData = new FormData(form);
        const newName = formData.get('name').trim();
        const originalName = form.dataset.originalName;
        const gpioPin = parseInt(form.querySelector('input[name="gpioPin"]').value, 10);
        const active = form.querySelector('input[name="active"]').checked;

        const payload = { name: newName, gpioPin, active };

        try {
          await apiPut(`/api/v1/zones/${originalName}`, payload);
          location.reload(); // refresh after rename
        } catch (err) {
          alert('Failed to update zone.');
        }
      });
    });
  }
}

customElements.define('zone-editor', ZoneEditor);
