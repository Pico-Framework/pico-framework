import { apiGet, apiPut } from '../utils/api.js';
import { utcToLocalTimeString } from '../utils/time.js';

class ProgramEditor extends HTMLElement {
  connectedCallback() {
    const params = new URLSearchParams(location.hash.split('?')[1] || '');
    this.programName = params.get('name');
    this.innerHTML = `<section><h2>Edit Program</h2><p>Loading...</p></section>`;
  
    setTimeout(async () => {
      try {
        let program = {
          name: '',
          start: '',
          days: [],
          zones: []
        };
  
        if (this.programName) {
          program = await apiGet(`/api/v1/programs/${encodeURIComponent(this.programName)}`);
        }
  
        this.render(program);
      } catch (err) {
        console.error('Fetch failed:', err);
        this.innerHTML = `<section><h2>Edit Program</h2><p>Error loading program.</p></section>`;
      }
    }, 100);
  }
  
  
  
  render(program) {
    const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

    const daysCheckboxes = days.map((day, i) => `
      <label><input type="checkbox" name="days" value="${1 << i}" ${program.days & (1 << i) ? 'checked' : ''}> ${day}</label>
    `).join('');

    const zonesHtml = program.zones.map((z, i) => `
      <div class="zone-row" data-index="${i}">
        <input type="text" name="zone" value="${z.zone}" placeholder="Zone name" />
        <input type="number" name="duration" value="${z.duration}" placeholder="Duration (s)" />
        <button type="button" class="remove-zone">✕</button>
      </div>
    `).join('');

    this.innerHTML = `
      <section>
        <h2>Edit Program: ${program.name}</h2>
        <form id="program-form">
          <label>Program Name</label>
          <input type="text" name="name" value="${program.name}" />

          <label>Start Time</label>
          <input type="time" name="start" value="${utcToLocalTimeString(program.start)}" />

          <fieldset><legend>Days</legend>${daysCheckboxes}</fieldset>

          <label>Zones</label>
          <div id="zones">${zonesHtml}</div>
          <button type="button" id="add-zone">+ Add Zone</button>

          <button type="submit">Save Program</button>
        </form>
      </section>
    `;

    this.querySelector('#add-zone').addEventListener('click', () => {
      const container = this.querySelector('#zones');
      const div = document.createElement('div');
      div.className = 'zone-row';
      div.innerHTML = `
        <input type="text" name="zone" placeholder="Flower Bed" />
        <input type="number" name="duration" value="0" placeholder="Duration (s)" />
        <button type="button" class="remove-zone">✕</button>
      `;
      container.appendChild(div);
      div.querySelector('.remove-zone').addEventListener('click', () => div.remove());
    });

    this.querySelectorAll('.remove-zone').forEach(btn =>
      btn.addEventListener('click', () => btn.closest('.zone-row').remove())
    );

    this.querySelector('#program-form').addEventListener('submit', async e => {
      e.preventDefault();
      const form = e.target;
      const formData = new FormData(form);
    
      const newName = formData.get('name').trim();
      const localTime = formData.get('start');
      const start = localTimeToUTCString(localTime);
      const days = [...form.querySelectorAll('input[name="days"]:checked')]
        .map(cb => parseInt(cb.value)).reduce((a, b) => a | b, 0);
    
      const zones = [...form.querySelectorAll('.zone-row')].map(row => ({
        zone: row.querySelector('input[name="zone"]').value.trim(),
        duration: parseInt(row.querySelector('input[name="duration"]').value)
      }));
    
      const payload = { name: newName, start, days, zones };
    
      if (!newName) {
        alert('Program name is required.');
        return;
      }
    
      try {
        if (this.programName) {
          await apiPut(`/api/v1/programs/${encodeURIComponent(this.programName)}`, payload);
        } else {
          await apiPost('/api/v1/programs', payload);
        }
        location.hash = '#/programs';
      } catch (err) {
        console.error('Save failed', err);
        alert('Failed to save program.');
      }
    });    
  }
}

customElements.define('program-editor', ProgramEditor);
