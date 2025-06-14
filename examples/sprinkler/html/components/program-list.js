import { apiGet, apiDelete } from '../utils/api.js';
import { utcToLocalTimeString } from '../utils/time.js';

class ProgramList extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `<section><h2>Programs</h2><p>Loading...</p></section>`;
  
    setTimeout(async () => {
      try {
        const programs = await apiGet('/api/v1/programs');
        this.render(programs);
      } catch (err) {
        console.error('Fetch failed:', err);
        this.innerHTML = `<section><h2>Programs</h2><p>Error loading programs.</p></section>`;
      }
    }, 100);
  }

  render(programs) {
    const html = programs.map(p => `
      <div class="program-card">
        <h3>${p.name}</h3>
        <div class="program-days">
          <div><strong>Start:</strong> ${utcToLocalTimeString(p.start)}</div>
          <div><strong>Days:</strong> ${this.daysToText(p.days)}</div>
        </div>
        <ul>
          ${p.zones.map(z => `<li>${z.zone} – ${z.duration}s</li>`).join('')}
        </ul>
        <div class="program-actions">
          <a href="#/programs/edit?name=${encodeURIComponent(p.name)}">
            <button>Edit</button>
          </a>
          <button data-name="${p.name}" class="delete">Delete</button>
        </div>
      </div>
    `).join('');
  
    this.innerHTML = `
      <section>
        <h2>Program List</h2>
        <button id="add-program-btn">➕ Add Program</button>
        <div class="program-grid">${html}</div>
      </section>
    `;
  
    this.querySelector('#add-program-btn')?.addEventListener('click', () => {
      location.hash = '#/programs/edit';
    });
  
    this.querySelectorAll('.delete').forEach(button => {
      button.addEventListener('click', async () => {
        const name = button.dataset.name;
        if (confirm(`Delete program "${name}"?`)) {
          try {
            await apiDelete(`/api/v1/programs/${encodeURIComponent(name)}`);
            const card = button.closest('.program-card');
            card.classList.add('fade-out');
            setTimeout(() => card.remove(), 400);
          } catch {
            alert('Failed to delete program.');
          }
        }
      });
    });
  }  

  daysToText(mask) {
    const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    return days.filter((_, i) => mask & (1 << i)).join(', ');
  }
}

customElements.define('program-list', ProgramList);
