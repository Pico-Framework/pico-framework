import { apiGet, apiPut } from '../utils/api.js';

class ZoneEditor extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `<section><h2>Zones</h2><p>Loading...</p></section>`;

    setTimeout(async () => {
      try {
        const zones = await apiGet('/api/v1/zones');
        this.renderZones(zones);
      } catch (err) {
        console.error('Fetch failed:', err);
        this.innerHTML = `<section><h2>Zones</h2><p>Error loading zones.</p></section>`;
      }
    }, 100);
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
        
        <img src="/uploads/${zone.name}.jpg" alt="${zone.name}" class="zone-preview">
        
        <label class="upload-label">
          Change Image
          <input type="file" name="image" data-name="${zone.name}" accept="image/*" hidden>
        </label>
        
        <button type="submit">Save</button>
      </form>
    `).join('');


    this.innerHTML = `
      <section>
        <h2>Zone Editor</h2>
        <div class="zone-editor-list">${html}</div>
      </section>
    `;

    this.querySelectorAll('input[type="file"][name="image"]').forEach(input => {
      input.addEventListener('change', async e => {
        const name = input.dataset.name;
        const file = input.files[0];
        if (!file) return;

        const formData = new FormData();
        formData.append('image', file);

        try {
          await apiPost(`/api/v1/zones/${encodeURIComponent(name)}/image`, formData);
          alert(`Image uploaded for zone ${name}.`);
          location.reload(); // or dynamically update preview if preferred
        } catch (err) {
          console.error('Upload failed:', err);
          alert('Failed to upload image.');
        }
      });
    });


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
          await apiPut(`/api/v1/zones/${encodeURIComponent(originalName)}`, payload);
          location.reload(); // refresh after rename
        } catch (err) {
          alert('Failed to update zone.');
        }
      });
    });
  }
}

customElements.define('zone-editor', ZoneEditor);
