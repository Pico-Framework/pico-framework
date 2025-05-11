import { apiGet, apiDelete } from '../utils/api.js';

class ProgramList extends HTMLElement {
  async connectedCallback() {
    this.innerHTML = `<section><h2>Program List</h2><p>Loading programs...</p></section>`;
    try {
      const programs = await apiGet('/api/v1/programs');
      this.render(programs);
    } catch (err) {
      this.innerHTML = `<section><h2>Program List</h2><p>Error loading programs.</p></section>`;
    }
  }

  render(programs) {
    const html = programs.map(p => `
      <div class="program-card">
        <h3>${p.name}</h3>
        <p><strong>Start:</strong> ${p.start}</p>
        <p><strong>Days:</strong> ${this.daysToText(p.days)}</p>
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
        <div class="program-grid">${html}</div>
      </section>
    `;

    this.querySelectorAll('.delete').forEach(button => {
      button.addEventListener('click', async () => {
        const name = button.dataset.name;
        if (confirm(`Delete program "${name}"?`)) {
          try {
            await apiDelete(`/api/v1/programs/${encodeURIComponent(name)}`);
            location.reload();
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
