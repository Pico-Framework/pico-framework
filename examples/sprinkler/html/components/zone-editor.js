import { apiGet, apiPut, apiPost } from '../utils/api.js';

class ZoneEditor extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `<section><h2>Zones</h2><p>Loading...</p></section>`;

    setTimeout(async () => {
      try {
        const zones = await apiGet('/api/v1/zones');
        console.log('Fetched zones:', zones);
        this.renderZones(zones);
      } catch (err) {
        console.error('Fetch failed:', err);
        this.innerHTML = `<section><h2>Zones</h2><p>Error loading zones.</p></section>`;
      }
    }, 100);
  }

  getZonePayload(form, imageOverride = null) {
    const id = form.dataset.id;
    const name = form.querySelector('input[name="name"]').value.trim();
    const gpioPin = parseInt(form.querySelector('input[name="gpioPin"]').value, 10);
    const active = form.querySelector('input[name="active"]').checked;

    const img = form.querySelector('img.zone-preview');
    const image = imageOverride || img.src.split('/').pop().split('?')[0];

    return {
      id,
      name,
      gpioPin,
      active,
      image
    };
  }


  renderZones(zones) {
    const html = zones.map(zone => `
      <form class="zone-editor-form" data-id="${zone.id}">
        <label>Name</label>
        <input type="text" name="name" value="${zone.name}" />
        
        <label>GPIO Pin</label>
        <input type="text" name="gpioPin" value="${zone.gpioPin}" disabled />
        
        <label>Active</label>
        <input type="checkbox" name="active" ${zone.active ? 'checked' : ''} disabled />
        
        <img src="/uploads/${zone.image}" alt="${zone.name}" class="zone-preview">
        
        <label class="upload-label">
          Change Image
          <input type="file" name="image" data-id="${zone.id}" accept="image/*" hidden>
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
        const id = input.dataset.id;
        const image = input.files[0].name;
        const file = input.files[0];
        if (!file) return;

        const form = input.closest('form');
        const img = form.querySelector('img.zone-preview');

        const formData = new FormData();
        formData.append('file', file);
        formData.append('id', id);
        formData.append('image', image);

        try {
          const response = await fetch('/api/v1/upload', {
            method: 'POST',
            body: formData
          });

          if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
          }

          const result = await response.json();
          if (result && result.image) {
            // Update the preview image without reload
            img.src = `/uploads/${encodeURIComponent(result.image)}?t=${Date.now()}`; // cache-bust
            alert(`Image ${result.image} uploaded for zone ID ${id}.`);
          } else {
            alert('Upload succeeded, but no image info returned.');
          }
        } catch (err) {
          console.error('Upload failed:', err);
          alert('Failed to upload image.');
        }
      });
    });

    this.querySelectorAll('.zone-editor-form').forEach(form => {
      form.addEventListener('submit', async e => {
        e.preventDefault();

        const payload = this.getZonePayload(form);

        try {
          await apiPut(`/api/v1/zones/${payload.id}`, payload);
          alert('Zone updated.');
        } catch (err) {
          console.error('Failed to update zone:', err);
          alert('Failed to update zone.');
        }
      });
    });
  }
}

customElements.define('zone-editor', ZoneEditor);
